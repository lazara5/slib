/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H

#include "slib/util/expr/ExpressionInputStream.h"
#include "slib/util/expr/Resolver.h"
#include "slib/String.h"
#include "slib/collections/HashMap.h"

namespace slib {
namespace expr {

class ExpressionEvaluator {
friend class Expression;
private:
	static std::unique_ptr<Map<String, Object>> _builtins;

	class InternalResolver : public Resolver {
	private:
		std::shared_ptr<Resolver> _externalResolver;
	public:
		InternalResolver(std::shared_ptr<Resolver> const& externalResolver)
		:_externalResolver(externalResolver) {}

		virtual std::shared_ptr<Object> getVar(const String &key) override;
	};
public:
	/** Resolver that provides the for loop variable */
	class LoopResolver : public Resolver {
	private:
		std::shared_ptr<String> _varName;
		std::shared_ptr<Object> _value;
		std::shared_ptr<Resolver> _parentResolver;
	public:
		LoopResolver(std::shared_ptr<String> const& varName,
					 std::shared_ptr<Resolver> const& parentResolver)
		:_varName(varName)
		,_parentResolver(parentResolver) {}

		void setVar(std::shared_ptr<Object> const& value) {
			_value = value;
		}

		virtual std::shared_ptr<Object> getVar(const String &key) override {
			if (_varName->equals(key))
				return _value;

			return _parentResolver->getVar(key);
		}
	};
public:
	/** @throws EvaluationException */
	static std::unique_ptr<String> strExpressionValue(std::shared_ptr<BasicString> const& input,
													  std::shared_ptr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static std::shared_ptr<Object> expressionValue(std::shared_ptr<BasicString> const& input,
												   std::shared_ptr<Resolver> const& resolver);
protected:
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
