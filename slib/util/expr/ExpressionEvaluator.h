/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H

#include "slib/util/expr/ExpressionInputStream.h"
#include "slib/util/expr/Resolver.h"
#include "slib/lang/String.h"
#include "slib/collections/HashMap.h"
#include "slib/collections/LinkedList.h"

namespace slib {
namespace expr {

class Function;

class Builtins : public HashMap<String, Object> {
public:
	const SPtr<Function> _objectConstructor;
	const SPtr<Function> _arrayConstructor;
public:
	Builtins();
	virtual ~Builtins();
};

class ExpressionEvaluator {
friend class Expression;
friend class ResultHolder;
public:
	static const UPtr<Builtins> _builtins;
private:
	class InternalResolver : public Resolver {
	private:
		SPtr<Resolver> const& _externalResolver;
	public:
		InternalResolver(SPtr<Resolver> const& externalResolver)
		:_externalResolver(externalResolver) {}

		virtual SPtr<Object> getVar(const String &key, ValueDomain domain) const override;

		virtual bool isReadOnly(ValueDomain domain) const override {
			return _externalResolver->isReadOnly(domain);
		}

		virtual void setVar(SPtr<String> const& key, SPtr<Object> const& value, ValueDomain domain) override {
			_externalResolver->setVar(key, value, domain);
		}
	};

public:
	/** @throws EvaluationException */
	static UPtr<String> strExpressionValue(SPtr<BasicString> const& input, SPtr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static SPtr<Object> expressionValue(SPtr<BasicString> const& input, SPtr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static UPtr<Value> expressionValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static UPtr<Value> singleExpressionValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver);

	/** @throws EvaluationException */
	static UPtr<String> interpolate(String const& pattern, SPtr<Resolver> const& resolver, bool ignoreMissing);

	/** @throws EvaluationException */
	static SPtr<Object> smartInterpolate(String const& pattern, SPtr<Resolver> const& resolver, bool ignoreMissing);
protected:
	/** @throws EvaluationException */
	static UPtr<String> strExpressionValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver);
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
