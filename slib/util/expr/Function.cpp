/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Function.h"

namespace slib {
namespace expr {

Function::~Function() {}

ArgList::~ArgList() {}

FunctionArgs::~FunctionArgs() {}

void FunctionArgs::add(std::shared_ptr<Object> const& obj) {
	size_t np = _args.size();
	Class const* pClass = _function->getParamType(np);
	if ((!obj) || (pClass->isAssignableFrom(obj->getClass())))
		_args.add(obj);
	else
		throw CastException(_HERE_, fmt::format("Function {}(): invalid parameter type: expected {}, got {}", *_symbolName, pClass->getName(), obj->getClass()->getName()).c_str());
}

const Class *FunctionArgs::peek() {
	return _function->getParamType(_args.size());
}

} // namespace expr
} // namespace slib
