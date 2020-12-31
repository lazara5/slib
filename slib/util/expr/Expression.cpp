/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Expression.h"
#include "slib/util/expr/ExpressionEvaluator.h"

namespace slib {
namespace expr {

Expression::~Expression() {}

UPtr<Value> Expression::evaluate(Resolver const& resolver) {
	return ExpressionEvaluator::expressionValue(std::make_shared<ExpressionInputStream>(_text), resolver);
}

} // namespace expr
} // namespace slib
