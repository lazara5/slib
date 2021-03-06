/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/Log.h"
#include "slib/lang/Numeric.h"
#include "slib/collections/ArrayList.h"
#include "slib/util/Config.h"
#include "slib/util/FileUtils.h"
#include "slib/lang/String.h"

#include "fmt/format.h"

#include <stdio.h>
#include <unistd.h>

#include <string>

#define STATIC_BUFFER_SIZE (1024)

namespace slib {

SPtr<spdlog::logger> Log::_staticLogger;
String Log::_staticFormat("[%d-%m-%C %T.%e][%l][%t] %v");
Log::Level Log::_staticLevel = Log::Level::Info;

/** Custom syslog sink. Allows for properly setting the facility */
class syslogSink : public spdlog::sinks::sink {
public:
	syslogSink(int syslog_option = 0, int facility = LOG_USER)
	:_facility(facility) {
		_priorities[static_cast<int>(spdlog::level::trace)] = LOG_DEBUG;
		_priorities[static_cast<int>(spdlog::level::debug)] = LOG_DEBUG;
		_priorities[static_cast<int>(spdlog::level::info)] = LOG_INFO;
		_priorities[static_cast<int>(spdlog::level::warn)] = LOG_WARNING;
		_priorities[static_cast<int>(spdlog::level::err)] = LOG_ERR;
		_priorities[static_cast<int>(spdlog::level::critical)] = LOG_CRIT;
		_priorities[static_cast<int>(spdlog::level::off)] = LOG_INFO;

		::openlog(nullptr, 0, 0);
	}

	~syslogSink() override {
		::closelog();
	}

	syslogSink(const syslogSink&) = delete;
	syslogSink& operator=(const syslogSink&) = delete;

	void log(const spdlog::details::log_msg &msg) override {
		::syslog(syslog_prio_from_level(msg) | (_facility << 3), "%s", msg.raw.str().c_str());
	}

	void flush() override {
	}

private:
	int _facility;
	std::array<int, 10> _priorities;

