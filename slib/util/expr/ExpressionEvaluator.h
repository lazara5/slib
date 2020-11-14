/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H

#include "slib/util/expr/ExpressionInputStream.h"
#include "slib/util/expr/Resolver.h"
#include "slib/lang/String.h"
#include "slib/collections/HashMap.h"

namespace slib {
namespace expr {

typedef enum {
	NONE,
	VALUE,
	LITERAL,
	OBJ_CONSTRUCTOR,
	ARRAY_CONSTRUCTOR
} PrimaryType;

class ExpressionEvaluator {
friend class Expression;
friend class ResultHolder;
private:
	static UPtr<Map<String, Object>> _builtins;

	class InternalResolver : public Resolver {
	private:
		Resolver const& _externalResolver;
	public:
		InternalResolver(Resolver const& externalResolver)
		:_externalResolver(externalResolver) {}

		virtual SPtr<Object> getVar(const String &key) const override;
	};
public:
	/** Resolver that provides the for loop variable */
	class LoopResolver : public Resolver {
	private:
		SPtr<String> _varName;
		SPtr<Object> _value;
		Resolver const& _parentResolver;
	public:
		virtual ~LoopResolver() override;

		LoopResolver(SPtr<String> const& varName,
					 Resolver const& parentResolver)
		:_varName(varName)
		,_parentResolver(parentResolver) {}

		void setVar(SPtr<Object> const& value) {
			_value = value;
		}

		virtual SPtr<Object> getVar(const String &key) const override {
			if (_varName->equals(key))
				return _value;

			return _parentResolver.getVar(key);
		}
	};
public:
	/** @throws EvaluationException */
	static UPtr<String> strExpressionValue(SPtr<BasicString> const& input, Resolver const& resolver);

	/** @throws EvaluationException */
	static SPtr<Object> expressionValue(SPtr<BasicString> const& input, Resolver const& resolver);

	/** @throws EvaluationException */
	static SPtr<Value> expressionValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver);

	/** @throws EvaluationException */
	static UPtr<String> interpolate(String const& pattern, Resolver const& resolver, bool ignoreMissing);

	/** @throws EvaluationException */
	static SPtr<Object> smartInterpolate(String const& pattern, Resolver const& resolver, bool ignoreMissing);
protected:
	/** @throws EvaluationException */
	static UPtr<String> strExpressionValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver);

	/** @throws EvaluationException */
	static SPtr<Value> prefixTermValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver);

	/** @throws EvaluationException */
	static SPtr<Value> termValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver);

	/** @throws EvaluationException */
	static SPtr<Value> factorValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver);

	/** @throws EvaluationException */
	static SPtr<Value> primaryValue(SPtr<ExpressionInputStream> const& input, Resolver const& resolver, PrimaryType &type);

	/** @throws EvaluationException */
	static SPtr<Value> evaluateSymbol(SPtr<ExpressionInputStream> const& input, Resolver const& resolver, PrimaryType &type);
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
