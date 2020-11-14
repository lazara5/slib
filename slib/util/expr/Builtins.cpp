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
		put(std::make_shared<String>("nil"), nullptr);

		SPtr<Map<String, Object>> math = std::make_shared<HashMap<String, Object>>();
		emplace_key<String>("math", math);

		math->put("ceil", Function::impl<Double>(
			[](Resolver const& resolver SLIB_UNUSED, ArgList const& args) {
				return Value::of(std::make_shared<Double>(ceil(args.get<Double>(0)->doubleValue())));
			}
		));
		math->put("floor", Function::impl<Double>(
			[](Resolver const& resolver SLIB_UNUSED, ArgList const& args) {
				return Value::of(std::make_shared<Double>(floor(args.get<Double>(0)->doubleValue())));
			}
		));
		math->put("abs", Function::impl<Number>(
			[](Resolver const& resolver SLIB_UNUSED, ArgList const& args) {
				return Value::of(std::make_shared<Double>(abs(args.get<Number>(0)->doubleValue())));
			}
		));

		emplace_key<String>("format", Function::impl<String>(
			[](Resolver const& resolver, ArgList const& args) {
				StringBuilder result;
				ExpressionFormatter::format(result, args, resolver);
				return Value::of(result.toString());
			}
		));

		emplace_key<String>("if", Function::impl<Object, Expression, Expression>(
			[](Resolver const& resolver, ArgList const& args) {
				bool val = Value::isTrue(args.getNullable(0));
				if (val)
					return (args.get<Expression>(1))->evaluate(resolver);
				else {
					if (args.size() > 2)
						return (args.get<Expression>(2))->evaluate(resolver);
					else
						return Value::of(std::make_shared<String>(""));
				}
			}
		));

		emplace_key<String>("for", Function::impl<String, Object, Expression, Expression, Expression>(
			[](Resolver const& resolver, ArgList const& args) {
				size_t nArgs = args.size();
				if (nArgs == 5) {
					// classic "for"
					SPtr<String> loopVarName = args.get<String>(0);
					SPtr<Object> initialValue = args.getNullable(1);
					SPtr<Expression> condExpression = args.get<Expression>(2);
					SPtr<Expression> updateExpression = args.get<Expression>(3);
					SPtr<Expression> evalExpression = args.get<Expression>(4);

					ExpressionEvaluator::LoopResolver loopResolver(loopVarName, resolver);
					loopResolver.setVar(initialValue);

					SPtr<Value> finalValue = Value::Nil();
					bool cond = true;

					while (cond) {
						SPtr<Value> condValue = condExpression->evaluate(loopResolver);
						if (!Value::isTrue(condValue))
							break;
						SPtr<Value> exprValue = evalExpression->evaluate(loopResolver);
						if (finalValue->isNil())
							finalValue = exprValue;
						else
							finalValue = finalValue->add(exprValue);
						SPtr<Value> updatedValue = updateExpression->evaluate(loopResolver);
						loopResolver.setVar(updatedValue->getValue());
					}

					return finalValue;
				} else if (nArgs == 3) {
					// generic "for"
					SPtr<String> loopVarName = args.get<String>(0);
					SPtr<Object> iterable = args.get(1);
					if (instanceof<ConstIterable<Object>>(iterable)) {
						SPtr<Expression> evalExpression = args.get<Expression>(2);

						ExpressionEvaluator::LoopResolver loopResolver(loopVarName, resolver);

						SPtr<Value> finalValue = Value::Nil();

						ConstIterable<Object> *i = Class::castPtr<ConstIterable<Object>>(iterable);
						ConstIterator<SPtr<Object>> iter = i->constIterator();
						while (iter.hasNext()) {
							SPtr<Object> val = iter.next();
							loopResolver.setVar(val);

							SPtr<Value> exprValue = evalExpression->evaluate(loopResolver);
							if (finalValue->isNil())
								finalValue = exprValue;
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

		emplace_key<String>("assert", Function::impl<Object>(
			[](Resolver const& resolver SLIB_UNUSED, ArgList const& args) {
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

		emplace_key<String>("$", Function::impl<String>(
			[](Resolver const& resolver, ArgList const& args) {
				SPtr<String> varName = args.get<String>(0);
				SPtr<Object> value = resolver.getVar(*varName);
				return value ? Value::of(value, varName) : Value::Nil(varName);
			}
		));

		emplace_key<String>("#", Function::impl<String>(
			[](Resolver const& resolver, ArgList const& args) {
				return ExpressionEvaluator::expressionValue(std::make_shared<ExpressionInputStream>(args.get<String>(0)), resolver);
			}
		));

		emplace_key<String>("::makeObj", Function::impl<KeyValueTuple<String>>(
			[](Resolver const& resolver SLIB_UNUSED, ArgList const& args) {
				SPtr<Map<String, Object>> map = newS<LinkedHashMap<String, Object>>();
				for (size_t i = 0; i < args.size(); i++) {
					SPtr<KeyValueTuple<String>> tuple = args.get<KeyValueTuple<String>>(i);
					map->put(tuple->key, tuple->value);
				}
				return Value::of(map);
			}
		));

		emplace_key<String>("::makeArray", Function::impl<Object>(
			[](Resolver const& resolver SLIB_UNUSED, ArgList const& args) {
				SPtr<ArrayList<Object>> array = newS<ArrayList<Object>>();
				for (size_t i = 0; i < args.size(); i++) {
					SPtr<Object> e = args.get(i);
					array->add(e);
				}
				return Value::of(array);
			}
		));
	}

	virtual ~Builtins();
};

Builtins::~Builtins() {}

UPtr<Map<String, Object>> ExpressionEvaluator::_builtins = std::make_unique<Builtins>();

} // namespace expr
} // namespace slib
