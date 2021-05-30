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

enum struct ConfigValueType {
	LONG,
	DOUBLE,
	BOOL,
	STRING,
	OBJ,
	EXCEPTION,
	NUL
};

class ObjConfig {
protected:
	struct ConfigValue {
		union {
			int64_t _long;
			double _double;
			bool _bool;
		};
		SPtr<String> _where;
		SPtr<String> _str;
		SPtr<ObjConfig> _obj;
		ConfigValueType _type;

		ConfigValue(): _long(0), _str(nullptr), _type(ConfigValueType::NUL) {}
		ConfigValue(const char *where, const char *msg)
		: _where(newS<String>(where))
		, _str(newS<String>(msg))
		, _type(ConfigValueType::EXCEPTION) {}
		ConfigValue(int64_t val) : _long(val), _type(ConfigValueType::LONG) {}
		ConfigValue(double val) : _double(val), _type(ConfigValueType::DOUBLE) {}
		ConfigValue(bool val) : _bool(val), _type(ConfigValueType::BOOL) {}
		ConfigValue(SPtr<String> const& str) : _str(str), _type(ConfigValueType::STRING) {}
		ConfigValue(SPtr<Map<BasicString, Object>> obj) : _obj(newS<ObjConfig>(obj)), _type(ConfigValueType::OBJ) {}
	};
protected:
	typedef std::function<SPtr<ConfigValue>(SPtr<Object> const&)> NewConfigValue;
	static const UPtr<Map<Class, NewConfigValue>> _mapper;
	static const SPtr<ConfigValue> _nullValue;

	SPtr<Map<BasicString, Object>> _configRoot;

	UPtr<Map<BasicString, ConfigValue>> _propCache;
protected:
	template <class S>
	[[ noreturn ]] void throwConfigException(const char *where, S const* name, const char *msg) const {
		_propCache->put(newS<String>(strData(name), strLen(name)),
						newS<ConfigValue>(_HERE_, msg));
		throw ConfigException(where, msg);
	}

