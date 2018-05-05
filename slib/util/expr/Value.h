/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_VALUE_H
#define H_SLIB_UTIL_EXPR_VALUE_H

#include "slib/Object.h"
#include "slib/String.h"
#include "slib/StringBuilder.h"
#include "slib/util/expr/Exceptions.h"
#include "slib/util/expr/Resolver.h"
#include "slib/collections/Map.h"
#include "slib/collections/List.h"
#include "slib/Numeric.h"

namespace slib {
namespace expr {

class Value {
friend class ExpressionEvaluator;
private:
	std::shared_ptr<Object> _value;
	std::shared_ptr<String> _name;
public:
	Value(std::shared_ptr<Object> const& value, std::shared_ptr<String> const& name = nullptr)
	:_value(value)
	,_name(name) {}

	static std::shared_ptr<Value> Nil() {
		return std::make_shared<Value>(nullptr, nullptr);
	}

	static std::shared_ptr<Value> Nil(std::shared_ptr<String> const& varName) {
		return std::make_shared<Value>(nullptr, varName);
	}

	std::shared_ptr<Value> clone() {
		return std::make_shared<Value>(_value, _name);
	}

	bool isNil() const {
		return (bool)_value;
	}

	/** @throws EvaluationException */
	static void checkNil(Value const& v){
		if (!v._value) {
			if (!v._name)
				throw MissingSymbolException(_HERE_, v._name);
			else
				throw NilValueException(_HERE_);
		}
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> inverse() {
		checkNil(*this);
		if (_value->instanceof<Number>()) {
			Number *n = Class::cast<Number>(_value);
			return std::make_shared<Value>(std::make_shared<Double>(-n->doubleValue()));
		}
		throw EvaluationException(_HERE_, "-", _value->getClass());
	}

	/**
	 * Checks if an object is true
	 * @param obj  the object
	 * @return Boolean: boolean value; Number: true if not 0; Object: true if not null
	 */
	static bool isTrue(std::shared_ptr<Object> const& obj) {
		if (!obj)
			return false;
		if (obj->instanceof<Boolean>())
			return Class::cast<Boolean>(obj)->booleanValue();
		if (obj->instanceof<Number>())
			return (Class::cast<Number>(obj)->doubleValue() != 0);
		return true;
	}

	static bool isTrue(std::shared_ptr<Value> const& v) {
		return isTrue(v->_value);
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> logicalNegate() {
		return std::make_shared<Value>(std::make_shared<Integer>(isTrue(_value) ? 0 : 1));
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> add(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return std::make_shared<Value>(std::make_shared<Double>(v1->doubleValue() + v2->doubleValue()));
			} else
				throw EvaluationException(_HERE_, "+", _value->getClass(), other->_value->getClass());
		} else if (_value->instanceof<BasicString>()) {
			BasicString *v1 = Class::cast<BasicString>(_value);
			if (other->_value->instanceof<BasicString>()) {
				BasicString *v2 = Class::cast<BasicString>(other->_value);
				StringBuilder result(*v1);
				result += *v2;
				return std::make_shared<Value>(result.toString());
			} else
				throw EvaluationException(_HERE_, "+", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "+", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> subtract(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return std::make_shared<Value>(std::make_shared<Double>(v1->doubleValue() - v2->doubleValue()));
			} else
				throw EvaluationException(_HERE_, "-", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "-", _value->getClass(), other->_value->getClass());
	}

	std::shared_ptr<Value> logicalAnd(std::shared_ptr<Value> const& other) {
		return (isTrue(_value) ? other : clone());
	}

	std::shared_ptr<Value> logicalOr(std::shared_ptr<Value> const& other) {
		return (isTrue(_value) ? clone() : other);
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> multiply(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return std::make_shared<Value>(std::make_shared<Double>(v1->doubleValue() * v2->doubleValue()));
			} else
				throw EvaluationException(_HERE_, "*", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "*", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> divide(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return std::make_shared<Value>(std::make_shared<Double>(v1->doubleValue() / v2->doubleValue()));
			} else
				throw EvaluationException(_HERE_, "/", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "/", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> remainder(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return std::make_shared<Value>(std::make_shared<Double>(std::fmod(v1->doubleValue(), v2->doubleValue())));
			} else
				throw EvaluationException(_HERE_, "%", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "%", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	bool gt(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return (v1->doubleValue() > v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, ">", _value->getClass(), other->_value->getClass());
		} else if (_value->instanceof<BasicString>()) {
			BasicString *v1 = Class::cast<BasicString>(_value);
			if (other->_value->instanceof<BasicString>()) {
				BasicString *v2 = Class::cast<BasicString>(other->_value);
				return v1->compareTo(*v2) > 0;
			} else
				throw EvaluationException(_HERE_, ">", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, ">", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	bool gte(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return (v1->doubleValue() >= v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, ">=", _value->getClass(), other->_value->getClass());
		} else if (_value->instanceof<BasicString>()) {
			BasicString *v1 = Class::cast<BasicString>(_value);
			if (other->_value->instanceof<BasicString>()) {
				BasicString *v2 = Class::cast<BasicString>(other->_value);
				return v1->compareTo(*v2) >= 0;
			} else
				throw EvaluationException(_HERE_, ">=", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, ">=", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	bool lt(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return (v1->doubleValue() < v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "<", _value->getClass(), other->_value->getClass());
		} else if (_value->instanceof<BasicString>()) {
			BasicString *v1 = Class::cast<BasicString>(_value);
			if (other->_value->instanceof<BasicString>()) {
				BasicString *v2 = Class::cast<BasicString>(other->_value);
				return v1->compareTo(*v2) < 0;
			} else
				throw EvaluationException(_HERE_, "<", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "<", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	bool lte(std::shared_ptr<Value> const& other) {
		checkNil(*this);
		checkNil(*other);
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return (v1->doubleValue() <= v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "<=", _value->getClass(), other->_value->getClass());
		} else if (_value->instanceof<BasicString>()) {
			BasicString *v1 = Class::cast<BasicString>(_value);
			if (other->_value->instanceof<BasicString>()) {
				BasicString *v2 = Class::cast<BasicString>(other->_value);
				return v1->compareTo(*v2) <= 0;
			} else
				throw EvaluationException(_HERE_, "<=", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "<=", _value->getClass(), other->_value->getClass());
	}

	/** @throws EvaluationException */
	bool eq(std::shared_ptr<Value> const& other) {
		if ((!this->_value) || (!other->_value))
			return ((!this->_value) && (!other->_value));
		if (_value->instanceof<Number>()) {
			Number *v1 = Class::cast<Number>(_value);
			if (other->_value->instanceof<Number>()) {
				Number *v2 = Class::cast<Number>(other->_value);
				return (v1->doubleValue() == v2->doubleValue());
			} else
				throw EvaluationException(_HERE_, "==", _value->getClass(), other->_value->getClass());
		} else if (_value->instanceof<BasicString>()) {
			BasicString *v1 = Class::cast<BasicString>(_value);
			if (other->_value->instanceof<BasicString>()) {
				BasicString *v2 = Class::cast<BasicString>(other->_value);
				return v1->equals(*v2);
			} else
				throw EvaluationException(_HERE_, "==", _value->getClass(), other->_value->getClass());
		} else
			throw EvaluationException(_HERE_, "==", _value->getClass(), other->_value->getClass());
	}

private:
	/** @throws EvaluationException */
	int64_t getIndex(std::shared_ptr<Value> const& arg) {
		if (!arg->_value->instanceof<Number>())
			throw EvaluationException(_HERE_, fmt::format("Operator '[]': expected numeric index, got '{}'", arg->_value->getClass()->getName()).c_str());
		Number *index = Class::cast<Number>(arg->_value);
		if (!Number::isMathematicalInteger(index->doubleValue()))
			throw EvaluationException(_HERE_, fmt::format("Operator '[]': expected integer index, got {}", index->doubleValue()).c_str());
		return index->longValue();
	}

public:
	/** @throws EvaluationException */
	std::shared_ptr<Value> index(std::shared_ptr<Value> const& arg) {
		checkNil(*this);
		checkNil(*arg);
		if (_value->instanceof<Map<String, Object>>()) {
			return std::make_shared<Value>(
				(Class::cast<Map<String, Object>>(_value))->get(*Class::cast<String>(arg->_value))
			);
		// TODO: Array
		} else if (_value->instanceof<List<Object>>()) {
			List<Object> *list = Class::cast<List<Object>>(_value);
			int64_t i = getIndex(arg);
			if ((i < 0) || (i >= list->size()))
				throw EvaluationException(_HERE_, "Array index out of bounds");
			return std::make_shared<Value>(list->get((int)i));
		} else
			throw EvaluationException(_HERE_, "[]", _value->getClass());
	}

	/** @throws EvaluationException */
	std::shared_ptr<Value> member(std::shared_ptr<String> const& memberName, std::shared_ptr<Resolver> const& resolver) {
		if (isNil()) {
			// maybe it is a dotted variable name
			if (!_name) {
				// this is not a named variable, no dotted expression possible
				checkNil(*this);
			}
			std::shared_ptr<String> dottedName = std::make_shared<String>(fmt::format("{}.{}", *_name, *memberName));
			std::shared_ptr<Object> val = resolver->getVar(*dottedName);
			if (val)
				return std::make_shared<Value>(val);
			return Nil(dottedName);
		} else {
			if (_value->instanceof<Resolver>())
				return std::make_shared<Value>((Class::cast<Resolver>(_value))->getVar(*memberName), memberName);
			else if (_value->instanceof<Map<String, Object>>())
				return std::make_shared<Value>((Class::cast<Map<String, Object>>(_value))->get(*memberName), memberName);
			else
				throw EvaluationException(_HERE_, ".", _value->getClass());
		}
	}
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_VALUE_H
