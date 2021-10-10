/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSION_H
#define H_SLIB_UTIL_EXPR_EXPRESSION_H

#include "slib/lang/Object.h"
#include "slib/lang/String.h"
#include "slib/util/expr/ExpressionInputStream.h"

namespace slib {

namespace expr {

class Value;
class Resolver;
class ExpressionInputStream;

class Expression : virtual public Object {
public:
	TYPE_INFO(Expression, CLASS(Expression), INHERITS(Object));
private:
	UPtr<ExpressionInputStream> _text;
public:
	Expression(SPtr<String> const& text)
	: _text(newU<ExpressionInputStream>(text)) {}

	virtual ~Expression() override;

	/** @throws EvaluationException */
	UPtr<Value> evaluate(SPtr<Resolver> const& resolver);

	/** @throws EvaluationException */
	UPtr<Value> readLiteral();
};

} // namespace expr
} // namespace slib

#endif //H_SLIB_UTIL_EXPR_EXPRESSION_H
