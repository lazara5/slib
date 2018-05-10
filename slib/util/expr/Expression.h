/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSION_H
#define H_SLIB_UTIL_EXPR_EXPRESSION_H

#include "slib/Object.h"
#include "slib/String.h"

namespace slib {
namespace expr {

class Value;
class Resolver;

class Expression : virtual public Object {
private:
	std::shared_ptr<String> _text;
public:
	Expression(std::shared_ptr<String> const& text)
	:_text(text) {}

	virtual ~Expression() override;

	static Class const* CLASS() {
		return EXPRESSIONCLASS();
	}

	virtual Class const* getClass() const override {
		return EXPRESSIONCLASS();
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> evaluate(std::shared_ptr<Resolver> resolver);
};

} // namespace expr
} // namespace slib

#endif //H_SLIB_UTIL_EXPR_EXPRESSION_H
