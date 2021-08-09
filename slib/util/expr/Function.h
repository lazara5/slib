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

/** @throws EvaluationException */
typedef std::function<UPtr<Value>(SPtr<Resolver> const& resolver, ArgList const& args)> Evaluate;

class FunctionInstance;

typedef std::function<SPtr<FunctionInstance>(SPtr<Function> const& function, SPtr<String> const& symbolName,
											 SPtr<Resolver> const& resolver)> NewFunctionInstance;

SPtr<FunctionInstance> defaultNewFunctionInstance(SPtr<Function> const& function, SPtr<String> const& symbolName,
												  SPtr<Resolver> const& resolver);

class Function : public std::enable_shared_from_this<Function>, virtual public Object {
public:
	TYPE_INFO(Function, CLASS(Function), INHERITS(Object));
public:
	const char _peekOverride;
	const char _argSeparator;
	const char _argClose;
private:
	const NewFunctionInstance _newFunctionInstance;

	/** Number of fixed params */
	size_t _fixedParams;

	/** Parameter types for fixed params */
	UPtr<std::vector<Class>> _paramTypes;
protected:
	Evaluate _evaluate;

public:
	/** do NOT use directly, only public for make_shared */
	Function(bool /* dontUse */, std::initializer_list<Class> argTypes, Evaluate evaluate,
			 NewFunctionInstance newFunctionInstance,
			 char peekOverride, char argSeparator, char argClose) 
	: _peekOverride(peekOverride)
	, _argSeparator(argSeparator)
	, _argClose(argClose)
	, _newFunctionInstance(newFunctionInstance)
	, _evaluate(evaluate) {
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
	static SPtr<Function> impl(Evaluate evaluate,
							   NewFunctionInstance newFunctionInstance = defaultNewFunctionInstance,
							   char peekOverride = '\0',
							   char argSeparator = ',',
							   char argClose = ')') {
		return newS<Function>(true, std::initializer_list<Class>({classOf<Args>::_class()...}), evaluate,
							  newFunctionInstance,
							  peekOverride, argSeparator, argClose);
	}

	virtual ~Function() override;

	SPtr<FunctionInstance> newInstance(SPtr<String> const& symbolName,
									   SPtr<Resolver> const& resolver) {
		return _newFunctionInstance(shared_from_this(), symbolName, resolver);
	}

	Class const& getParamType(size_t i) {
		return (i < _fixedParams) ? (*_paramTypes)[i] : classOf<Object>::_class();
	}

	size_t getFixedParamsCount() {
		return _fixedParams;
	}

	/** @throws EvaluationException; */
	UPtr<Value> evaluate(SPtr<Resolver> const& resolver, ArgList const& args) {
		return _evaluate(resolver, args);
	}
};

class FunctionInstance : public ArgList {
protected:
	SPtr<Function> _function;
	SPtr<Resolver> _evalResolver;
	SPtr<Resolver> _argResolver;
protected:
	Class const& peekArg();

	/** @throws CastException */
	virtual void addArg(SPtr<Object> const& obj);
public:
	FunctionInstance(SPtr<Function> const& function, SPtr<String> const& symbolName,
					 SPtr<Resolver> evalResolver, SPtr<Resolver> argResolver)
	: ArgList(symbolName)
	, _function(function)
	, _evalResolver(evalResolver)
	, _argResolver(argResolver) {}

	void readArg(SPtr<ExpressionInputStream> const& input);

	virtual UPtr<Value> evaluate() {
		return _function->evaluate(_evalResolver, *this);
	}
};


} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_FUNCTION_H
