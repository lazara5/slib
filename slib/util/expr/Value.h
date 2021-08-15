/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_VALUE_H
#define H_SLIB_UTIL_EXPR_VALUE_H

#include "slib/lang/Object.h"
#include "slib/lang/String.h"
#include "slib/lang/StringBuilder.h"
#include "slib/util/expr/Exceptions.h"
#include "slib/util/expr/Resolver.h"
#include "slib/collections/Map.h"
#include "slib/collections/List.h"
#include "slib/lang/Numeric.h"

namespace slib {
namespace expr {

class Assignable {
public:
	virtual ~Assignable() {};

	virtual void assign(SPtr<Object> const& value) = 0;
};

class ResolverAssignable : public Assignable {
private:
	SPtr<Resolver> _resolver;
	SPtr<String> _name;
	ValueDomain _domain;
public:
	ResolverAssignable(SPtr<Resolver> resolver, SPtr<String> name, ValueDomain domain)
	: _resolver(resolver)
	, _name(name)
	, _domain(domain) {}

	virtual void assign(SPtr<Object> const& value) override {
		_resolver->setVar(_name, value, _domain);
	}
};

class Value {
friend class ExpressionEvaluator;
friend class ResultHolder;
private:
	const SPtr<Object> _value;
	const SPtr<String> _name;
	const SPtr<Assignable> _assignable;
	const ValueDomain _domain;
private:
	static Object _void;
public:
	Value(SPtr<Assignable> assignable, SPtr<Object> const& value,
		  SPtr<String> const& name = nullptr, ValueDomain domain = ValueDomain::DEFAULT)
	: _value(value)
	, _name(name)
	, _assignable(std::move(assignable))
	, _domain(domain) {}

	static SPtr<Object> voidObj() {
		return SPtr<Object>(SPtr<Object>{}, &_void);
	}

	static UPtr<Value> of(double value) {
		return newU<Value>(nullptr, newS<Double>(value));
	}

	static UPtr<Value> of(int64_t value) {
		return newU<Value>(nullptr, newS<Long>(value));
	}

	static UPtr<Value> of(bool value) {
		return newU<Value>(nullptr, newS<Long>(value));
	}

	static UPtr<Value> normalize(UPtr<Value> && val) {
		if (instanceof<Number>(val->_value)) {
			double d = Class::cast<Number>(val->_value)->doubleValue();

			if (Number::isMathematicalInteger(d)) {
				if ((d <= Double::MAX_SAFE_INTEGER) && (d >= Double::MIN_SAFE_INTEGER))
					return Value::of((int64_t)d);
			} else
				return Value::of(d);

		}
		return std::move(val);
	}

	/*template <class T, enableIf<std::is_base_of<Number, T>>...>
	static UPtr<Value> of(SPtr<T> const& value) {
		return valueOf(value->doubleValue());
	}*/

	static UPtr<Value> of(SPtr<Object> const& value, SPtr<String> const& varName) {
		return newU<Value>(nullptr, value, varName);
	}

	static UPtr<Value> of(SPtr<Object> const& value,
						  ValueDomain domain = ValueDomain::DEFAULT) {
		return newU<Value>(nullptr, value, nullptr, domain);
	}

	static UPtr<Value> assignableOf(SPtr<Assignable> assignable,
									SPtr<Object> const& value, SPtr<String> const& varName,
									ValueDomain domain = ValueDomain::DEFAULT) {
		return newU<Value>(assignable, value, varName, domain);
	}

	static UPtr<Value> assignableOf(SPtr<Assignable> assignable,
									SPtr<Object> const& value,
									ValueDomain domain = ValueDomain::DEFAULT) {
		return newU<Value>(assignable, value, nullptr, domain);
	}

	static UPtr<Value> Nil() {
		return newU<Value>(nullptr, nullptr, nullptr);
	}

	static UPtr<Value> Nil(SPtr<String> const& varName) {
		return newU<Value>(nullptr, nullptr, varName);
	}

	SPtr<Object> getValue() const {
		return _value;
	}

	SPtr<String> getName() const {
		return _name;
	}

	ValueDomain getDomain() const {
		return _domain;
	}

	bool isNil() const {
		return !_value;
	}

	bool isVoid() const {
		return (_value.get() == &_void);
	}

	template <class T>
	bool is() const {
		return instanceof<T>(_value);
	}

	void assign(SPtr<Object> const& value) {
		if (_assignable)
			_assignable->assign(value);
		else
			throw EvaluationException(_HERE_, "Can only assign to lvalues");
	}

