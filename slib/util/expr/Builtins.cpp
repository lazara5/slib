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

class ObjResolver : public Resolver {
private:
	SPtr<Map<BasicString, Object>> _obj;
	SPtr<Resolver> _parentResolver;
public:
	ObjResolver(SPtr<Resolver> const& parentResolver)
	: _obj(newS<LinkedHashMap<BasicString, Object>>())
	, _parentResolver(parentResolver) {}

	virtual ~ObjResolver() {};

	virtual SPtr<Object> getVar(const String &key, ValueDomain domain) const override {
		switch (domain) {
			case ValueDomain::LOCAL:
				return _parentResolver->getVar(key, domain);
			case ValueDomain::DEFAULT: {
				SPtr<Object> res =_obj->get(key);
				if (res)
					return res;
			}
			/* fall through */
			case ValueDomain::GLOBAL:
				return _parentResolver->getVar(key, domain);
			default:
				throw EvaluationException(_HERE_, "Invalid value domain");
		}
	}

	virtual bool isWritable(ValueDomain domain) const override {
		return (domain == ValueDomain::DEFAULT) ? true : _parentResolver->isWritable(domain);
	}

	virtual void setVar(SPtr<String> const& key, SPtr<Object> const& value, ValueDomain domain) override {
		switch (domain) {
			case ValueDomain::LOCAL:
			case ValueDomain::GLOBAL:
				_parentResolver->setVar(key, value, domain);
				break;
			case ValueDomain::DEFAULT:
				_obj->put(key, value);
				break;
			default:
				throw EvaluationException(_HERE_, "Invalid value domain");
		}
	}

	SPtr<Map<BasicString, Object>> getObj() {
		return _obj;
	}
};

class ObjFunctionInstance : public FunctionInstance {
private:
	SPtr<ObjResolver> _objResolver;
public:
	ObjFunctionInstance(SPtr<Function> const& function, SPtr<String> const& symbolName, SPtr<Resolver> const& resolver)
	: FunctionInstance(function, symbolName, resolver, nullptr)
	, _objResolver(newS<ObjResolver>(resolver)) {
		_argResolver = _objResolver;
	}

	virtual UPtr<Value> evaluate() override {
		return Value::of(_objResolver->getObj());
	}
};

Builtins::Builtins()
: _objectConstructor(Function::impl<Object>(
		/* LCOV_EXCL_START */
		[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args SLIB_UNUSED) {
			// unused, the instance evaluates directly
			return nullptr;
		},
		[](SPtr<Function> const& function, SPtr<String> const& symbolName, SPtr<Resolver> const& resolver) {
			return newS<ObjFunctionInstance>(function, symbolName, resolver);
		},
		',', '}'
		/* LCOV_EXCL_STOP */
	))
