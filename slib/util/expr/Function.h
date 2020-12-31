/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_FUNCTION_H
#define H_SLIB_UTIL_EXPR_FUNCTION_H

#include "slib/lang/Object.h"
#include "slib/lang/Class.h"
#include "slib/util/Iterator.h"
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
public:
	ArgList(SPtr<String> const& symbolName)
	:_symbolName(symbolName) {}

	virtual ~ArgList();

	virtual size_t size() const = 0;

	virtual SPtr<Object> getNullable(size_t index) const = 0;

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
			throw EvaluationException(_HERE_, fmt::format("Function {}(): expected non-nil argument: {}", *_symbolName, index).c_str());
		return obj;
	}

	template<class T>
	SPtr<T> get(size_t index) const {
		SPtr<T> obj = getNullable<T>(index);
		if (!obj)
			throw EvaluationException(_HERE_, fmt::format("Function {}(): expected non-nil argument: {}", *_symbolName, index).c_str());
		return obj;
	}

	//virtual ConstIterator<SPtr<Object>> varargIterator() const = 0;
};

class FunctionArgs : public ArgList {
private:
	SPtr<Function> const& _function;
	ArrayList<Object> _args;
public:
	FunctionArgs(SPtr<Function> const& function, SPtr<String> const& symbolName)
	:ArgList(symbolName)
	,_function(function) {}

	virtual ~FunctionArgs() override;

	virtual size_t size() const override {
		return _args.size();
	}

	virtual SPtr<Object> getNullable(size_t index) const override {
		if (_args.size() <= index)
			throw EvaluationException(_HERE_, fmt::format("Function {}(): invalid argument index: ", *_symbolName, index).c_str());
		return _args.get(index);
	}

	/** @throws CastException */
	void add(const SPtr<Object> &obj);

	Class const& peek();
};

/** @throws EvaluationException */
typedef std::function<UPtr<Value>(Resolver const& resolver, ArgList const& args)> Evaluate;

class Function : virtual public Object {
public:
	TYPE_INFO(Function, CLASS(Function), INHERITS(Object));
private:
	/** Number of fixed params */
	size_t _fixedParams;

	/** Parameter types for fixed params */
	UPtr<std::vector<Class>> _paramTypes;
protected:
	Evaluate _evaluate;
public:
	/** do NOT use directly, only public for make_shared */
	Function(bool /* dontUse */, std::initializer_list<Class> argTypes, Evaluate evaluate)
	:_evaluate(evaluate) {
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
	static SPtr<Function> impl(Evaluate evaluate) {
		return newS<Function>(true, std::initializer_list<Class>({classOf<Args>::_class()...}), evaluate);
	}

	virtual ~Function() override;

	Class const& getParamType(size_t i) {
		return (i < _fixedParams) ? (*_paramTypes)[i] : classOf<Object>::_class();
	}

	size_t getFixedParamsCount() {
		return _fixedParams;
	}

	/** @throws EvaluationException; */
	UPtr<Value> evaluate(Resolver const& resolver, ArgList const& args) {
		return _evaluate(resolver, args);
	}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_FUNCTION_H
