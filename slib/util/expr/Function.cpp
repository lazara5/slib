/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

Function::~Function() {}

void FunctionInstance::addArg(SPtr<Object> const& obj) {
	size_t np = _args.size();
	Class const& clazz = _function->getParamType(np);
	if ((!obj) || (clazz.isAssignableFrom(obj->getClass())))
		_args.add(obj);
	else
		throw CastException(_HERE_, fmt::format("Function {}(): invalid parameter type: expected {}, got {}", *_symbolName, clazz.getName(), obj->getClass().getName()).c_str());
}

Class const& FunctionInstance::peekArg() {
	return _function->getParamType(_args.size());
}

void FunctionInstance::readArg(SPtr<ExpressionInputStream> const& input) {
	Class const& argClass = peekArg();
	if (argClass == classOf<Lambda>::_class())
		addArg(input->readArgLambda(_function->_argSeparator, _function->_argClose));
	else {
		if (_function->_argSeparator == ',')
			addArg(ExpressionEvaluator::singleExpressionValue(input, _argResolver)->getValue());
		else
			addArg(ExpressionEvaluator::expressionValue(input, _argResolver)->getValue());
	}
}

SPtr<FunctionInstance> defaultNewFunctionInstance(SPtr<Function> const& function, SPtr<String> const& symbolName,
												  SPtr<Resolver> const& resolver) {
	return newU<FunctionInstance>(function, symbolName, resolver, resolver);
}

} // namespace expr
} // namespace slib