, _arrayConstructor(Function::impl<Object>(
		[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
			SPtr<ArrayList<Object>> array = newS<ArrayList<Object>>();
			for (size_t i = 0; i < args.size(); i++) {
				SPtr<Object> e = args.getNullable(i);
				array->add(e);
			}
			return Value::of(array);
		},
		defaultNewFunctionInstance,
		',', ']'
	)) {
		// Math library
		SPtr<Map<BasicString, Object>> math = newS<HashMap<BasicString, Object>>();
		put("math"_SPTR, math);

		math->put("ceil"_SPTR, Function::impl<Number>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
				return Value::of(newS<Double>(ceil(args.get<Number>(0)->doubleValue())));
			}
		));
		math->put("floor"_SPTR, Function::impl<Number>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
				return Value::of(newS<Double>(floor(args.get<Number>(0)->doubleValue())));
			}
		));
		math->put("abs"_SPTR, Function::impl<Number>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
				return Value::of(newS<Double>(abs(args.get<Number>(0)->doubleValue())));
			}
		));

		put("format"_SPTR, Function::impl<String>(
			[](SPtr<Resolver> const& resolver, ArgList const& args) {
				StringBuilder result;
				ExpressionFormatter::format(result, args, resolver);
				return Value::of(result.toString());
			}
		));

		put("double"_SPTR, Function::impl<Object>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
				try {
					SPtr<Object> obj = args.get(0);
					if (instanceof<String>(obj))
						return Value::of(Double::parseDouble(Class::cast<String>(obj)));
					else if (instanceof<Number>(obj))
						return Value::of((Class::cast<Number>(obj))->doubleValue());
					else
						throw EvaluationException(_HERE_, fmt::format("double(): unsupported conversion from {}", obj->getClass().getName()).c_str());
				} catch (NumberFormatException const& e) {
					throw EvaluationException(_HERE_, "double()", e);
				}
			}
		));

		put("long"_SPTR, Function::impl<Object>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
				try {
					SPtr<Object> obj = args.get(0);
						if (instanceof<String>(obj))
							return Value::of(Long::parseLong(Class::cast<String>(obj)));
						else if (instanceof<Number>(obj))
							return Value::of((Class::cast<Number>(obj))->longValue());
						else
							throw  EvaluationException(_HERE_, fmt::format("long(): unsupported conversion from {}", obj->getClass().getName()).c_str());
				} catch (NumberFormatException e) {
					throw EvaluationException(_HERE_, "long()", e);
				}
			}
		));

		put("string"_SPTR, Function::impl<Object>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
				SPtr<Object> obj = args.get(0);
				return Value::of(obj->toString());
			}
		));

		put("if"_SPTR, Function::impl<Object, Lambda, Lambda>(
			[](SPtr<Resolver> const& resolver, ArgList const& args) {
				bool val = Value::isTrue(args.getNullable(0));
				if (val)
					return (args.get<Lambda>(1))->evaluate(resolver);
				else {
					if (args.size() > 2)
						return (args.get<Lambda>(2))->evaluate(resolver);
					else
						return Value::of(""_SPTR);
				}
			}
		));

		/** Resolver that provides the for loop variable */
		class LoopResolver : public Resolver {
		private:
			SPtr<Object> _result;
			UPtr<Map<String, Object>> _locals;
			SPtr<Resolver> _parentResolver;
		public:
			LoopResolver(SPtr<Resolver> const& parentResolver)
			: _result(Value::voidObj())
			, _locals(newU<HashMap<String, Object>>())
			, _parentResolver(parentResolver) {}

			virtual ~LoopResolver() {};

			SPtr<Object> getResult() {
				return _result;
			}

			virtual SPtr<Object> getVar(const String &key, ValueDomain domain) const override {
				if (key.equals("$"))
					return _result;

				switch (domain) {
					case ValueDomain::LOCAL:
						return _locals->get(key);
					case ValueDomain::DEFAULT:
					case ValueDomain::GLOBAL:
						return _parentResolver->getVar(key, domain);
					default:
						throw EvaluationException(_HERE_, "Invalid value domain");
				}
			}

			virtual bool isWritable(ValueDomain domain) const override {
				return (domain == ValueDomain::LOCAL) ? true : _parentResolver->isWritable(domain);
			}

			virtual void setVar(SPtr<String> const& key, SPtr<Object> const& value, ValueDomain domain) override {
				if (StringView::equals(key, "$"))
					_result = value;

				switch (domain) {
					case ValueDomain::LOCAL:
						_locals->put(key, value);
						break;
					case ValueDomain::DEFAULT:
					case ValueDomain::GLOBAL:
						_parentResolver->setVar(key, value, domain);
						break;
					default:
						throw EvaluationException(_HERE_, "Invalid value domain");
				}
			}
		};

		put("for"_SPTR, Function::impl<Lambda, Lambda, Lambda, Lambda>(
			[](SPtr<Resolver> const& resolver, ArgList const& args) {
				size_t nArgs = args.size();
				if (nArgs == 4) {
					// classic "for"
					SPtr<Lambda> loopInit = args.get<Lambda>(0);
					SPtr<Lambda> loopCond = args.get<Lambda>(1);
					SPtr<Lambda> loopUpdate = args.get<Lambda>(2);
					SPtr<Lambda> loopEval = args.get<Lambda>(3);

					SPtr<LoopResolver> loopResolver = newS<LoopResolver>(resolver);
					loopInit->evaluate(loopResolver);

					UPtr<Value> finalValue = Value::Nil();

					while (true) {
						UPtr<Value> condValue = loopCond->evaluate(loopResolver);
						if (!Value::isTrue(condValue))
							break;
						loopEval->evaluate(loopResolver);
						SPtr<Value> updatedValue = loopUpdate->evaluate(loopResolver);
					}

					return Value::of(loopResolver->getResult());
				} else if (nArgs == 3) {
					// generic "for"
					SPtr<Lambda> loopVarLambda = args.get<Lambda>(0);
					UPtr<Value> loopVar = loopVarLambda->readLiteral();
					SPtr<String> loopVarName = Class::cast<String>(loopVar->getValue());
					UPtr<Value> iterableValue = args.get<Lambda>(1)->evaluate(resolver);
					SPtr<Object> iterable = iterableValue ? iterableValue->getValue() : nullptr;

					if (instanceof<ConstIterable<Object>>(iterable)) {
						SPtr<Lambda> loopEval = args.get<Lambda>(2);

						SPtr<LoopResolver> loopResolver = newS<LoopResolver>(resolver);

						UPtr<Value> finalValue = Value::Nil();

						ConstIterable<Object> *i = Class::castPtr<ConstIterable<Object>>(iterable);
						UPtr<ConstIterator<SPtr<Object>>> iter = i->constIterator();
						while (iter->hasNext()) {
							SPtr<Object> val = iter->next();
							loopResolver->setVar(loopVarName, val, loopVar->getDomain());
							loopEval->evaluate(loopResolver);
						}

						return Value::of(loopResolver->getResult());
					} else
						throw EvaluationException(_HERE_, "generic for(): second argument is not iterable");
				} else
					throw EvaluationException(_HERE_, "for(): invalid number of arguments");
			},
			defaultNewFunctionInstance,
			';', ')'
		));

		put("assert"_SPTR, Function::impl<Object>(
			[](SPtr<Resolver> const& resolver SLIB_UNUSED, ArgList const& args) {
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

		put("@"_SPTR, Function::impl<String>(
			[](SPtr<Resolver> const& resolver, ArgList const& args) {
				SPtr<String> pattern = args.get<String>(0);
				try {
					SPtr<String> value = ExpressionEvaluator::interpolate(*pattern, resolver, false);
					return Value::of(value);
				} catch (NullPointerException const&) {
					return Value::Nil();
				}
			}
		));

		put("$"_SPTR, Function::impl<String>(
			[](SPtr<Resolver> const& resolver, ArgList const& args) {
				SPtr<String> varName = args.get<String>(0);
				SPtr<Object> value = resolver->getVar(*varName, ValueDomain::DEFAULT);
				return value ? Value::of(value, varName) : Value::Nil(varName);
			}
		));

		put("#"_SPTR, Function::impl<String>(
			[](SPtr<Resolver> const& resolver, ArgList const& args) {
				return ExpressionEvaluator::expressionValue(newS<ExpressionInputStream>(args.get<String>(0)), resolver);
			}
		));
	}

Builtins::~Builtins() {}

const UPtr<Builtins> ExpressionEvaluator::_builtins = newU<Builtins>();

} // namespace expr
} // namespace slib
