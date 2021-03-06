/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Function.h"
#include "slib/util/expr/ExpressionFormatter.h"

#include <cmath>

namespace slib {
namespace expr {

class Builtins : public HashMap<String, Object> {
public:
	Builtins() {
		// constants
		put("true", std::make_shared<Boolean>(true));
		put("false", std::make_shared<Boolean>(false));
		put("nil", nullptr);

		SPtr<Map<String, Object>> math = std::make_shared<HashMap<String, Object>>();
		put("math", math);

		math->put("ceil", Function::impl<Double>(
			[](Resolver const& /* resolver */, ArgList const& args) {
				return Value::of(std::make_shared<Double>(ceil(args.get<Double>(0)->doubleValue())));
			}
		));
		math->put("floor", Function::impl<Double>(
			[](Resolver const& /* resolver */, ArgList const& args) {
				return Value::of(std::make_shared<Double>(floor(args.get<Double>(0)->doubleValue())));
			}
		));
		math->put("abs", Function::impl<Double>(
			[](Resolver const& /* resolver */, ArgList const& args) {
				return Value::of(std::make_shared<Double>(abs(args.get<Double>(0)->doubleValue())));
			}
		));

		put("format", Function::impl<String>(
			[](Resolver const& resolver, ArgList const& args) {
				StringBuilder result;
				ExpressionFormatter::format(result, args, resolver);
				return Value::of(result.toString());
			}
		));

		put("if", Function::impl<Object, Expression, Expression>(
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

		put("for", Function::impl<String, Object, Expression, Expression, Expression>(
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

		put("$", Function::impl<String>(
			[](Resolver const& resolver, ArgList const& args) {
				SPtr<String> varName = args.get<String>(0);
				SPtr<Object> value = resolver.getVar(*varName);
				return value ? Value::of(value, varName) : Value::Nil(varName);
			}
		));

		put("#", Function::impl<String>(
			[](Resolver const& resolver, ArgList const& args) {
				return ExpressionEvaluator::expressionValue(std::make_shared<ExpressionInputStream>(args.get<String>(0)), resolver);
			}
		));
	}

	virtual ~Builtins();
};

Builtins::~Builtins() {}

UPtr<Map<String, Object>> ExpressionEvaluator::_builtins = std::make_unique<Builtins>();

} // namespace expr
} // namespace slib
