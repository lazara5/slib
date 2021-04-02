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

class Value {
friend class ExpressionEvaluator;
friend class ResultHolder;
private:
	SPtr<Object> _value;
	SPtr<String> _name;
	SPtr<String> _tag;
public:
	Value(SPtr<Object> const& value, SPtr<String> const& name = nullptr, SPtr<String> const& tag = nullptr)
	: _value(value)
	, _name(name)
	, _tag(tag) {}

	static UPtr<Value> of(SPtr<Object> const& value) {
		return newU<Value>(value);
	}

	static UPtr<Value> of(double value) {
		return newU<Value>(newS<Double>(value));
	}

	static UPtr<Value> of(int64_t value) {
		return newU<Value>(newS<Long>(value));
	}

	static UPtr<Value> of(bool value) {
		return newU<Value>(newS<Long>(value));
	}

	static UPtr<Value> normalize(UPtr<Value> && val) {
		if (instanceof<Number>(val->_value)) {
			double d = Class::cast<Number>(val->_value)->doubleValue();

			if (Number::isMathematicalInteger(d)) {
				if ((d <= Double::MAX_SAFE_INTEGER) && (d >= Double::MIN_SAFE_INTEGER))
					return newU<Value>(newS<Long>((int64_t)d));
			}

		}
		return std::move(val);
	}

	/*template <class T, enableIf<std::is_base_of<Number, T>>...>
	static UPtr<Value> of(SPtr<T> const& value) {
		return valueOf(value->doubleValue());
	}*/

	static UPtr<Value> of(SPtr<Object> const& value, SPtr<String> const& varName) {
		return newU<Value>(value, varName);
	}

	static UPtr<Value> taggedOf(SPtr<Object> const& value, SPtr<String> const& tag) {
		return newU<Value>(value, nullptr, tag);
	}

	static UPtr<Value> Nil() {
		return newU<Value>(nullptr, nullptr);
	}

	static UPtr<Value> Nil(SPtr<String> const& varName) {
		return newU<Value>(nullptr, varName);
	}

	SPtr<Object> getValue() const {
		return _value;
	}

	SPtr<String> getName() const {
		return _name;
	}

	SPtr<String> getTag() const {
		return _tag;
	}

	UPtr<Value> clone() {
		return newU<Value>(_value, _name);
	}

	bool isNil() const {
		return !_value;
	}

	/** @throws EvaluationException */
	static void checkNil(SPtr<Object> const& value, SPtr<String> const& name) {
		if (!value) {
			if (name)
				throw MissingSymbolException(_HERE_, name);
			else
				throw NilValueException(_HERE_);
		}
	}

	/** @throws EvaluationException */
	static void checkNil(Value const& v) {
		checkNil(v._value, v._name);
	}

	/** @throws EvaluationException */
	static UPtr<String> asString(SPtr<Object> const& value, SPtr<String> const& name = nullptr) {
		if (instanceof<Number>(value)) {
			double d = (Class::castPtr<Number>(value))->doubleValue();
			if (Number::isMathematicalInteger(d))
				return Long::toString((long) d);
			else
				return Double::toString(d);
		} else {
			checkNil(value, name);
			return value->toString();
		}
	}

	/** @throws EvaluationException */
	UPtr<String> asString() {
		return asString(_value, _name);
	}

	/** @throws EvaluationException */
	UPtr<Value> inverse() {
		checkNil(*this);
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
		return newU<Value>(newS<Integer>(isTrue(_value) ? 0 : 1));
	}

