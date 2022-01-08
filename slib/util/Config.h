/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_CONFIG_H
#define H_SLIB_UTIL_CONFIG_H

#include "slib/lang/String.h"
#include "slib/util/StringUtils.h"
#include "slib/lang/Numeric.h"
#include "slib/collections/LinkedList.h"
#include "slib/collections/Properties.h"
#include "slib/util/expr/Resolver.h"
#include "slib/util/SystemInfo.h"
#include "slib/util/expr/Expression.h"

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

	virtual SPtr<Object> getVar(String const& key, expr::ValueDomain valueDomain) const override {
		return _resolver->getVar(key, valueDomain);
	}
};

enum struct ConfigValueType {
	LONG,
	DOUBLE,
	BOOL,
	STRING,
	OBJ,
	ARRAY
};

class ConfigObj {
protected:
	struct ConfigValue {
		union {
			int64_t _long;
			double _double;
			bool _bool;
		};
		SPtr<String> _str;
		SPtr<ConfigObj> _obj;
		ConfigValueType _type;

		ConfigValue(int64_t val) : _long(val), _type(ConfigValueType::LONG) {}
		ConfigValue(double val) : _double(val), _type(ConfigValueType::DOUBLE) {}
		ConfigValue(bool val) : _bool(val), _type(ConfigValueType::BOOL) {}
		ConfigValue(SPtr<String> const& str) : _str(str), _type(ConfigValueType::STRING) {}
		ConfigValue(SPtr<Map<IString, Object>> obj) : _obj(newS<ConfigObj>(obj)), _type(ConfigValueType::OBJ) {}
		ConfigValue(SPtr<ArrayList<Object>> obj) : _obj(newS<ConfigObj>(obj)), _type(ConfigValueType::ARRAY) {}
	};
protected:
	typedef std::function<UPtr<ConfigValue>(SPtr<Object> const&)> NewConfigValue;
	static const UPtr<Map<Class, NewConfigValue>> _mapper;

