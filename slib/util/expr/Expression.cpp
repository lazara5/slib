/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/Expression.h"
#include "slib/util/expr/ExpressionEvaluator.h"

namespace slib {
namespace expr {

Expression::~Expression() {}

UPtr<Value> Expression::evaluate(SPtr<Resolver> const& resolver, EvalFlags evalFlags) {
	return ExpressionEvaluator::expressionValue(newS<ExpressionInputStream>(_text), resolver, evalFlags);
}

} // namespace expr
} // namespace slib