	/** @throws EvaluationException */
	UPtr<Value> add(UPtr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (instanceof<Number>(_value)) {
			Number *v1 = Class::castPtr<Number>(_value);
			if (instanceof<Number>(other->_value)) {
				Number *v2 = Class::castPtr<Number>(other->_value);
				return Value::of(v1->doubleValue() + v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "+", _value->getClass(), other->_value->getClass());
		} else if (instanceof<BasicString>(_value)) {
			BasicString *v1 = Class::castPtr<BasicString>(_value);
			if (instanceof<BasicString>(other->_value)) {
				BasicString *v2 = Class::castPtr<BasicString>(other->_value);
				StringBuilder result(*v1);
				result += *v2;
				return newU<Value>(result.toString());
			} else
				throw EvaluationException(_HERE_, "+", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "+", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> subtract(UPtr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
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

	UPtr<Value> logicalAnd(UPtr<Value> &other) {
		return (isTrue(_value) ? std::move(other) : clone());
	}

	UPtr<Value> logicalOr(UPtr<Value> &other) {
		return (isTrue(_value) ? clone() : std::move(other));
	}

	/** @throws EvaluationException */
	UPtr<Value> multiply(UPtr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
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
		checkNil(*this);
		checkNil(*other);
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
		checkNil(*this);
		checkNil(*other);
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
		checkNil(*this);
		checkNil(*other);
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
		checkNil(*this);
		checkNil(*other);
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
		checkNil(*this);
		checkNil(*other);
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
		checkNil(*this);
		checkNil(*other);
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
		checkNil(*this);
		checkNil(*arg);
		if (instanceof<Map<String, Object>>(_value)) {
			return newU<Value>(
				(Class::castPtr<Map<String, Object>>(_value))->get(*Class::castPtr<String>(arg->_value))
			);
		} else if (instanceof<Map<Object, Object>>(_value)) {
			SPtr<Object> val = (Class::castPtr<Map<Object, Object>>(_value))->get(*(arg->_value));
			if (val)
				return newU<Value>(std::move(val));

			// expression indices are internally normalized to Long. Retry with Double
			if (arg->_value->getClass() == classOf<Long>::_class()) {
				SPtr<Object> dVal = newS<Double>(Class::castPtr<Long>(arg->_value)->doubleValue());
				return Value::of((Class::castPtr<Map<Object, Object>>(_value))->get(*dVal));
			}

			return Value::Nil();
		// TODO: Array
		} else if (instanceof<List<Object>>(_value)) {
			List<Object> *list = Class::castPtr<List<Object>>(_value);
			int64_t i = getIndex(arg);
			if ((i < 0) || (i >= (int64_t)list->size()))
				throw EvaluationException(_HERE_, "Array index out of bounds");
			return newU<Value>(list->get((int)i));
		} else
			throw EvaluationException(_HERE_, "[]", _value->getClass());
	}

	/** @throws EvaluationException */
	UPtr<Value> member(SPtr<String> const& memberName, SPtr<Resolver> const& resolver) {
		if (isNil()) {
			// maybe it is a dotted variable name
			if (!_name) {
				// this is not a named variable, no dotted expression possible
				checkNil(*this);
			}
			SPtr<String> dottedName = newS<String>(fmt::format("{}.{}", *_name, *memberName));
			SPtr<Object> val = resolver->getVar(*dottedName);
			if (val)
				return newU<Value>(val);
			return Nil(dottedName);
		} else {
			if (instanceof<Resolver>(_value))
				return newU<Value>((Class::cast<Resolver>(_value))->getVar(*memberName), memberName);
			else if (instanceof<Map<String, Object>>(_value)) {
				return newU<Value>((Class::cast<Map<String, Object>>(_value))->get(*memberName), memberName);
			} else
				throw EvaluationException(_HERE_, ".", _value->getClass());
		}
	}
};
template <class K>
class KeyValueTuple : virtual public Object {
public:
	TYPE_INFO(KeyValueTuple, CLASS(KeyValueTuple<K>), INHERITS(Object));
public:
	const SPtr<K> key;
	const SPtr<Object> value;

	KeyValueTuple(SPtr<K> const& key, SPtr<Object> const& value)
	: key(key)
	, value(value) {}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_VALUE_H
