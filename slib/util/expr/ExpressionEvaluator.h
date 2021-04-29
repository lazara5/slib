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

typedef enum {
	NONE,
	VALUE,
	LITERAL,
	OBJ_CONSTRUCTOR,
	ARRAY_CONSTRUCTOR
} PrimaryType;

class Namespace : public Object{
public:
	SPtr<Object> _obj;
	SPtr<String> _name;

	Namespace(SPtr<Object> const& obj)
	: _obj(obj) {}
};

class ExpressionContext {
private:
	LinkedList<Namespace> _namespaceStack;
public:
	ExpressionContext(SPtr<Object> rootNamespace) {
		_namespaceStack.push(newS<Namespace>(rootNamespace));
	}

	void pushNamespace(SPtr<Object> const& obj);

	void popNamespace();

	void setName(SPtr<String> const& name);

	void clearName();

	void setNamedObject(SPtr<Object> const& obj);
};

class ExpressionEvaluator {
friend class Expression;
friend class ResultHolder;
private:
	static UPtr<Map<String, Object>> _builtins;

	class InternalResolver : public Resolver {
	private:
		SPtr<Resolver> const& _externalResolver;
	public:
		InternalResolver(SPtr<Resolver> const& externalResolver)
		:_externalResolver(externalResolver) {}

		virtual SPtr<Object> getVar(const String &key) const override;

		virtual bool isReadOnly() const override {
			return _externalResolver->isReadOnly();
		}

		virtual void setVar(String const& key, SPtr<Object> const& value) override {
			_externalResolver->setVar(key, value);
		}
	};
public:
	/** Resolver that provides the for loop variable */
	class LoopResolver : public Resolver {
	private:
		SPtr<String> _varName;
		SPtr<Object> _value;
		SPtr<Resolver> _parentResolver;
	public:
		virtual ~LoopResolver() override;

		LoopResolver(SPtr<String> const& varName,
					 SPtr<Resolver> const& parentResolver)
		:_varName(varName)
		,_parentResolver(parentResolver) {}

		void setVar(SPtr<Object> const& value) {
			_value = value;
		}

		virtual SPtr<Object> getVar(const String &key) const override {
			if (_varName->equals(key))
				return _value;

			return _parentResolver->getVar(key);
		}

		virtual bool isReadOnly() const override {
			return _parentResolver->isReadOnly();
		}

		virtual void setVar(String const& key, SPtr<Object> const& value) override {
			_parentResolver->setVar(key, value);
		}
	};
public:
	/** @throws EvaluationException */
	static UPtr<String> strExpressionValue(SPtr<BasicString> const& input, SPtr<Resolver> const& resolver, EvalFlags evalFlags);

	/** @throws EvaluationException */
	static SPtr<Object> expressionValue(SPtr<BasicString> const& input, SPtr<Resolver> const& resolver, EvalFlags evalFlags);

	/** @throws EvaluationException */
	static UPtr<Value> expressionValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver, EvalFlags evalFlags,
									   UPtr<ExpressionContext> const& ctx = nullptr);

	/** @throws EvaluationException */
	static UPtr<String> interpolate(String const& pattern, SPtr<Resolver> const& resolver, bool ignoreMissing);

	/** @throws EvaluationException */
	static SPtr<Object> smartInterpolate(String const& pattern, SPtr<Resolver> const& resolver, bool ignoreMissing);
protected:
	/** @throws EvaluationException */
	static UPtr<String> strExpressionValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver, EvalFlags evalFlags);

	/** @throws EvaluationException */
	static UPtr<Value> prefixTermValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
									   EvalFlags evalFlags, UPtr<ExpressionContext> const& ctx);

	/** @throws EvaluationException */
	static UPtr<Value> termValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
								 EvalFlags evalFlags, UPtr<ExpressionContext> const& ctx);

	/** @throws EvaluationException */
	static UPtr<Value> factorValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver,
								   EvalFlags evalFlags, UPtr<ExpressionContext> const& ctx);

	/** @throws EvaluationException */
	static UPtr<Value> primaryValue(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver, PrimaryType &type,
									EvalFlags evalFlags, UPtr<ExpressionContext> const& ctx);

	/** @throws EvaluationException */
	static UPtr<Value> evaluateSymbol(SPtr<ExpressionInputStream> const& input, SPtr<Resolver> const& resolver, PrimaryType &type, bool global);
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONEVALUATOR_H