	/** @throws EvaluationException */
	static UPtr<String> asString(SPtr<Object> const& value) {
		if (!value)
			return nullptr;
		if (instanceof<Number>(value)) {
			double d = (Class::castPtr<Number>(value))->doubleValue();
			if (Number::isMathematicalInteger(d))
				return Long::toString((long) d);
			else
				return Double::toString(d);
		} else
			return value->toString();
	}

	/** @throws EvaluationException */
	UPtr<String> asString() {
		return asString(_value);
	}

	/** @throws EvaluationException */
	UPtr<Value> inverse() {
		if (isNil())
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *n = Class::castPtr<Number>(_value);
			return Value::of(-n->doubleValue());
		}
		throw EvaluationException(_HERE_, "-", _value->getClass());
	}

	/**
	 * Checks if an object is true
	 * @param obj  the object
	 * @return Boolean: boolean value; Number: true if not 0; Object: true if not null
	 */
	static bool isTrue(SPtr<Object> const& obj) {
		if (!obj)
			return false;
		if (instanceof<Boolean>(obj))
			return Class::castPtr<Boolean>(obj)->booleanValue();
		if (instanceof<Number>(obj))
			return (Class::castPtr<Number>(obj)->doubleValue() != 0);
		return true;
	}

	static bool isTrue(UPtr<Value> const& v) {
		return isTrue(v->_value);
	}

	/** @throws EvaluationException */
	UPtr<Value> logicalNegate() {
		return Value::of(newS<Integer>(isTrue(_value) ? 0 : 1));
	}

