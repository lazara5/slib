/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSION_H
#define H_SLIB_UTIL_EXPR_EXPRESSION_H

#include "slib/lang/Object.h"
#include "slib/lang/String.h"

namespace slib {
namespace expr {

class Value;
class Resolver;

class Expression : virtual public Object {
private:
	SPtr<String> _text;
public:
	Expression(SPtr<String> const& text)
	:_text(text) {}

	virtual ~Expression() override;

	static constexpr Class _class = EXPRESSIONCLASS;

	virtual Class const& getClass() const override {
		return EXPRESSIONCLASS;
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> evaluate(Resolver const& resolver);
};

} // namespace expr
} // namespace slib

#endif //H_SLIB_UTIL_EXPR_EXPRESSION_H
