/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionEvaluator.h"
#include "slib/util/expr/Function.h"

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

		std::shared_ptr<Map<String, Object>> math = std::make_shared<HashMap<String, Object>>();
		put("math", math);

		math->put("ceil", Function::impl<Double>(
			[](std::shared_ptr<Resolver> const& resolver, ArgList const& args) {
				return Value::of(std::make_shared<Double>(ceil(args.get<Double>(0)->doubleValue())));
			}
		));
		math->put("floor", Function::impl<Double>(
			[](std::shared_ptr<Resolver> const& resolver, ArgList const& args) {
				return Value::of(std::make_shared<Double>(floor(args.get<Double>(0)->doubleValue())));
			}
		));
		math->put("abs", Function::impl<Double>(
			[](std::shared_ptr<Resolver> const& resolver, ArgList const& args) {
				return Value::of(std::make_shared<Double>(abs(args.get<Double>(0)->doubleValue())));
			}
		));

		put("if", Function::impl<Object, Expression, Expression>(
			[](std::shared_ptr<Resolver> const& resolver, ArgList const& args) {
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
			[](std::shared_ptr<Resolver> const& resolver, ArgList const& args) {
				size_t nArgs = args.size();
				if (nArgs == 5) {
					// classic "for"
					std::shared_ptr<String> loopVarName = args.get<String>(0);
					std::shared_ptr<Object> initialValue = args.getNullable(1);
					std::shared_ptr<Expression> condExpression = args.get<Expression>(2);
					std::shared_ptr<Expression> updateExpression = args.get<Expression>(3);
					std::shared_ptr<Expression> evalExpression = args.get<Expression>(4);

					std::shared_ptr<ExpressionEvaluator::LoopResolver> loopResolver = std::make_shared<ExpressionEvaluator::LoopResolver>(loopVarName, resolver);
					loopResolver->setVar(initialValue);

					std::shared_ptr<Value> finalValue = Value::Nil();
					bool cond = true;

					while (cond) {
						std::shared_ptr<Value> condValue = condExpression->evaluate(loopResolver);
						if (!Value::isTrue(condValue))
							break;
						std::shared_ptr<Value> exprValue = evalExpression->evaluate(loopResolver);
						if (finalValue->isNil())
							finalValue = exprValue;
						else
							finalValue = finalValue->add(exprValue);
						std::shared_ptr<Value> updatedValue = updateExpression->evaluate(loopResolver);
						loopResolver->setVar(updatedValue->getValue());
					}

					return finalValue;
				} else if (nArgs == 3) {
					// generic "for"
					std::shared_ptr<String> loopVarName = args.get<String>(0);
					std::shared_ptr<Object> iterable = args.get(1);
					if (instanceof<ConstIterable<Object>>(iterable)) {
						std::shared_ptr<Expression> evalExpression = args.get<Expression>(2);

						std::shared_ptr<ExpressionEvaluator::LoopResolver> loopResolver = std::make_shared<ExpressionEvaluator::LoopResolver>(loopVarName, resolver);

						std::shared_ptr<Value> finalValue = Value::Nil();

						ConstIterable<Object> *i = Class::castPtr<ConstIterable<Object>>(iterable);
						ConstIterator<std::shared_ptr<Object>> iter = i->constIterator();
						while (iter.hasNext()) {
							std::shared_ptr<Object> val = iter.next();
							loopResolver->setVar(val);

							std::shared_ptr<Value> exprValue = evalExpression->evaluate(loopResolver);
							if (finalValue->isNil())
								finalValue = exprValue;
							else
								finalValue = finalValue->add(exprValue);
						}

						/*for (Object val : (Iterable<?>)iterable) {
							loopResolver.setVar(val);

							Value exprValue = evalExpression.evaluate(loopResolver);
							if (finalValue->isNil())
								finalValue = exprValue;
							else
								finalValue = finalValue->add(exprValue);
						}*/

						return finalValue;
					} else
						throw EvaluationException(_HERE_, "generic for(): second argument is not iterable");
				} else
					throw EvaluationException(_HERE_, "for(): invalid number of arguments");
			}
		));
	}

	virtual ~Builtins();
};

Builtins::~Builtins() {}

std::unique_ptr<Map<String, Object>> ExpressionEvaluator::_builtins = std::make_unique<Builtins>();

} // namespace expr
} // namespace slib
