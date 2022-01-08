/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/String.h"
#include "slib/util/Config.h"
#include "slib/util/Log.h"
#include "slib/util/FileUtils.h"
#include "slib/util/FilenameUtils.h"
#include "slib/collections/ArrayList.h"
#include "slib/io/FileInputStream.h"
#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/lang/Reflection.h"

#include <iostream>
#include <fstream>
#include <string>

#define PROCSELFEXE "/proc/self/exe"

namespace slib {

using namespace expr;

const UPtr<Map<Class, ConfigObj::NewConfigValue>> ConfigObj::_mapper
	= newU<HashMap<Class, ConfigObj::NewConfigValue>, std::initializer_list<std::pair<SPtr<Class>, SPtr<ConfigObj::NewConfigValue>>>>({
	{
		newS<Class>(classOf<String>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newU<ConfigValue>(Class::cast<String>(obj));
		})
	},
	{
		newS<Class>(classOf<Long>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newU<ConfigValue>(Class::cast<Long>(obj)->longValue());
		})
	},
	{
		newS<Class>(classOf<Double>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newU<ConfigValue>(Class::cast<Double>(obj)->doubleValue());
		})
	},
	{
		newS<Class>(classOf<Boolean>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newU<ConfigValue>(Class::cast<Boolean>(obj)->booleanValue());
		})
	},
	{
		newS<Class>(classOf<Map<IString, Object>>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newU<ConfigValue>(Class::cast<Map<IString, Object>>(obj));
		})
	},
	{
		newS<Class>(classOf<ArrayList<Object>>::_class()),
		newS<Config::NewConfigValue>([](SPtr<Object> const& obj) {
			return newU<ConfigValue>(Class::cast<ArrayList<Object>>(obj));
		})
	},
});

const ConfigLoader::OptionTypeDesc ConfigLoader::_optionTypeDescTable[] = {
	{"UNKNOWN"_UPTR,	[](){ return newU<ConfigLoader::UnknownConstraint>(); } },
	{"STRING"_UPTR,		[](){ return newU<ConfigLoader::StringConstraint>(); } },
	{"LONG"_UPTR,		[](){ return newU<ConfigLoader::LongConstraint>(); } },
	{"DOUBLE"_UPTR,		[](){ return newU<ConfigLoader::DoubleConstraint>(); } },
	{"BOOL"_UPTR,		[](){ return newU<ConfigLoader::BoolConstraint>(); } },
	{"ARRAY"_UPTR,		[](){ return newU<ConfigLoader::ArrayConstraint>(); } },
	{"OBJ"_UPTR,		[](){ return newU<ConfigLoader::ObjectConstraint>(); } }
};

const size_t ConfigLoader::_optionTypeDescTableSize = sizeof(ConfigLoader::_optionTypeDescTable) / sizeof(OptionTypeDesc);

ConfigLoader::ConfigResolver::ConfigResolver() {
	char pathBuf[PATH_MAX] = "";
	struct stat st;

	if (stat(PROCSELFEXE, &st) < 0)
		perror("stat " PROCSELFEXE);
	else if (readlink(PROCSELFEXE, pathBuf, sizeof(pathBuf)) < 0) {
		perror("readlink " PROCSELFEXE);
		pathBuf[0] = '\0';
	}

	if (pathBuf[0] != '\0') {
		char *r = strrchr(pathBuf, '/');
		if (r)
			*r = '\0';
		_exeDir = newS<String>(pathBuf);
	}

	if (getcwd(pathBuf, sizeof(pathBuf)) != nullptr)
		_cwd = newS<String>(pathBuf);
}

SPtr<Object> ConfigLoader::ConfigResolver::getVar(String const& key, ValueDomain domain) const {
	if (domain == ValueDomain::LOCAL)
		return nullptr;

	if (StringView::equals(key, "_EXEDIR"_SV)) {
		return _exeDir;
	} else if (StringView::equals(key, "_CWD"_SV)) {
		return _cwd;
	}

	return nullptr;
}

void ConfigLoader::StringConstraint::validateValue(SPtr<Object> const& obj) {
	if (!instanceof<String>(obj))
		throw InitException(_HERE_, fmt::format("Invalid option type: expected String, got {}", obj->getClass().getName()).c_str());
}

