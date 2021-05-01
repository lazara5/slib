/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Function.h"
#include "slib/util/expr/ExpressionFormatter.h"
#include "slib/collections/LinkedHashMap.h"

#include <cmath>

namespace slib {
namespace expr {

class Builtins : public HashMap<String, Object> {
public:
	Builtins() {
		// constants
		emplace<String, Boolean>("true", true);
		emplace<String, Boolean>("false", false);
		put("nil"_SPTR, nullptr);

		SPtr<Map<String, Object>> math = newS<HashMap<String, Object>>();
		emplaceKey<String>("math", math);

		math->emplaceKey<String>("ceil", Function::impl<Number>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, EvalFlags evalFlags SLIB_UNUSED, ArgList const& args) {
				return Value::of(newS<Double>(ceil(args.get<Number>(0)->doubleValue())));
			}
		));
		math->emplaceKey<String>("floor", Function::impl<Number>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED,EvalFlags evalFlags SLIB_UNUSED, ArgList const& args) {
				return Value::of(newS<Double>(floor(args.get<Number>(0)->doubleValue())));
			}
		));
		math->emplaceKey<String>("abs", Function::impl<Number>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED,EvalFlags evalFlags SLIB_UNUSED, ArgList const& args) {
				return Value::of(newS<Double>(abs(args.get<Number>(0)->doubleValue())));
			}
		));

		emplaceKey<String>("format", Function::impl<String>(
			[](SPtr<Resolver> const& resolver, EvalFlags evalFlags SLIB_UNUSED, ArgList const& args) {
				StringBuilder result;
				ExpressionFormatter::format(result, args, resolver);
				return Value::of(result.toString());
			}
		));

		emplaceKey<String>("if", Function::impl<Object, Expression, Expression>(
			[](SPtr<Resolver> const& resolver, EvalFlags evalFlags, ArgList const& args) {
				bool val = Value::isTrue(args.getNullable(0));
				if (val)
					return (args.get<Expression>(1))->evaluate(resolver, evalFlags);
				else {
					if (args.size() > 2)
						return (args.get<Expression>(2))->evaluate(resolver, evalFlags);
					else
						return Value::of(""_SPTR);
				}
			}
		));

		emplaceKey<String>("for", Function::impl<String, Object, Expression, Expression, Expression>(
			[](SPtr<Resolver> const& resolver, EvalFlags evalFlags, ArgList const& args) {
				size_t nArgs = args.size();
				if (nArgs == 5) {
					// classic "for"
					SPtr<String> loopVarName = args.get<String>(0);
					SPtr<Object> initialValue = args.getNullable(1);
					SPtr<Expression> condExpression = args.get<Expression>(2);
					SPtr<Expression> updateExpression = args.get<Expression>(3);
					SPtr<Expression> evalExpression = args.get<Expression>(4);

					SPtr<ExpressionEvaluator::LoopResolver> loopResolver = newS<ExpressionEvaluator::LoopResolver>(loopVarName, resolver);
					loopResolver->setVar(initialValue);

					UPtr<Value> finalValue = Value::Nil();
					bool cond = true;

					while (cond) {
						UPtr<Value> condValue = condExpression->evaluate(loopResolver, evalFlags);
						if (!Value::isTrue(condValue))
							break;
						UPtr<Value> exprValue = evalExpression->evaluate(loopResolver, evalFlags);
						if (finalValue->isNil())
							finalValue = std::move(exprValue);
						else
							finalValue = finalValue->add(exprValue);
						SPtr<Value> updatedValue = updateExpression->evaluate(loopResolver, evalFlags);
						loopResolver->setVar(updatedValue->getValue());
					}

					return finalValue;
				} else if (nArgs == 3) {
					// generic "for"
					SPtr<String> loopVarName = args.get<String>(0);
					SPtr<Object> iterable = args.get(1);
					if (instanceof<ConstIterable<Object>>(iterable)) {
						SPtr<Expression> evalExpression = args.get<Expression>(2);

						SPtr<ExpressionEvaluator::LoopResolver> loopResolver = newS<ExpressionEvaluator::LoopResolver>(loopVarName, resolver);

						UPtr<Value> finalValue = Value::Nil();

						ConstIterable<Object> *i = Class::castPtr<ConstIterable<Object>>(iterable);
						UPtr<ConstIterator<SPtr<Object>>> iter = i->constIterator();
						while (iter->hasNext()) {
							SPtr<Object> val = iter->next();
							loopResolver->setVar(val);

							UPtr<Value> exprValue = evalExpression->evaluate(loopResolver, evalFlags);
							if (finalValue->isNil())
								finalValue = std::move(exprValue);
							else
								finalValue = finalValue->add(exprValue);
						}

						return finalValue;
					} else
						throw EvaluationException(_HERE_, "generic for(): second argument is not iterable");
				} else
					throw EvaluationException(_HERE_, "for(): invalid number of arguments");
			}
		));

		emplaceKey<String>("assert", Function::impl<Object>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, EvalFlags evalFlags SLIB_UNUSED, ArgList const& args) {
				int nArgs = args.size();
				for (int i = 0; i < nArgs - 1; i += 2) {
					bool val = Value::isTrue(args.getNullable(i));
					if (!val)
						throw AssertException(_HERE_, args.get<String>(i + 1)->c_str());
				}
				if (nArgs %2 != 0)
					return Value::of(args.get(nArgs - 1));
				else
					return Value::of(""_SPTR);
			}
		));

		emplaceKey<String>("$", Function::impl<String>(
			[](SPtr<Resolver> const& resolver, EvalFlags evalFlags SLIB_UNUSED, ArgList const& args) {
				SPtr<String> varName = args.get<String>(0);
				SPtr<Object> value = resolver->getVar(*varName);
				return value ? Value::of(value, varName) : Value::Nil(varName);
			}
		));

		emplaceKey<String>("#", Function::impl<String>(
			[](SPtr<Resolver> const& resolver, EvalFlags evalFlags, ArgList const& args) {
				return ExpressionEvaluator::expressionValue(newS<ExpressionInputStream>(args.get<String>(0)), resolver, evalFlags);
			}
		));

		class ObjParseContext : public FunctionParseContext {
		private:
			SPtr<Map<String, Object>> _obj;
			int _numVal;
		public:
			ObjParseContext(SPtr<Function> const& function, SPtr<String> const& symbolName, SPtr<Resolver> const& resolver)
			: FunctionParseContext(function, symbolName, resolver)
			, _obj(newS<LinkedHashMap<String, Object>>()) {}

			virtual void addArg(SPtr<Object> const& obj) override {
				SPtr<KeyValue<String>> tuple = Class::cast<KeyValue<String>>(obj);
				if (tuple->_global)
					_resolver->setVar(*tuple->_key, tuple->_value);
				else {
					_obj->put(tuple->_key, tuple->_value);
					_numVal++;
				}
			}

			Class const& peekArg() {
				return _function->getParamType(_numVal);
			}

			virtual UPtr<Value> evaluate(SPtr<Resolver> const& resolver SLIB_UNUSED, EvalFlags evalFlags SLIB_UNUSED) override {
				return Value::of(_obj);
			}
		};

		emplaceKey<String>("::makeObj", Function::impl<KeyValue<String>>(
			/* LCOV_EXCL_START */
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, EvalFlags evalFlags SLIB_UNUSED, ArgList const& args SLIB_UNUSED) {
				// unused, the parse context evaluates directly
				return nullptr;
			},
			/* LCOV_EXCL_STOP */
			[](SPtr<Function> const& function, SPtr<String> const& symbolName, SPtr<Resolver> const& resolver) {
				return newU<ObjParseContext>(function, symbolName, resolver);
			}
		));

		emplaceKey<String>("::makeArray", Function::impl<Object>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, EvalFlags evalFlags SLIB_UNUSED, ArgList const& args) {
				SPtr<ArrayList<Object>> array = newS<ArrayList<Object>>();
				for (size_t i = 0; i < args.size(); i++) {
					SPtr<Object> e = args.getNullable(i);
					array->add(e);
				}
				return Value::of(array);
			}
		));
	}

	virtual ~Builtins();
};

Builtins::~Builtins() {}

UPtr<Map<String, Object>> ExpressionEvaluator::_builtins = newU<Builtins>();

} // namespace expr
} // namespace slib
