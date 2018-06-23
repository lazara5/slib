/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

Function::~Function() {}

constexpr Class Function::_class;

ArgList::~ArgList() {}

FunctionArgs::~FunctionArgs() {}

void FunctionArgs::add(SPtr<Object> const& obj) {
	size_t np = _args.size();
	Class const& clazz = _function->getParamType(np);
	if ((!obj) || (clazz.isAssignableFrom(obj->getClass())))
		_args.add(obj);
	else
		throw CastException(_HERE_, fmt::format("Function {}(): invalid parameter type: expected {}, got {}", *_symbolName, clazz.getName(), obj->getClass().getName()).c_str());
}

Class const& FunctionArgs::peek() {
	return _function->getParamType(_args.size());
}

} // namespace expr
} // namespace slib
