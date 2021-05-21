/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_CONFIG_H
#define H_SLIB_UTIL_CONFIG_H

#include "slib/lang/String.h"
//#include "slib/util/Log.h"
#include "slib/util/StringUtils.h"
#include "slib/lang/Numeric.h"
#include "slib/collections/LinkedList.h"
#include "slib/collections/Properties.h"
#include "slib/util/expr/Resolver.h"
#include "slib/util/SystemInfo.h"

#include "fmt/format.h"

#include <string>

namespace slib {

class ConfigException : public Exception {
protected:
	ConfigException(const char *where, const char *className, const char *msg)
	:Exception(where, className, msg) {}

	ConfigException(const char *where, const char *className, const char *msg, Exception const& cause)
	:Exception(where, className, msg, cause) {}
public:
	ConfigException(const char *where, const char *msg)
	:ConfigException(where, "ConfigException", msg) {}

	ConfigException(const char *where, const char *msg, Exception const& cause)
	:ConfigException(where, "EvaluationException", msg, cause) {}
};

class ResolverProxy : public expr::Resolver {
private:
	expr::Resolver *_resolver;
public:
	ResolverProxy(Resolver *resolver)
	: _resolver(resolver) {}

	virtual SPtr<Object> getVar(String const& key) const override {
		return _resolver->getVar(key);
	}
};

class Config {
protected:
	enum struct ValueType {
		INT,
		DOUBLE,
		BOOL,
		STRING,
		EXCEPTION,
		NUL
	};
	struct ConfigValue {
		union {
			int64_t _int;
			double _double;
			bool _bool;
		};
		SPtr<String> _where;
		SPtr<String> _str;
		ValueType _type;

		ConfigValue(): _int(0), _str(nullptr), _type(ValueType::NUL) {}
		ConfigValue(const char *where, const char *msg)
		: _where(newS<String>(where))
		, _str(newS<String>(msg))
		, _type(ValueType::EXCEPTION) {}
		ConfigValue(int64_t val) : _int(val), _str(nullptr), _type(ValueType::INT) {}
		ConfigValue(SPtr<String> const& str) : _int(0), _str(str), _type(ValueType::STRING) {}
	};
protected:
	static const SPtr<ConfigValue> _nullValue;

	SPtr<Map<BasicString, Object>> _configRoot;

	//std::string _rootDir;
	SPtr<String> _appDir;
	SPtr<String> _appName;
	SPtr<String> _confDir;
	//SPtr<String> _logDir;

	UPtr<Map<BasicString, ConfigValue>> _propCache;
protected:
	template <class S>
	SPtr<ConfigValue> internalGetProperty(S const* name) const {
		BasicStringView nameRef(strData(name), strLen(name));
		SPtr<ConfigValue> val = _propCache->get(nameRef);
		if (val) {
			switch (val->_type) {
				case ValueType::NUL:
					return nullptr;
				case ValueType::EXCEPTION:
					throw ConfigException(val->_where->c_str(), val->_str->c_str());
				default:
					return val;
			}

			return val;
		}

		size_t pathLen = 0;
		bool first = true;
		SPtr<Object> obj = _configRoot;
		StringSplitIterator si(name, '.');
		while (si.hasNext()) {
			BasicStringView elem = si.next();
			if (instanceof<Map<BasicString, Object>>(obj)) {
				obj = (Class::cast<Map<BasicString, Object>>(obj))->get(elem);
			} else if (instanceof<List<Object>>(obj)) {
				try {
					obj = (Class::cast<List<Object>>(obj))->get(ULong::parseULong(CPtr(elem)));
				} catch (Exception const& e) {
					_propCache->put(newS<String>(strData(name), strLen(name)),
									newS<ConfigValue>(_HERE_, fmt::format("Caused by {} [{} ({})]",
																		  e.getName(), e.getMessage(), e.where()).c_str()));
					// this time we will throw from the cache
					return internalGetProperty(name);
				}
			} else {
				_propCache->put(newS<String>(strData(name), strLen(name)),
								newS<ConfigValue>(_HERE_, fmt::format("{} is not an object or array",
																	  StringView(strData(name), pathLen)).c_str()));
				// this time we will throw from the cache
				return internalGetProperty(name);
			}
			if (!obj) {
				_propCache->put(newS<String>(strData(name), strLen(name)), _nullValue);
				return _nullValue;
			}
			pathLen += elem.length();
			if (!first)
				pathLen++;
			first = false;
			if (!si.hasNext()) {
				// leaf: cache and return
				SPtr<ConfigValue> val;

				if (instanceof<String>(obj))
					val = newS<ConfigValue>(Class::cast<String>(obj));
				else if (instanceof<Long>(obj))
					val = newS<ConfigValue>(Class::cast<Long>(obj)->longValue());

				_propCache->put(newS<String>(strData(name), strLen(name)), val);
				return val;
			}
		}

		return nullptr;
	}