void ConfigLoader::ConstraintNode::checkOrSetType(OptionType type) {
	if (_constraint->_type == OptionType::UNKNOWN) {
		switch(type) {
			case OptionType::OBJ:
				_constraint = newU<ObjectConstraint>();
				_constraint->setDefault("{}");
				break;
			case OptionType::ARRAY:
				_constraint = newU<ArrayConstraint>();
				_constraint->setDefault("[]");
				break;
			default:
				THROW(InitException, "Non-leaf option nodes can only be objects or arrays");
		}
	}

	if (_constraint->_type != type)
		THROW(InitException, "Option type mismatch");
}

void ConfigLoader::ConstraintNode::validateChildren(SPtr<Object> const& obj, SPtr<Resolver> const& resolver) {
	if (_children->size() > 0) {
		switch (_constraint->_type) {
			case OptionType::OBJ: {
				SPtr<Map<IString, Object>> objMap = Class::cast<Map<IString, Object>>(obj);
				auto it = _children->constIterator();
				while (it->hasNext()) {
					auto &constraintEntry = it->next();
					SPtr<IString> objKey = constraintEntry.getKey();
					SPtr<ConstraintNode> constraintNode = constraintEntry.getValue();
					SPtr<Object> member = objMap->get(*objKey);
					SPtr<Object> validatedMember = constraintNode->validate(member, resolver);
					if (validatedMember)
						objMap->put(objKey, validatedMember);
				}
				break;
			}
			case OptionType::ARRAY: {
				SPtr<ArrayList<Object>> objArray = Class::cast<ArrayList<Object>>(obj);
				SPtr<ConstraintNode> constraintNode = _children->get("[]"_IS);
				auto it = objArray->constIterator();
				while (it->hasNext()) {
					SPtr<Object> item = it->next();
					constraintNode->validate(item, resolver);
				}
				break;
			}
			default:
				THROW(InitException, "Only objects and arrays can have child constraints");
		}
	}
}

SPtr<Object> ConfigLoader::ConstraintNode::validate(SPtr<Object> const& obj, SPtr<Resolver> const& resolver) {
	if (obj) {
		_constraint->validateValue(obj);
		validateChildren(obj, resolver);
		return nullptr;
	} else {
		if (!_constraint->_defaultExpr)
			throw InitException(_HERE_, "Missing mandatory value");
		try {
			UPtr<Value> defaultValue = _constraint->_defaultExpr->evaluate(resolver);
			SPtr<Object> value = defaultValue->getValue();
			_constraint->validateValue(value);
			validateChildren(value, resolver);
			return value;
		} catch (EvaluationException const& e) {
			throw InitException(e.where(), fmt::format("Error evaluating default expression: {}", e.getMessage()).c_str());
		}
	}
}


ConfigLoader::ConfigLoader(SPtr<String> const& confFileName, SPtr<String> const& appName)
: _confFileName(confFileName)
, _appName(appName)
, _vars(newS<HashMap<String, Object>>())
, _quickResolver(expr::ChainedResolver::newInstance())
, _resolver(expr::ChainedResolver::newInstance())
, _rootConstraint(newS<ConstraintNode>(newU<ObjectConstraint>())) {
	(*_quickResolver).add(newS<ConfigResolver>())
	.add("env"_SPTR, newS<EnvResolver>());

	(*_resolver)
		.add(_quickResolver)
		.add(_vars, ValueDomain::LOCAL, Resolver::Mode::WRITABLE);

	_searchPaths.add(newS<StringPair>("/etc"_SPTR, nullptr));
	_searchPaths.add(newS<StringPair>("${_EXEDIR}/conf"_SPTR, "${_EXEDIR}"_SPTR));
	_searchPaths.add(newS<StringPair>("${_CWD}/conf"_SPTR, "${_CWD}"_SPTR));
}

ConfigLoader& ConfigLoader::clearPaths() {
	_searchPaths.clear();
	_crtOption = nullptr;
	return *this;
}

ConfigLoader& ConfigLoader::search(SPtr<String> const& path, SPtr<String> const& rootDir /* = nullptr */) {
	_searchPaths.add(newS<StringPair>(path, rootDir));
	_crtOption = nullptr;
	return *this;
}

ConfigLoader& ConfigLoader::withResolver(SPtr<expr::Resolver> const& resolver) {
	_resolver->add(resolver);
	_crtOption = nullptr;
	return *this;
}

ConfigLoader& ConfigLoader::withResolver(SPtr<String> const& name, SPtr<expr::Resolver> const& resolver) {
	_resolver->add(name, resolver);
	_crtOption = nullptr;
	return *this;
}