	template <class S>
	SPtr<ConfigValue> internalGetProperty(S const* name, std::initializer_list<int> indices) const {
		BasicStringView nameRef(strData(name), strLen(name));
		SPtr<ConfigValue> val = _propCache->get(nameRef);
		if (val) {
			switch (val->_type) {
				case ConfigValueType::NUL:
					return nullptr;
				case ConfigValueType::EXCEPTION:
					throw ConfigException(val->_where->c_str(), val->_str->c_str());
				default:
					return val;
			}

			return val;
		}

		size_t pathLen = 0;
		bool first = true;
		SPtr<Object> obj = _configRoot;
		auto crtIndex = indices.begin();
		StringSplitIterator si(name, '.');
		while (si.hasNext()) {
			BasicStringView elem = si.next();

			if (instanceof<Map<BasicString, Object>>(obj))
				obj = (Class::cast<Map<BasicString, Object>>(obj))->get(elem);
			else if (instanceof<List<Object>>(obj)) {
				try {
					size_t index;
					if ((elem.length() == 1) && (*elem.data() == '*')) {
						if (crtIndex == indices.end())
							throwConfigException(_HERE_, name, "Missing index");
						index = *crtIndex;
						crtIndex++;
					} else
						index = ULong::parseULong(CPtr(elem));
					obj = (Class::cast<List<Object>>(obj))->get(index);
				} catch (Exception const& e) {
					throwConfigException(_HERE_, name,
										 fmt::format("Caused by {} [{} ({})]",
													 e.getName(), e.getMessage(), e.where()).c_str());
				}
			} else {
				throwConfigException(_HERE_, name,
									 fmt::format("{} is not an object or array",
												 StringView(strData(name), pathLen)).c_str());
			}

			if (!obj) {
				_propCache->put(newS<String>(strData(name), strLen(name)), _nullValue);
				return nullptr;
			}
			pathLen += elem.length();
			if (!first)
				pathLen++;
			first = false;
			if (!si.hasNext()) {
				// leaf: cache and return
				SPtr<ConfigValue> val;

				SPtr<NewConfigValue> mapper = _mapper->get(obj->getClass());
				if (mapper)
					val = (*mapper)(obj);
				else {
					throwConfigException(_HERE_, name,
										 fmt::format("Unsupported config value type: {}",
													 obj->getClass().getName()).c_str());
				}

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
	ObjConfig(SPtr<Map<BasicString, Object>> const& configRoot)
	: _configRoot(configRoot)
	, _propCache(newU<HashMap<BasicString, ConfigValue>>()) {}

	virtual ~ObjConfig() {}

	template <class S>
	SPtr<String> getString(S const* name, std::initializer_list<int> indices = {}) const {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			throw MissingValueException(_HERE_, name);
		else if (val->_type == ConfigValueType::STRING)
			return val->_str;
		else
			throw ConfigException(_HERE_, "Type mismatch");
	}

	template <class S>
	SPtr<String> getString(S const* name, SPtr<String> const& defaultValue) const {
		try {
			return getString(name);
		} catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	SPtr<String> getString(S const* name, std::initializer_list<int> indices,
						   SPtr<String> const& defaultValue) const {
		try {
			return getString(name, indices);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	double getDouble(S const* name, std::initializer_list<int> indices = {}) {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			throw MissingValueException(_HERE_, name);
		switch (val->_type) {
			case ConfigValueType::DOUBLE:
				return val->_double;
			case ConfigValueType::LONG:
				return (double)val->_long;
			default:
				throw ConfigException(_HERE_, "Type mismatch");
		}
	}

	template <class S>
	double getDouble(S const* name, double defaultValue) const {
		try {
			return getDouble(name);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	double getDouble(S const* name, std::initializer_list<int> indices,
					 double defaultValue) const {
		try {
			return getDouble(name, indices);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	bool getBool(S const* name, std::initializer_list<int> indices = {}) {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			throw MissingValueException(_HERE_, name);
		switch (val->_type) {
			case ConfigValueType::BOOL:
				return val->_bool;
			default:
				throw ConfigException(_HERE_, "Type mismatch");
		}
	}

	template <class S>
	bool getBool(S const* name, bool defaultValue) const {
		try {
			return getBool(name);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	bool getBool(S const* name, std::initializer_list<int> indices,
				 bool defaultValue) const {
		try {
			return getBool(name, indices);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	int64_t getLong(S const* name, std::initializer_list<int> indices = {}) {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			throw MissingValueException(_HERE_, name);
		switch (val->_type) {
			case ConfigValueType::LONG:
				return (double)val->_long;
			default:
				throw ConfigException(_HERE_, "Type mismatch");
		}
	}

	template <class S>
	int64_t getLong(S const* name, int64_t defaultValue) const {
		try {
			return getLong(name);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	int64_t getLong(S const* name, std::initializer_list<int> indices,
					int64_t defaultValue) const {
		try {
			return getLong(name, indices);
		}  catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	SPtr<ObjConfig> getObj(S const* name,  std::initializer_list<int> indices = {}) {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			throw MissingValueException(_HERE_, name);
		switch (val->_type) {
			case ConfigValueType::OBJ:
				return val->_obj;
			default:
				throw ConfigException(_HERE_, "Type mismatch");
		}
	}
};

class Config : public ObjConfig {
protected:
	SPtr<String> _appDir;
	SPtr<String> _appName;
	SPtr<String> _confDir;
public:
	Config(SPtr<Map<BasicString, Object>> const& configRoot,
		   SPtr<String> const& appName,
		   SPtr<String> const& confDir, SPtr<String> const& appDir)
	: ObjConfig(configRoot)
	, _appDir(appDir)
	, _appName(appName)
	, _confDir(confDir) {}

	SPtr<String> getAppName() const { return _appName; }
	SPtr<String> getAppDir() const { return _appDir; }
	SPtr<String> getConfDir() const { return _confDir; }

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