	/** @throws NumberFormatException */
	template<class T, T MIN, T MAX>
	static const T& rangeCheck(const char *where, const T& value) {
		if ((value < MIN) || (value > MAX))
			throw NumberFormatException(where, "Value out of range");
		return value;
	}

public:
	Config(SPtr<Map<BasicString, Object>> const& configRoot,
		   SPtr<String> const& appName,
		   SPtr<String> const& confDir, SPtr<String> const& appDir)
	: _configRoot(configRoot)
	, _appDir(appDir)
	, _appName(appName)
	, _confDir(confDir)
	, _propCache(newU<HashMap<BasicString, ConfigValue>>()) {}

	Config(String const& confFileName, String const& appName);

	virtual ~Config() {}

	SPtr<String> getAppName() const { return _appName; }
	SPtr<String> getAppDir() const { return _appDir; }
	SPtr<String> getConfDir() const { return _confDir; }

	template <class S>
	SPtr<String> getString(S const* name) const {
		SPtr<ConfigValue> val = internalGetProperty(name);
		if (!val)
			throw MissingValueException(_HERE_, name);
		else if (val->_type == ValueType::STRING)
			return val->_str;
		else
			throw ConfigException(_HERE_, "Type mismatch");
	}

	template <class S>
	SPtr<String> getString(S const* name, SPtr<String> const& defaultValue) const {
		try {
			return getString(name);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	double getDouble(S const* name) {
		SPtr<ConfigValue> val = internalGetProperty(name);
		if (!val)
			throw MissingValueException(_HERE_, name);
		switch (val->_type) {
			case ValueType::DOUBLE:
				return val->_double;
			case ValueType::INT:
				return (double)val->_int;
			default:
				throw ConfigException(_HERE_, "Type mismatch");
		}
	}
};

class ConfigLoader {
protected:
	class ConfigResolver : public expr::Resolver {
	private:
		SPtr<String> _exeDir;
		SPtr<String> _cwd;
	public:
		ConfigResolver();

		virtual SPtr<Object> getVar(String const& key) const;
	};

	struct StringPair {
		SPtr<String> _first;
		SPtr<String> _second;

		StringPair(SPtr<String> first, SPtr<String> second)
		: _first(first)
		, _second(second) {}

		bool operator==(StringPair const& other) const {
			if (this == &other)
				return true;

			if (!_first) {
				if (other._first)
					return false;
			} else if (!(*_first == *other._first))
				return false;

			if (!_second) {
				if (other._second)
					return false;
			} else if (!(*_second == *other._second))
				return false;

			return true;
		}

	};

	SPtr<String> _confFileName;
	SPtr<String> _appName;

	SPtr<Map<String, Object>> _vars;
	SPtr<expr::ChainedResolver> _quickResolver;
	SPtr<expr::ChainedResolver> _resolver;
	LinkedList<StringPair> _searchPaths;
protected:
	SPtr<StringPair> searchConfigDir();
public:
	ConfigLoader(SPtr<String> const& confFileName, SPtr<String> const& appName);

	ConfigLoader& clearPaths();

	ConfigLoader& search(SPtr<String> const& path, SPtr<String> const& rootDir = nullptr);

	ConfigLoader& withResolver(SPtr<expr::Resolver> const& resolver);

	ConfigLoader& withResolver(SPtr<String> const& name, SPtr<expr::Resolver> const& resolver);

	/** @throws EvaluationException */
	SPtr<Config> load(bool quick = false);
};

} // namespace slib

#endif // H_SLIB_UTIL_CONFIG_H