	SPtr<Object> _configRoot;
protected:
	template <class S>
	UPtr<ConfigValue> internalGetProperty(S const* path, std::initializer_list<int> indices) const {
		size_t pathLen = 0;
		bool first = true;
		SPtr<Object> obj = _configRoot;
		auto crtIndex = indices.begin();
		StringSplitIterator si(path, '.');
		while (si.hasNext()) {
			BasicStringView elem = si.next();

			if (instanceof<Map<IString, Object>>(obj))
				obj = (Class::cast<Map<IString, Object>>(obj))->get(elem);
			else if (instanceof<List<Object>>(obj)) {
				try {
					size_t index;
					if ((elem.length() == 2) && (elem.data()[0] == '[') && (elem.data()[1] == ']')) {
						if (crtIndex == indices.end())
							throw ConfigException(_HERE_, "Missing index");
						index = *crtIndex;
						crtIndex++;
					} else
						index = ULong::parseULong(elem);
					obj = (Class::cast<List<Object>>(obj))->get(index);
				} catch (Exception const& e) {
					THROW(ConfigException, fmt::format("Caused by {} [{} ({})]",
													   e.getName(), e.getMessage(), e.where()).c_str());
				}
			} else {
				THROW(ConfigException, fmt::format("{} is not an object or array",
												   StringView(strData(path), pathLen)).c_str());
			}

			if (!obj)
				return nullptr;

			pathLen += elem.length();
			if (!first)
				pathLen++;
			first = false;
			if (!si.hasNext()) {
				// leaf: cache and return
				UPtr<ConfigValue> val;

				SPtr<NewConfigValue> mapper = _mapper->get(obj->getClass());
				if (mapper)
					val = (*mapper)(obj);
				else {
					throw ConfigException(_HERE_, fmt::format("Unsupported config value type: {}",
															  obj->getClass().getName()).c_str());
				}

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
	ConfigObj(SPtr<Map<IString, Object>> const& configRoot)
	: _configRoot(configRoot) {}

	ConfigObj(SPtr<ArrayList<Object>> const& configRoot)
	: _configRoot(configRoot) {}

	virtual ~ConfigObj() {}

	SPtr<Object> getRoot() {
		return _configRoot;
	}

	template <class S>
	SPtr<String> getString(S const* name, std::initializer_list<int> indices = {}) const {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			THROW(MissingValueException, name);
		else if (val->_type == ConfigValueType::STRING)
			return val->_str;
		else
			THROW(ConfigException, "Type mismatch");
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
		} catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	double getDouble(S const* name, std::initializer_list<int> indices = {}) {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			THROW(MissingValueException, name);
		switch (val->_type) {
			case ConfigValueType::DOUBLE:
				return val->_double;
			case ConfigValueType::LONG:
				return (double)val->_long;
			default:
				THROW(ConfigException, "Type mismatch");
		}
	}

	template <class S>
	double getDouble(S const* name, double defaultValue) const {
		try {
			return getDouble(name);
		} catch (MissingValueException const&) {
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
				THROW(ConfigException, "Type mismatch");
		}
	}

	template <class S>
	bool getBool(S const* name, bool defaultValue) const {
		try {
			return getBool(name);
		} catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	template <class S>
	bool getBool(S const* name, std::initializer_list<int> indices,
				 bool defaultValue) const {
		try {
			return getBool(name, indices);
		} catch (MissingValueException const&) {
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
	SPtr<ConfigObj> getObj(S const* name,  std::initializer_list<int> indices = {}) {
		SPtr<ConfigValue> val = internalGetProperty(name, indices);
		if (!val)
			THROW(MissingValueException, name);
		switch (val->_type) {
			case ConfigValueType::OBJ:
				return val->_obj;
			default:
				throw ConfigException(_HERE_, "Type mismatch");
		}
	}
};

class Config : public ConfigObj {
protected:
	SPtr<String> _appDir;
	SPtr<String> _appName;
	SPtr<String> _confDir;
public:
	Config(SPtr<Map<IString, Object>> const& configRoot,
		   SPtr<String> const& appName,
		   SPtr<String> const& confDir, SPtr<String> const& appDir)
	: ConfigObj(configRoot)
	, _appDir(appDir)
	, _appName(appName)
	, _confDir(confDir) {}

	SPtr<String> getAppName() const { return _appName; }
	SPtr<String> getAppDir() const { return _appDir; }
	SPtr<String> getConfDir() const { return _confDir; }
};

enum class OptionType : uint8_t {
	UNKNOWN,
	STRING,
	LONG,
	DOUBLE,
	BOOL,
	ARRAY,
	OBJ
};

class ConfigLoader {
protected:
	class ConfigResolver : public expr::Resolver {
	private:
		SPtr<String> _exeDir;
		SPtr<String> _cwd;
	public:
		ConfigResolver();

		virtual SPtr<Object> getVar(String const& key, expr::ValueDomain domain) const override;
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

protected:
	class Constraint {
	public:
		OptionType _type;
		UPtr<expr::Expression> _defaultExpr;
	protected:
		Constraint(OptionType type)
		: _type(type) {}
	public:
		virtual ~Constraint() {}

		bool operator==(Constraint const& other) const {
			return (this == &other);
		}

		template <class S>
		void setDefault(S const& defaultExpr) {
			_defaultExpr = newU<expr::Expression>(newS<String>(defaultExpr));
		}

		virtual void range(int64_t min, int64_t max) = 0;
		virtual void range(double min, double max) = 0;

		/** @throws InitException */
		virtual void validateValue(SPtr<Object> const& obj) = 0;
	};

	class StringConstraint : public Constraint {
	public:
		StringConstraint()
		: Constraint(OptionType::STRING) {}

		virtual void range(int64_t, int64_t) override {
			throw InitException(_HERE_, "Cannot apply range to string option");
		}

		virtual void range(double, double) override {
			throw InitException(_HERE_, "Cannot apply range to string option");
		}

		virtual void validateValue(SPtr<Object> const& obj) override;
	};

	class LongConstraint : public Constraint {
	protected:
		int64_t _min;
		int64_t _max;
	public:
		LongConstraint()
		: Constraint(OptionType::LONG)
		, _min(Long::MIN_VALUE)
		, _max(Long::MAX_VALUE) {}

		virtual void range(int64_t min, int64_t max) override {
			_min = min;
			_max = max;
		}

		virtual void range(double, double) override {
			throw InitException(_HERE_, "Cannot apply double range to long option");
		}

		virtual void validateValue(SPtr<Object> const& obj) override;
	};

	class DoubleConstraint : public Constraint {
	protected:
		double _min;
		double _max;
	public:
		DoubleConstraint()
		: Constraint(OptionType::DOUBLE)
		, _min(Double::MIN_VALUE)
		, _max(Double::MAX_VALUE) {}

		virtual void range(int64_t min, int64_t max) override {
			_min = min;
			_max = max;
		}

		virtual void range(double min, double max) override {
			_min = min;
			_max = max;
		}

		virtual void validateValue(SPtr<Object> const& obj) override;
	};

	class BoolConstraint : public Constraint {
	public:
		BoolConstraint()
		: Constraint(OptionType::BOOL) {}

		virtual void range(int64_t, int64_t) override {
			throw InitException(_HERE_, "Cannot apply range to bool option");
		}

		virtual void range(double, double) override {
			throw InitException(_HERE_, "Cannot apply range to bool option");
		}

		virtual void validateValue(SPtr<Object> const& obj) override;
	};

	class ObjectConstraint : public Constraint {
	public:
		ObjectConstraint()
		: Constraint(OptionType::OBJ) {}

		virtual void range(int64_t, int64_t) override {
			throw InitException(_HERE_, "Cannot apply range to obj option");
		}

		virtual void range(double, double) override {
			throw InitException(_HERE_, "Cannot apply range to obj option");
		}

		virtual void validateValue(SPtr<Object> const& obj) override;
	};

	class ArrayConstraint : public Constraint {
	public:
		ArrayConstraint()
		: Constraint(OptionType::ARRAY) {}

		virtual void range(int64_t, int64_t) override {
			throw InitException(_HERE_, "Cannot apply range to array option");
		}

		virtual void range(double, double) override {
			throw InitException(_HERE_, "Cannot apply range to array option");
		}

		virtual void validateValue(SPtr<Object> const& obj) override;
	};

	class UnknownConstraint : public Constraint {
	public:
		UnknownConstraint()
		: Constraint(OptionType::UNKNOWN) {}

		virtual void range(int64_t, int64_t) override {
			THROW(InitException, "Cannot apply range to <unknown> option");
		}

		virtual void range(double, double) override {
			THROW(InitException, "Cannot apply range to <unknown> option");
		}

		virtual void validateValue(SPtr<Object> const& obj) override;
	};

	typedef std::function<UPtr<Constraint>()> ConstraintConstructor;

	class ConstraintNode {
	public:
		UPtr<Constraint> _constraint;
		UPtr<Map<IString, ConstraintNode>> _children;
	protected:
		void validateChildren(const SPtr<Object> &obj, const SPtr<expr::Resolver> &resolver);
	public:
		ConstraintNode(UPtr<Constraint> constraint)
		: _constraint(std::move(constraint))
		, _children(newU<HashMap<IString, ConstraintNode>>()) {}

		/** @throws InitException */
		void checkOrSetType(OptionType type);

		SPtr<Object> validate(SPtr<Object> const& obj, SPtr<expr::Resolver> const& resolver);
	};

	typedef struct _OptionTypeDesc {
		UPtr<String> _name;
		ConstraintConstructor _constructor;
	} OptionTypeDesc;

	static const OptionTypeDesc _optionTypeDescTable[];
	static const size_t _optionTypeDescTableSize;

	/*class ConstraintNode {
	public:
		SPtr<Constraint> _constraint;
		UPtr<Map<IString, ConstraintNode>> _children;
	public:
		ConstraintNode(SPtr<Constraint> const& constraint)
		: _constraint(constraint)
		, _children(newU<HashMap<IString, ConstraintNode>>()) {}

		bool operator==(ConstraintNode const& other) const {
			return (this == &other);
		}

		// @throws InitException
		void checkOrSetType(OptionType type);
	};*/

	SPtr<String> _confFileName;
	SPtr<String> _appName;

	SPtr<Map<String, Object>> _vars;
	SPtr<expr::ChainedResolver> _quickResolver;
	SPtr<expr::ChainedResolver> _resolver;
	LinkedList<StringPair> _searchPaths;

	SPtr<ConstraintNode> _rootConstraint;
	SPtr<ConstraintNode> _crtOption;
protected:
	SPtr<StringPair> searchConfigDir();

	void validate();

	void map(SPtr<Object> cfgObj, Field *field, ObjRef const& obj);

	SPtr<Map<IString, Object> > internalLoad(UPtr<Array<uint8_t>> const& contents, bool quick);
public:
	ConfigLoader(SPtr<String> const& confFileName, SPtr<String> const& appName);

	ConfigLoader& clearPaths();

	ConfigLoader& search(SPtr<String> const& path, SPtr<String> const& rootDir = nullptr);

	ConfigLoader& withResolver(SPtr<expr::Resolver> const& resolver);

	ConfigLoader& withResolver(SPtr<String> const& name, SPtr<expr::Resolver> const& resolver);

	template <class S>
	ConfigLoader& option(S const& path, OptionType optionType) {
		_crtOption = nullptr;

		StringSplitIterator si(path, '.');
		SPtr<ConstraintNode> parent = _rootConstraint;
		while (si.hasNext()) {
			BasicStringView elem = si.next();

			OptionType parentType = StringView::equals(elem, "[]") ? OptionType::ARRAY : OptionType::OBJ;
			parent->checkOrSetType(parentType);

			if (si.hasNext()) { // node
				SPtr<ConstraintNode> oldParent = parent;
				parent = oldParent->_children->get(elem);
				if (!parent) {
					parent = newS<ConstraintNode>(newU<UnknownConstraint>());
					oldParent->_children->put(newS<String>(elem), parent);
				}
			} else { // leaf
				_crtOption = newS<ConstraintNode>(_optionTypeDescTable[(size_t)optionType]._constructor());
				if (parent->_children->containsKey(elem))
					THROW(InitException, fmt::format("Attempted to redefine option '{}'", path).c_str());
				parent->_children->put(newS<String>(elem), _crtOption);
			}
		}

		return *this;
	}

	ConfigLoader& range(int64_t min, int64_t max);

	ConfigLoader& range(int min, int max) {
		return range((int64_t)min, (int64_t)max);
	}

	ConfigLoader& range(double min, double max);

	template <class S>
	ConfigLoader& defaultValue(S const& defaultExpr) {
		if (!_crtOption)
			THROW(InitException, "Default can only be applied to an option");
		_crtOption->_constraint->setDefault(defaultExpr);
		return *this;
	}

	/** @throws EvaluationException */
	SPtr<Config> load(bool quick = false);

	/** @throws EvaluationException */
	template <class S>
	SPtr<Config> load(S const& contents, bool quick = false) {
		String strContents(contents);
		UPtr<Array<uint8_t>> contentBytes = strContents.getBytes();

		SPtr<Map<IString, Object>> configRoot = internalLoad(contentBytes, quick);
		_rootConstraint->validate(configRoot, quick ? _quickResolver : _resolver);

		UPtr<String> s = configRoot->toString();
		fmt::print("Config: {}\n", *s);

		return newS<Config>(configRoot, _appName, nullptr, nullptr);
	}

	template <class T>
	SPtr<T> toObject(SPtr<Config> cfg) {
		SPtr<T> obj = newS<T>();
		map(cfg->getRoot(), nullptr, ObjRef(obj.get(), RefType::INSTANCE, classOf<T>::_class()));
		return obj;
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_CONFIG_H