	/** @throws EvaluationException */
	static UPtr<Value> add(UPtr<Value> first, UPtr<Value> second) {
		if (first->isNil() || (second->isNil()))
			return Value::Nil();
		if (first->isVoid())
			return second;
		if (second->isVoid())
			return first;
		if (instanceof<Number>(first->_value)) {
			Number *v1 = Class::castPtr<Number>(first->_value);
			if (instanceof<Number>(second->_value)) {
				Number *v2 = Class::castPtr<Number>(second->_value);
				return Value::of(v1->doubleValue() + v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "+", first->_value->getClass(), second->_value->getClass());
		} else if (instanceof<BasicString>(first->_value)) {
			BasicString *v1 = Class::castPtr<BasicString>(first->_value);
			if (instanceof<BasicString>(second->_value)) {
				BasicString *v2 = Class::castPtr<BasicString>(second->_value);
				StringBuilder result(*v1);
				result += *v2;
				return Value::of(result.toString());
			} else
				throw EvaluationException(_HERE_, "+", first->_value->getClass(), second->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "+", first->_value->getClass(), second->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> subtract(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() - v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "-", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "-", _value->getClass(), other->_value->getClass());
	}

	static UPtr<Value> logicalAnd(UPtr<Value> first, UPtr<Value> second) {
		return (isTrue(first->_value) ? std::move(second) : std::move(first));
	}

	static UPtr<Value> logicalOr(UPtr<Value> first, UPtr<Value> second) {
		return (isTrue(first->_value) ? std::move(first) : std::move(second));
	}

	/** @throws EvaluationException */
	UPtr<Value> multiply(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() * v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "*", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "*", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> divide(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() / v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "/", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "/", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> remainder(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(std::fmod(v1->doubleValue(), v2->doubleValue()));
			} else
				throw EvaluationException(_HERE_, "%", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "%", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> gt(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() > v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, ">", _value->getClass(), other->_value->getClass());
		} else if (instanceof<BasicString>(_value)) {
			BasicString *v1 = Class::castPtr<BasicString>(_value);
			if (instanceof<BasicString>(other->_value)) {
				BasicString *v2 = Class::castPtr<BasicString>(other->_value);
				return Value::of(v1->compareTo(*v2) > 0);
			} else
				throw EvaluationException(_HERE_, ">", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, ">", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> gte(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() >= v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, ">=", _value->getClass(), other->_value->getClass());
		} else if (instanceof<BasicString>(_value)) {
			BasicString *v1 = Class::castPtr<BasicString>(_value);
			if (instanceof<BasicString>(other->_value)) {
				BasicString *v2 = Class::castPtr<BasicString>(other->_value);
				return Value::of(v1->compareTo(*v2) >= 0);
			} else
				throw EvaluationException(_HERE_, ">=", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, ">=", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> lt(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() < v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "<", _value->getClass(), other->_value->getClass());
		} else if (instanceof<BasicString>(_value)) {
			BasicString *v1 = Class::castPtr<BasicString>(_value);
			if (instanceof<BasicString>(other->_value)) {
				BasicString *v2 = Class::castPtr<BasicString>(other->_value);
				return Value::of(v1->compareTo(*v2) < 0);
			} else
				throw EvaluationException(_HERE_, "<", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "<", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> lte(UPtr<Value> const& other) {
		if (isNil() || (other->isNil()))
			return Value::Nil();
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() <= v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "<=", _value->getClass(), other->_value->getClass());
		} else if (instanceof<BasicString>(_value)) {
			BasicString *v1 = Class::castPtr<BasicString>(_value);
			if (instanceof<BasicString>(other->_value)) {
				BasicString *v2 = Class::castPtr<BasicString>(other->_value);
				return Value::of(v1->compareTo(*v2) <= 0);
			} else
				throw EvaluationException(_HERE_, "<=", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "<=", _value->getClass(), other->_value->getClass());
	}

private:
	/** @throws EvaluationException */
	bool _eq(UPtr<Value> const& other) {
		if ((!this->_value) || (!other->_value))
			return ((!this->_value) && (!other->_value));
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return (v1->doubleValue() == v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "==", _value->getClass(), other->_value->getClass());
		} else if (instanceof<BasicString>(_value)) {
			BasicString *v1 = Class::castPtr<BasicString>(_value);
			if (instanceof<BasicString>(other->_value)) {
				BasicString *v2 = Class::castPtr<BasicString>(other->_value);
				return v1->equals(*v2);
			} else
				throw EvaluationException(_HERE_, "==", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "==", _value->getClass(), other->_value->getClass());
	}

public:
	UPtr<Value> eq(UPtr<Value> const& other) {
		return Value::of(_eq(other));
	}

	UPtr<Value> neq(UPtr<Value> const& other) {
		return Value::of(!_eq(other));
	}

private:
	/** @throws EvaluationException */
	static int64_t getIndex(UPtr<Value> const& arg) {
		if (!instanceof<Number>(arg->_value))
			throw EvaluationException(_HERE_, fmt::format("Operator '[]': expected numeric index, got '{}'", arg->_value->getClass().getName()).c_str());
		Number *index = Class::castPtr<Number>(arg->_value);
		if (!Number::isMathematicalInteger(index->doubleValue()))
			throw EvaluationException(_HERE_, fmt::format("Operator '[]': expected integer index, got {}", index->doubleValue()).c_str());
		return index->longValue();
	}

public:
	/** @throws EvaluationException */
	UPtr<Value> index(UPtr<Value> const& arg) {
		if (isNil() || arg->isNil())
			return Value::Nil();
		if (instanceof<Map<BasicString, Object>>(_value)) {
			return newU<Value>(
				nullptr, (Class::castPtr<Map<BasicString, Object>>(_value))->get(*Class::castPtr<String>(arg->_value))
			);
		} else if (instanceof<Map<Object, Object>>(_value)) {
			SPtr<Object> val = (Class::castPtr<Map<Object, Object>>(_value))->get(*(arg->_value));
			if (val)
				return newU<Value>(nullptr, std::move(val));

			// expression indices are internally normalized to Long. Retry with Double
			if (arg->_value->getClass() == classOf<Long>::_class()) {
				SPtr<Object> dVal = newS<Double>(Class::castPtr<Long>(arg->_value)->doubleValue());
				return Value::of((Class::castPtr<Map<Object, Object>>(_value))->get(*dVal));
			}

			return Value::Nil();
		} else if (instanceof<List<Object>>(_value)) {
			List<Object> *list = Class::castPtr<List<Object>>(_value);
			int64_t i = getIndex(arg);
			if ((i < 0) || (i >= (int64_t)list->size()))
				throw EvaluationException(_HERE_, "Array index out of bounds");
			return newU<Value>(nullptr, list->get((int)i));
		} else
			throw EvaluationException(_HERE_, "[]", _value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> member(SPtr<String> const& memberName, SPtr<Resolver> const& resolver) {
		if (isNil()) {
			// maybe it is a dotted variable name
			if (!_name) {
				// this is not a named variable, no dotted expression possible
				return Value::Nil();
			}
			SPtr<String> dottedName = newS<String>(fmt::format("{}.{}", *_name, *memberName));
			SPtr<Object> val = resolver->getVar(*dottedName, ValueDomain::DEFAULT);
			if (val)
				return newU<Value>(nullptr, val);
			return Nil(dottedName);
		} else {
			if (instanceof<Resolver>(_value))
				return newU<Value>(nullptr, (Class::cast<Resolver>(_value))->getVar(*memberName, ValueDomain::DEFAULT), memberName);
			else if (instanceof<Map<BasicString, Object>>(_value)) {
				return newU<Value>(nullptr, (Class::cast<Map<BasicString, Object>>(_value))->get(*memberName), memberName);
			} else
				throw EvaluationException(_HERE_, ".", _value->getClass());
		}
	}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_VALUE_H