void ConfigLoader::LongConstraint::validateValue(SPtr<Object> const& obj) {
	if (!instanceof<Long>(obj))
		THROW(InitException, fmt::format("Invalid option type: expected Long, got {}", obj->getClass().getName()).c_str());
	int64_t val = Class::cast<Long>(obj)->longValue();
	if ((val < _min) || (val > _max))
		throw InitException(_HERE_, fmt::format("Value {} out of range [{} .. {}]", val, _min, _max).c_str());
}

void ConfigLoader::DoubleConstraint::validateValue(SPtr<Object> const& obj) {
	if (!instanceof<Double>(obj))
		THROW(InitException, fmt::format("Invalid option type: expected Double, got {}", obj->getClass().getName()).c_str());
	double val = Class::cast<Double>(obj)->doubleValue();
	if ((val < _min) || (val > _max))
		throw InitException(_HERE_, fmt::format("Value {} out of range [{} .. {}]", val, _min, _max).c_str());
}

void ConfigLoader::BoolConstraint::validateValue(SPtr<Object> const& obj) {
	if (!instanceof<Boolean>(obj))
		THROW(InitException, fmt::format("Invalid option type: expected Boolean, got {}", obj->getClass().getName()).c_str());
}

void ConfigLoader::ObjectConstraint::validateValue(SPtr<Object> const& obj) {
	if (!instanceof<Map<IString, Object>>(obj))
		throw InitException(_HERE_, fmt::format("Invalid option type: expected Map<IString, Object>, got {}", obj->getClass().getName()).c_str());
}

void ConfigLoader::ArrayConstraint::validateValue(SPtr<Object> const& obj) {
	if (!instanceof<ArrayList<Object>>(obj))
		throw InitException(_HERE_, fmt::format("Invalid option type: expected ArrayList<Object>, got {}", obj->getClass().getName()).c_str());
}

void ConfigLoader::UnknownConstraint::validateValue(SPtr<Object> const& obj SLIB_UNUSED) {
	THROW(InitException, "Invalid option type: <unknown>");
}

ConfigLoader& ConfigLoader::range(int64_t min, int64_t max) {
	if (!_crtOption)
		throw InitException(_HERE_, "Range can only be applied to an option");
	_crtOption->_constraint->range(min, max);
	return *this;
}

ConfigLoader& ConfigLoader::range(double min, double max) {
	if (!_crtOption)
		throw InitException(_HERE_, "Range can only be applied to an option");
	_crtOption->_constraint->range(min, max);
	return *this;
}

/** @throws EvaluationException */
SPtr<ConfigLoader::StringPair> ConfigLoader::searchConfigDir() {
	UPtr<ConstIterator<SPtr<StringPair>>> i = _searchPaths.constIterator();
	while (i->hasNext()) {
		SPtr<StringPair> const& cfgEntry = i->next();
		UPtr<String> dir = ExpressionEvaluator::interpolate(*cfgEntry->_first, _quickResolver, true);
		if (dir) {
			UPtr<String> fileName = FilenameUtils::concat(dir, _confFileName);
			if (access(fileName->c_str(), 0) == 0) {
				UPtr<String> rootDir = (cfgEntry->_second)
					? ExpressionEvaluator::interpolate(*cfgEntry->_first, _quickResolver, true)
					: nullptr;
				return newS<StringPair>(std::move(dir), std::move(rootDir));
			}
		}
	}

	return nullptr;
}

void ConfigLoader::map(SPtr<Object> cfgObj, Field *field, ObjRef const& obj) {
	if (instanceof<Map<IString, Object>>(cfgObj)) {
		auto map = Class::cast<Map<IString, Object>>(cfgObj.get());
		auto i = map->constIterator();
		while (i->hasNext()) {
			auto const& e = i->next();
			IString const* name = e.getKeyPtr();
			try {
				SPtr<Field> objField = obj._class.getDeclaredField(name);
				SPtr<Object> fieldValue = e.getValue();
				if (classOf<Map<IString, Object>>::_class().isAssignableFrom(fieldValue->getClass())) {
					ObjRef fieldObj = objField->getRef(obj);
					this->map(fieldValue, objField.get(), fieldObj);
				} else
					this->map(fieldValue, objField.get(), obj);
			} catch (NoSuchFieldException const&) {
			}
		}
	} else if (instanceof<ArrayList<Object>>(cfgObj)) {
		Class const& fieldType = field->getType();
		if (fieldType.isArray()) {
			auto array = Class::cast<ArrayList<Object>>(cfgObj.get());
			auto i = array->constIterator();
		} else
			THROW(IllegalArgumentException, "Object field is not an array");
	} else {
		if (!field)
			THROW(IllegalArgumentException);
		field->set(obj, cfgObj);
	}
}

