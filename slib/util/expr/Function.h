/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_FUNCTION_H
#define H_SLIB_UTIL_EXPR_FUNCTION_H

#include "slib/lang/Object.h"
#include "slib/lang/Class.h"
#include "slib/util/Iterator.h"
#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Value.h"
#include "slib/collections/ArrayList.h"

#include <functional>

namespace slib {

namespace expr {
	class Function;
}

namespace expr {

class ArgList {
protected:
	SPtr<String> _symbolName;
	ArrayList<Object> _args;
public:
	ArgList(SPtr<String> const& symbolName)
	:_symbolName(symbolName) {}

	size_t size() const {
		return _args.size();
	}

	SPtr<Object> getNullable(size_t index) const {
		if (_args.size() <= index)
			throw EvaluationException(_HERE_, fmt::format("Function {}(): invalid argument index: ", *_symbolName, index).c_str());
		return _args.get(index);
	}

	template<class T>
	SPtr<T> getNullable(size_t index) const {
		SPtr<Object> obj = getNullable(index);
		if (!obj)
			return nullptr;
		if (!classOf<T>::_class().isAssignableFrom(obj->getClass()))
			throw CastException(_HERE_, fmt::format("Function {}(): invalid parameter type: expected {}, got {}", *_symbolName, classOf<T>::_class().getName(), obj->getClass().getName()).c_str());
		return Class::cast<T>(obj);
	}

	/** @throws EvaluationException */
	SPtr<Object> get(size_t index) const {
		SPtr<Object> obj = getNullable(index);
		if (!obj)
			throw NilValueException(_HERE_);
			//throw EvaluationException(_HERE_, fmt::format("Function {}(): expected non-nil argument: {}", *_symbolName, index).c_str());
		return obj;
	}

	template<class T>
	SPtr<T> get(size_t index) const {
		SPtr<T> obj = getNullable<T>(index);
		if (!obj)
			throw NilValueException(_HERE_);
			//throw EvaluationException(_HERE_, fmt::format("Function {}(): expected non-nil argument: {}", *_symbolName, index).c_str());
		return obj;
	}

	//virtual ConstIterator<SPtr<Object>> varargIterator() const = 0;
};

class FunctionArgs : public ArgList {
public:
	FunctionArgs(SPtr<String> const& symbolName)
	: ArgList(symbolName) {}

	/** @throws CastException */
	void add(SPtr<Object> const& obj, SPtr<Function> const& function);

	Class const& peek(SPtr<Function> const& function);
};

class ParseContext;

/** @throws EvaluationException */
typedef std::function<UPtr<Value>(SPtr<Resolver> const& resolver, EvalFlags evalFlags, ArgList const& args)> Evaluate;
typedef std::function<UPtr<ParseContext>(SPtr<Function> const& function, SPtr<String> const& symbolName,
										 SPtr<Resolver> const& resolver, SPtr<String> const& tag)> NewParseContext;

UPtr<ParseContext> defaultNewParseContext(SPtr<Function> const& function, SPtr<String> const& symbolName,
										  SPtr<Resolver> const& resolver, SPtr<String> const& tag);

class Function : virtual public Object {
public:
	TYPE_INFO(Function, CLASS(Function), INHERITS(Object));
public:
	friend class ParseContext;
private:
	/** Number of fixed params */
	size_t _fixedParams;

	/** Parameter types for fixed params */
	UPtr<std::vector<Class>> _paramTypes;
protected:
	Evaluate _evaluate;
	NewParseContext _newParseContext;
public:
	/** do NOT use directly, only public for make_shared */
	Function(bool /* dontUse */, std::initializer_list<Class> argTypes, Evaluate evaluate, NewParseContext newParseContext)
	: _evaluate(evaluate)
	, _newParseContext(newParseContext) {
		auto functionParams = argTypes.size();
		if (functionParams > 0) {
			_paramTypes = newU<std::vector<Class>>();
			_paramTypes->reserve(functionParams);
			//_paramTypes->insert(_paramTypes->end(), argTypes);
			for (Class const& c : argTypes)
				_paramTypes->push_back(c);
			_fixedParams = (unsigned int)functionParams;
		} else {
			_fixedParams = 0;
			_paramTypes = nullptr;
		}
	}

	template <typename ...Args>
	static SPtr<Function> impl(Evaluate evaluate, NewParseContext newParsecontext = defaultNewParseContext) {
		return newS<Function>(true, std::initializer_list<Class>({classOf<Args>::_class()...}), evaluate, newParsecontext);
	}

	virtual ~Function() override;

	Class const& getParamType(size_t i) {
		return (i < _fixedParams) ? (*_paramTypes)[i] : classOf<Object>::_class();
	}

	size_t getFixedParamsCount() {
		return _fixedParams;
	}

	/** @throws EvaluationException; */
	UPtr<Value> evaluate(SPtr<Resolver> const& resolver, EvalFlags evalFlags, ArgList const& args) {
		return _evaluate(resolver, evalFlags, args);
	}
};

class ParseContext {
protected:
	SPtr<Function> _function;
	UPtr<FunctionArgs> _argList;
public:
	ParseContext(SPtr<Function> const& function, SPtr<String> const& symbolName,
				 SPtr<Resolver> const& resolver SLIB_UNUSED, SPtr<String> const& tag SLIB_UNUSED)
	: _function(function)
	, _argList(newU<FunctionArgs>(symbolName)) {}

	static UPtr<ParseContext> forFunction(SPtr<Function> const& function, SPtr<String> const& symbolName,
										  SPtr<Resolver> const& resolver, SPtr<String> const& tag) {
		return function->_newParseContext(function, symbolName, resolver, tag);
	}

	/** @throws CastException */
	virtual void addArg(SPtr<Object> const& obj) {
		_argList->add(obj, _function);
	}

	Class const& peekArg() {
		return _argList->peek(_function);
	}

	virtual UPtr<Value> evaluate(SPtr<Resolver> const& resolver, EvalFlags evalFlags) {
		return _function->evaluate(resolver, evalFlags, *_argList);
	}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_FUNCTION_H
