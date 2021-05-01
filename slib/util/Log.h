/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_LOG_H
#define H_SLIB_UTIL_LOG_H

#include "slib/util/Config.h"
#include "slib/util/Iterator.h"

#include "spdlog/spdlog.h"

#include <stdarg.h>
#include <syslog.h>

namespace slib {

class Config;

typedef enum {
	LTYPE_NONE = -1,
	LTYPE_DISABLED = 0,
	LTYPE_CONSOLE,
	LTYPE_FILE,
	LTYPE_ROT_SIZE,
	LTYPE_ROT_DAILY,
	LTYPE_SYSLOG
} LogType;

class Log {
public:
	enum class Level {
		None = -1,
		Disabled = 0,
		Fatal,
		Error,
		Warn,
		Notice,
		Info,
		Debug,
		Trace
	};
protected:
	static SPtr<spdlog::logger> _staticLogger;

	std::string _name;
	static String _staticFormat;
	static Level _staticLevel;
protected:
	static SPtr<spdlog::logger> initConsole(const Config& cfg, String const& name, UPtr<ConstIterator<SPtr<String>>> const& params);
	static SPtr<spdlog::logger> initRotating(const Config& cfg, String const& name, UPtr<ConstIterator<SPtr<String>>> const& params);
	static SPtr<spdlog::logger> initSyslog(const Config& cfg, String const& name, UPtr<ConstIterator<SPtr<String>>> const& params);

	static char *fmtb(char *staticBuffer, size_t bufferLen, const char *format, va_list ap);
protected:
	static void sys(int priority, const char *message, va_list ap, va_list ap1);
public:
	/*Log(std::string const& name) {
		_name = name;
		//_format = "[%d-%m-%C %T.%e] %v"; 
		_format = "[%d-%m-%C %T.%e][%l][%t] %v";
		_level = Level::Info;
	}*/

	static bool enabled() {
		return (bool)_staticLogger;
	}

	static void sys(int level, const char *message);

	static void sysf(int level, const char *format, ...)
	__attribute__((format (printf, 2, 3))) {
		va_list ap, ap1;
		va_start(ap, format);
		va_copy(ap1, ap);
		sys(level, format, ap, ap1);
		va_end(ap1);
		va_end(ap);
	}

	static void startup(int level, const char *message);
	
	/** logs a startup error message. This is written to stderr and system log.
	 * This function can be called before log::init() because it is NOT necessarily writing in the log files
	 */
	template <typename... Args>
	void startupf(int level, const char *format, const Args&... args) {
		startup(level, fmt::format(format, args...).c_str());
	}

	static void staticInit(const Config& cfg, UPtr<ConstIterator<SPtr<String>>> const& params);
	static void staticInit(const Config& cfg, String const& name, String const& defaultValue = "");

	static bool enabled(Level level) {
		if (!_staticLogger)
			return false;
		return (_staticLevel >= level);
	}

	static void log(Level level, spdlog::level::level_enum l, const char *msg) {
		if (_staticLevel < level)
			return;
		if (_staticLogger) {
			try {
				_staticLogger->log(l, msg);
			} catch(const spdlog::spdlog_ex& e) {
				fprintf(stderr, "log failed: %s\n", e.what());
			}
		}
	}

	template <typename... Args>
	static void log(Level level, spdlog::level::level_enum l, const char *fmt, const Args&... args) {
		if (_staticLevel < level)
			return;
		if (_staticLogger) {
			try {
				_staticLogger->log(l, fmt, args...);
			} catch(const spdlog::spdlog_ex& e) {
				fprintf(stderr, "log failed: %s\n", e.what());
			}
		}
	}

	static void logvf(Level level, spdlog::level::level_enum l, const char *format, va_list ap) {
		if (_staticLevel < level)
			return;
		if (_staticLogger) {
			char staticBuffer[256];
			char *fmtBuffer = nullptr;
			try {
				fmtBuffer = fmtb(staticBuffer, sizeof(staticBuffer), format, ap);
				_staticLogger->log(l, fmtBuffer);
			} catch(const spdlog::spdlog_ex& e) {
				fprintf(stderr, "log failed: %s\n", e.what());
			}
			if (fmtBuffer && (fmtBuffer != staticBuffer))
				free(fmtBuffer);
		}
	}

	void info(const char *message) {
		log(Level::Info, spdlog::level::info, message);
	}

	template <typename... Args>
	void infof(const char *format, const Args&... args) {
		log(Level::Info, spdlog::level::info, format, args...);
	}

	void warn(const char *message) {
		log(Level::Warn, spdlog::level::warn, message);
	}

	template <typename... Args>
	static void warnf(const char *format, const Args&... args) {
		log(Level::Warn, spdlog::level::warn, format, args...);
	}

	void error(const char *message) {
		log(Level::Error, spdlog::level::err, message);
	}

	template <typename... Args>
	void errorf(const char *format, const Args&... args) {
		log(Level::Error, spdlog::level::err, format, args...);
	}

	void errorvf(const char *format, va_list ap) {
		logvf(Level::Error, spdlog::level::err, format, ap);
	}

	template <typename... Args>
	void fatalf(const char *format, const Args&... args) {
		log(Level::Fatal, spdlog::level::critical, format, args...);
	}

	void fatal(const char *message) {
		log(Level::Fatal, spdlog::level::critical, message);
	}

	void debug(const char *message) {
		log(Level::Debug, spdlog::level::debug, message);
	}

	template <typename... Args>
	void debugf(const char *format, const Args&... args) {
		log(Level::Debug, spdlog::level::debug, format, args...);
	}

	template <typename... Args>
	void tracef(const char *format, const Args&... args) {
		log(Level::Trace, spdlog::level::trace, format, args...);
	}

	void trace(const char *message) {
		log(Level::Trace, spdlog::level::trace, message);
	}

	static void shutdown() {
		// Under VisualStudio, this must be called before main finishes to workaround a known VS issue
		spdlog::drop_all(); 
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_LOG_H
