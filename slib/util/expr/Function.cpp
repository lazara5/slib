/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

UPtr<FunctionParseContext> defaultNewParseContext(SPtr<Function> const& function, SPtr<String> const& symbolName,
										  SPtr<Resolver> const& resolver, const UPtr<ExpressionContext> &ctx SLIB_UNUSED) {
	return newU<FunctionParseContext>(function, symbolName, resolver);
}

Function::~Function() {}

void FunctionArgs::add(SPtr<Object> const& obj, SPtr<Function> const& function) {
	size_t np = _args.size();
	Class const& clazz = function->getParamType(np);
	if ((!obj) || (clazz.isAssignableFrom(obj->getClass())))
		_args.add(obj);
	else
		throw CastException(_HERE_, fmt::format("Function {}(): invalid parameter type: expected {}, got {}", *_symbolName, clazz.getName(), obj->getClass().getName()).c_str());
}

Class const& FunctionArgs::peek(SPtr<Function> const& function) {
	return function->getParamType(_args.size());
}

} // namespace expr
} // namespace slib