SPtr<Map<IString, Object>> ConfigLoader::internalLoad(UPtr<Array<uint8_t>> const& contents, bool quick) {
	try {
		StringBuilder configContents("{");
		configContents.add(*contents)
			.add('}');
		SPtr<String> configText = configContents.toString();

		_vars->clear();
		SPtr<Object> parsedConfig = ExpressionEvaluator::expressionValue(configText,
			quick ? _quickResolver : _resolver);
		if (!parsedConfig)
			throw InitException(_HERE_, "Error parsing config");
		if (!instanceof<Map<IString, Object>>(*parsedConfig))
			throw InitException(_HERE_, fmt::format("Invalid config: expected Map<String, Object>, got {}", parsedConfig->getClass().getName()).c_str());
		SPtr<Map<IString, Object>> configRoot = Class::cast<Map<IString, Object>>(parsedConfig);

		_rootConstraint->validate(configRoot, quick ? _quickResolver : _resolver);

		UPtr<String> s = configRoot->toString();
		fmt::print("Config: {}\n", *s);

		return configRoot;
	} catch (EvaluationException const& e) {
		THROW(InitException, e);
	}
}

SPtr<Config> ConfigLoader::load(bool quick) {
	try {
		SPtr<StringPair> conf = searchConfigDir();
		if (!conf)
			throw InitException(_HERE_, fmt::format("Could not locate config file '{}'", *_confFileName).c_str());

		SPtr<String> confDir(std::move(conf->_first));
		SPtr<String> appDir(std::move(conf->_second));

		UPtr<String> fileName = FilenameUtils::concat(confDir, _confFileName);

		UPtr<Array<uint8_t>> contents = FileUtils::readAllBytes(fileName);

		SPtr<Map<IString, Object>> configRoot = internalLoad(contents, quick);
		_rootConstraint->validate(configRoot, quick ? _quickResolver : _resolver);

		UPtr<String> s = configRoot->toString();
		fmt::print("Config: {}\n", *s);

		return newS<Config>(configRoot, _appName, confDir, appDir);
	} catch (IOException const& e) {
		throw InitException(_HERE_, e);
	}
}

/** @throws InitException */
/*void Config::internalInit(bool minimal) {
	char pathBuf[1024] = "";
	struct stat s;

	if (stat(PROCSELFEXE, &s) < 0)
		perror("stat " PROCSELFEXE);
	else if (readlink(PROCSELFEXE, pathBuf, sizeof(pathBuf)) < 0) {
		perror("readlink " PROCSELFEXE);
		pathBuf[0] = '\0';
	}

	if (pathBuf[0] == '\0') {
		// cannot get exe dir, attempt to use current directory
		if (getcwd(pathBuf, sizeof(pathBuf)) == nullptr)
			throw InitException(_HERE_, fmt::format("getcwd() failed, errno={:d}", errno).c_str());
	} else {
		char *r = strrchr(pathBuf, '/');
		if (r)
			*r = '\0';
	}
	char *r = strrchr(pathBuf, '/');
		if (r)
			*r = '\0';

	_rootDir = pathBuf;

	SPtr<String> configDir = locateConfigFile(_confFileName);
	if (!configDir)
		throw InitException(_HERE_, fmt::format("Could not locate config file '{}'", _confFileName).c_str());

	_confDir = configDir;

	clear();
	openConfigFile(minimal);
	//_vars.forEach(&dmp, nullptr);
	readConfig();
}

void Config::openConfigFile(bool minimal) {
	UPtr<String> configFile = FileUtils::buildPath(CPtr(_confDir), CPtr(_confFileName));
	try {
		FileInputStream propsFile(CPtr(configFile));
		if (minimal)
			load(propsFile, &_simpleCfgProc);
		else
			load(propsFile, &_cfgProc);
	} catch (IOException const& e) {
		throw InitException(_HERE_, fmt::format("I/O error reading config file '{}': {}", *configFile, e.getMessage()).c_str());
	}
}

void Config::shutdown() {
	Log::shutdown();
}

void Config::registerPropertySink(String const& name, ConfigProcessor::PropertySink const& sink) {
	_cfgProc.registerSink(name, sink);
}*/

} // namespace