	// Simply maps spdlog's log level to syslog priority level.
	int syslog_prio_from_level(const spdlog::details::log_msg &msg) const {
		return _priorities[static_cast<int>(msg.level)];
	}
};

static bool inConsole() {
	if (stderr == nullptr)
		return false;
	return isatty(fileno(stderr));
}

void Log::sys(int priority, const char *message, va_list ap, va_list ap1) {
	vsyslog(priority, message, ap);

	if (inConsole()) {
		vfprintf(stderr, message, ap1);
		fprintf(stderr, "\n");
	}
}

void Log::sys(int priority, const char *message) {
	syslog(priority, "%s", message);

	if (inConsole()) {
		fprintf(stderr, "%s", message);
		fprintf(stderr, "\n");
	}
}

void Log::startup(int priority, const char *message) {
	if (_staticLogger) {
		try {
			_staticLogger->error(message);
		} catch(const spdlog::spdlog_ex& e) {
			fprintf(stderr, "log failed: %s\n", e.what());
		}
	} else
		sys(priority, message);
}

char *Log::fmtb(char *staticBuffer, size_t bufferLen, const char *format, va_list ap) {
	char *buffer = staticBuffer;
	va_list ap1;
	va_copy(ap1, ap);

	int fmtLen = vsnprintf(nullptr, 0, format, ap);
	if ((size_t)fmtLen + 1 > bufferLen)
		buffer = (char*)malloc(fmtLen + 1);

	vsnprintf(buffer, fmtLen + 1, format, ap1);
	va_end(ap1);
	return buffer;
}

static UPtr<String> getNextParam(ConstIterator<SPtr<String>> &params) {
	if (params.hasNext())
		return params.next()->trim();
	throw InitException(_HERE_, "Missing log parameter");
}

static UPtr<String> getNextParam(ConstIterator<SPtr<String>> &params, String const& defaultValue) {
	if (params.hasNext())
		return params.next()->trim();
	else
		return std::make_unique<String>(defaultValue);
}

static int getNextIntParam(ConstIterator<SPtr<String>> &params, int defaultValue) {
	if (params.hasNext()) {
		UPtr<String> strValue = params.next()->trim();
		try {
			return Integer::parseInt(CPtr(strValue));
		} catch (NumberFormatException const&) {
			throw InitException(_HERE_, fmt::format("Invalid int parameter: {}", *strValue).c_str());
		}
	} else
		return defaultValue;
}

bool getNextBoolParam(ConstIterator<std::string> &params, bool defaultValue) {
	if (params.hasNext()) {
		const std::string& strValue = String::trim(CPtr(params.next()));
		return Boolean::parseBoolean(CPtr(strValue));
	} else
		return defaultValue;
}

static const char* logTypes[] = {"DISABLED", "CONSOLE", "FILE", "SIZE_ROTATE", "DAILY_ROTATE", "SYSLOG", nullptr};

static LogType getLogType(String const& str) {
	for (int i = 0; logTypes[i] != nullptr; i++)
		if (!strcasecmp(str.c_str(), logTypes[i]))
			return (LogType)i;
	return LTYPE_NONE;
}

static const char* logLevels[] = {"DISABLED", "FATAL", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG", "TRACE", nullptr};

Log::Level static getLogLevel(String const& str) {
	for (int i = 0; logLevels[i] != nullptr; i++)
		if (!strcasecmp(str.c_str(), logLevels[i]))
			return (Log::Level)i;
	return Log::Level::None;
}

void Log::staticInit(const Config& cfg, ConstIterator<SPtr<String>> params) {
	UPtr<String> levelName = getNextParam(params);
	Level level = getLogLevel(*levelName);
	
	switch (level) {
		case Level::None:
			throw InitException(_HERE_, fmt::format("Invalid log level: {}", *levelName).c_str());
		case Level::Disabled:
			return;
		default:
			break;
	}

	_staticLevel = level;

	UPtr<String> mode = getNextParam(params);
	LogType type = getLogType(*mode);
		
	switch (type) {
		case LTYPE_NONE:
			throw InitException(_HERE_, fmt::format("Invalid log type: {}", *mode).c_str());
		case LTYPE_DISABLED:
			return;
		case LTYPE_CONSOLE:
			_staticLogger = initConsole(cfg, "log", params);
			break;
		case LTYPE_ROT_SIZE:
			_staticLogger = initRotating(cfg, "log", params);
			_staticLogger->set_pattern(_staticFormat.c_str());
			break;
		case LTYPE_SYSLOG:
			_staticLogger = initSyslog(cfg, "log", params);
			_staticLogger->set_pattern(_staticFormat.c_str());
			break;
		default:
			throw InitException(_HERE_, fmt::format("Unsupported log type: {}", *mode).c_str());
	}

	_staticLogger->set_level(spdlog::level::trace);
}

void Log::staticInit(const Config& cfg, String const& name, String const& defaultValue /*= ""*/) {
	SPtr<String> logCfg = cfg.getString(name, defaultValue);
	if (!logCfg)
		throw InitException(_HERE_, fmt::format("Log config not found ({})", name).c_str());

	UPtr<ArrayList<String>> logParams = logCfg->split(",");
	
	staticInit(cfg, logParams->constIterator());
}

static UPtr<String> createLogPath(const Config& cfg, String const& fileName) {
	UPtr<String> logFileName;
	if (FileUtils::isPathAbsolute(fileName))
		logFileName = std::make_unique<String>(fileName);
	else
		logFileName = FileUtils::buildPath(*cfg.getLogDir(), fileName);

	UPtr<String> logPath = FileUtils::getPath(*logFileName);
	int res = FileUtils::mkdirs(*logPath, "rwxrwxr-x");
	if (res < 0)
		throw InitException(_HERE_, fmt::format("Error creating log path '{}', errno={:d}", *logPath, errno).c_str());

	return logFileName;
}

typedef enum {
	CDEST_NONE = -1,
	CDEST_STDOUT = 0,
	CDEST_STDERR
} ConsoleDest;

static const char* consoleDestNames[] = {"STDOUT", "STDERR", nullptr};

static ConsoleDest getConsoleDest(String const& str) {
	for (int i = 0; consoleDestNames[i] != nullptr; i++)
		if (!strcasecmp(str.c_str(), consoleDestNames[i]))
			return (ConsoleDest)i;
	return CDEST_NONE;
}

SPtr<spdlog::logger> Log::initConsole(const Config& cfg, String const& name, ConstIterator<SPtr<String>> &params) {
	UPtr<String> destName = getNextParam(params, "STDOUT");
	ConsoleDest consoleDest = getConsoleDest(*destName);

	try {
		switch (consoleDest) {
			case CDEST_STDOUT:
				return spdlog::stdout_logger_mt(name.c_str());
			case CDEST_STDERR:
				return spdlog::stderr_logger_mt(name.c_str());
			case CDEST_NONE:
			default:
				throw InitException(_HERE_, fmt::format("Invalid console type: {}", *destName).c_str());
		}
	} catch(const spdlog::spdlog_ex& e) {
		throw InitException(_HERE_, fmt::format("Error creating log: {}", e.what()).c_str());
	}
}

SPtr<spdlog::logger> Log::initRotating(const Config& cfg, String const& name, ConstIterator<SPtr<String>> &params) {
	UPtr<String> fileName = getNextParam(params);
	int maxFileSize = getNextIntParam(params, 1024 * 1024);
	int maxFiles = getNextIntParam(params, 10);

	try {
		return spdlog::create<spdlog::sinks::rotating_file_sink_mt>(name.c_str(), createLogPath(cfg, *fileName)->c_str(),
																	maxFileSize, maxFiles);
	} catch(const spdlog::spdlog_ex& e) {
		throw InitException(_HERE_, fmt::format("Error creating log: {}", e.what()).c_str());
	}
}

SPtr<spdlog::logger> Log::initSyslog(const Config& cfg, String const& name, ConstIterator<SPtr<String>> &params) {
	int facility = getNextIntParam(params, LOG_USER);

	return spdlog::create<syslogSink>(name.c_str(), 0, facility);
}

} // namespace
