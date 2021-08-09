/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_LAMBDA_H
#define H_SLIB_UTIL_EXPR_LAMBDA_H

#include "slib/lang/Object.h"
#include "slib/lang/String.h"

namespace slib {

namespace expr {

class Value;
class Resolver;

class Lambda : virtual public Object {
public:
	TYPE_INFO(Lambda, CLASS(Lambda), INHERITS(Object));
private:
	SPtr<String> _text;
public:
	Lambda(SPtr<String> const& text)
	:_text(text) {}

	virtual ~Lambda() override;

	/** @throws EvaluationException */
	UPtr<Value> evaluate(SPtr<Resolver> const& resolver);

	/** @throws EvaluationException */
	UPtr<Value> readLiteral();
};

} // namespace expr
} // namespace slib

#endif //H_SLIB_UTIL_EXPR_LAMBDA_H
