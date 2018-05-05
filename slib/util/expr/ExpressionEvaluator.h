/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H

#include "slib/util/expr/ExpressionInputStream.h"
#include "slib/util/expr/Resolver.h"
#include "slib/String.h"

namespace slib {
namespace expr {

class ExpressionEvaluator {
public:
	/** @throws EvaluationException */
	static std::unique_ptr<String> strExpressionValue(std::shared_ptr<ExpressionInputStream> const& input,
													  std::shared_ptr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static std::shared_ptr<Value> expressionValue(std::shared_ptr<ExpressionInputStream> const& input,
												  std::shared_ptr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static std::shared_ptr<Value> prefixTermValue(std::shared_ptr<ExpressionInputStream> const& input,
												  std::shared_ptr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static std::shared_ptr<Value> termValue(std::shared_ptr<ExpressionInputStream> const& input,
											std::shared_ptr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static std::shared_ptr<Value> factorValue(std::shared_ptr<ExpressionInputStream> const& input,
											  std::shared_ptr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static std::shared_ptr<Value> primaryValue(std::shared_ptr<ExpressionInputStream> const& input,
											   std::shared_ptr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static std::shared_ptr<Value> evaluateSymbol(std::shared_ptr<ExpressionInputStream> const& input,
												 std::shared_ptr<Resolver> const& resolver);
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
