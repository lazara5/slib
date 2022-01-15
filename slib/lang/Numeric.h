/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_NUMERIC_H
#define H_SLIB_NUMERIC_H

#include "slib/lang/Object.h"
#include "slib/exception/NumericExceptions.h"
#include "slib/lang/String.h"
#include "slib/lang/Character.h"
#include "slib/third-party/fast_float/fast_float.h"

#include "fmt/format.h"

#include <string>
#include <sstream>

#include <math.h>
#include <limits.h>
#include <cmath>

#define slib_min(a,b)  (((a) < (b)) ? (a) : (b))

namespace slib {

class Number : virtual public Object {
public:
	TYPE_INFO(Number, CLASS(Number), INHERITS(Object));
public:
	virtual ~Number();

	virtual int64_t longValue() const = 0;

	virtual double doubleValue() const = 0;

	static UPtr<Number> createNumber(UPtr<String> const& str);
	static UPtr<Number> createLongOrDouble(UPtr<String> const& str);

	static bool isMathematicalInteger(double val);
};

/** 64-bit signed integer */
class Long : public Number {
public:
	TYPE_INFO(Long, CLASS(Long), INHERITS(Number));
private:
	int64_t _value;
public:
	static constexpr int64_t MIN_VALUE = INT64_MIN;
	static constexpr int64_t MAX_VALUE = INT64_MAX;

	Long()
	:_value(0) {}

	Long(int64_t value)
	:_value(value) {}

	/**
	 * Returns a hash code for this <code>Long</code>. The result is
	 * the exclusive OR of the two halves of the primitive
	 * <code>int64_t</code> value held by this <code>Long</code>
	 * object. That is, the hashcode is the value of the expression:
	 * <blockquote><pre>
	 * (int)(_value()^((unsigned)_value()&gt;&gt;32))
	 * </pre></blockquote>
	 *
	 * @return  a hash code value for this object.
	 */
	virtual int32_t hashCode() const override {
		return (int32_t)((uint64_t)_value ^ (((uint64_t)_value) >> 32));
	}

	Long& operator =(const Long& other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	using Object::equals;

	bool equals(Long const& other) const {
		return (_value == other._value);
	}

	virtual bool equals(Object const& other) const override {
		if (instanceof<Long>(other))
			return _value == Class::constCast<Long>(&other)->_value;
		return false;
	}

	bool equals(SPtr<Long> const& other) const {
		if (!other)
			return false;
		return (_value == other->_value);
	}

	bool operator==(const Long& other) const {
		return equals(other);
	}

	virtual int64_t longValue() const override;
	virtual double doubleValue() const override;

	template <class S>
	static int64_t parseLong(S const& s, int radix = 10) {
		const char *str = strData(CPtr(s));
		if (!str)
			throw NumberFormatException(_HERE_, "null");

		long result = 0;
		bool negative = false;
		size_t i = 0;
		size_t len = strLen(CPtr(s));

		int64_t limit = -MAX_VALUE;
		int64_t cutoff;
		int digit;

		if (len > 0) {
			char firstChar = str[0];
			if (firstChar < '0') { // Look for leading sign
				if (firstChar == '-') {
					negative = true;
					limit = MIN_VALUE;
				} else if (firstChar != '+')
					throw NumberFormatException(_HERE_, "Not a decimal number");

				if (len == 1) // No digit remaining
					throw NumberFormatException(_HERE_, "Not a decimal number");
				i++;
			}
			cutoff = limit / radix;
			while (i < len) {
				digit = Character::digit(str[i++], radix);
				if (digit < 0)
					throw NumberFormatException(_HERE_, "Not a decimal number");
				if (result < cutoff)
					throw NumberFormatException(_HERE_, "Not a decimal number");
				result *= radix;
				if (result < limit + digit)
					throw NumberFormatException(_HERE_, "Not a decimal number");
				result -= digit;
			}
		} else
			throw NumberFormatException(_HERE_, "Not a decimal number");
		return negative ? result : (-result);
	}

	/** @throws NumberFormatException */
	template <class S>
	static int64_t decode(S const* str) {
		const char *buffer = str ? str->c_str() : nullptr;
		size_t len = str ? str->length() : 0;

		int radix = 10;
		size_t index = 0;
		bool negative = false;

		if (len == 0)
			throw NumberFormatException(_HERE_, "Empty string");

		char firstChar = buffer[0];
		if (firstChar == '-') {
			negative = true;
			index++;
		} else if (firstChar == '+')
			index++;

		if (String::startsWith(str, "0x", index) || String::startsWith(str, "0X", index)) {
			radix = 16;
			index += 2;
		} else if (String::startsWith(str, "#", index)) {
			radix = 16;
			index++;
		} else if (String::startsWith(str, "0", index) && (len > 1 + index)) {
			radix = 8;
			index ++;
		}

		if (String::startsWith(str, "-", index) || String::startsWith(str, "+", index))
			throw NumberFormatException("Misplaced sign character");

		int64_t result = 0;

		try {
			result = parseLong(buffer + index, radix);
			if (negative)
				result = -result;
		} catch (NumberFormatException const&) {
			result = negative ? parseLong(std::string("-") + (buffer + index), radix) :
								parseLong(buffer + index, radix);
		}

		return result;
	}

	static UPtr<String> toString(int64_t i) {
		std::stringstream stream;
		stream << i;
		return newU<String>(stream.str());
	}

	virtual UPtr<String> toString() const override {
		return toString(_value);
	}
};

/** 64-bit unsigned integer */
class ULong : public Number {
public:
	TYPE_INFO(ULong, CLASS(ULong), INHERITS(Number));
private:
	uint64_t _value;
public:
	static constexpr uint64_t MAX_VALUE = UINT64_MAX;

	ULong(uint64_t value)
	:_value(value) {}

	/**
	 * Returns a hash code for this <code>Long</code>. The result is
	 * the exclusive OR of the two halves of the primitive
	 * <code>uint64_t</code> value held by this <code>Long</code>
	 * object. That is, the hashcode is the value of the expression:
	 * <blockquote><pre>
	 * (int)(_value()^((unsigned)_value()&gt;&gt;32))
	 * </pre></blockquote>
	 *
	 * @return  a hash code value for this object.
	 */
	int32_t hashCode() const override {
		return (int)(_value ^ (_value >> 32));
	}

	ULong& operator =(const ULong& other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	using Object::equals;

	bool equals(const ULong& other) const {
		return (_value == other._value);
	}

	bool equals(SPtr<ULong> const& other) const {
		if (!other)
			return false;
		return (_value == other->_value);
	}

	bool operator ==(const ULong& other) const {
		return equals(other);
	}

	bool operator <(const ULong& other) const {
		return _value < other._value;
	}

	bool operator >(const ULong& other) const {
		return _value > other._value;
	}

	uint64_t ulongValue() const {
		return _value;
	}

	virtual double doubleValue() const override;

	/** @throws NumberFormatException */
	template <class S>
	static int64_t parseULong(S const& s, int radix = 10) {
		const char *str = strData(CPtr(s));
		if (!str)
			throw NumberFormatException(_HERE_, "null");

		size_t len = strLen(CPtr(s));
		if (len > 0) {
			char firstChar = str[0];

			if (firstChar == '-') {
				throw NumberFormatException(_HERE_,
					"Illegal leading minus sign on unsigned string");
			} else {
				if ((len <= 12) || (radix == 10 && len < 18))
					return Long::parseLong(s, radix);

				int64_t prefix = Long::parseLong(StringView(str, len - 1), radix);
				int suffix = Character::digit(str[len - 1], radix);
				if (suffix < 0)
					throw NumberFormatException(_HERE_, "Not a decimal number");
				int64_t result = prefix * radix + suffix;
				if ((uint64_t)result < (uint64_t)prefix)
					throw NumericOverflowException(_HERE_, "Out of range");
				return (uint64_t)result;
			}
		} else
			throw NumberFormatException(_HERE_, "Not a decimal number");
	}

	struct hash {
		/** hash based on mix from Murmur3 */
		size_t operator()(const uint64_t& key) const {
			uint64_t k = key ^ (key >> 33);
			k *= 0xff51afd7ed558ccd;
			k ^= k >> 33;
			k *= 0xc4ceb9fe1a85ec53;
			k ^= k >> 33;
			return (size_t) k;
		}
	};

	static UPtr<String> toString(uint64_t i) {
		std::stringstream stream;
		stream << i;
		return newU<String>(stream.str());
	}

	virtual UPtr<String> toString() const override {
		return toString(_value);
	}
};

/** 32-bit signed integer */
class Integer : public Number {
public:
	TYPE_INFO(Integer, CLASS(Integer), INHERITS(Number));
private:
	int32_t _value;
public:
	static constexpr int32_t MIN_VALUE = INT32_MIN;
	static constexpr int32_t MAX_VALUE = INT32_MAX;

	Integer()
	:_value(0) {}

	Integer(int32_t value)
	:_value(value) {}

	Integer(const Integer& other)
	: _value(other._value) {}

	virtual int32_t hashCode() const override {
		return _value;
	}

	Integer& operator =(const Integer& other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	bool equals(const Integer& other) const {
		return (_value == other._value);
	}

	virtual bool equals(Object const& other) const override {
		if (instanceof<Integer>(other))
			return _value == Class::constCast<Integer>(&other)->_value;
		return false;
	}

	bool equals(SPtr<Integer> const& other) const {
		if (!other)
			return false;
		return (_value == other->_value);
	}

	bool operator ==(const Integer& other) const {
		return equals(other);
	}

	int32_t intValue() const {
		return _value;
	}

	operator int32_t() const {
		return _value;
	}

	virtual int64_t longValue() const override;
	virtual double doubleValue() const override;

	/** @throws NumberFormatException */
	template <class S>
	static int32_t parseInt(S const& str, int radix = 10) {
		int64_t val = Long::parseLong(str, radix);
		if ((val > MAX_VALUE) || (val < MIN_VALUE))
			throw NumericOverflowException(_HERE_, "Out of range");
		return (int32_t)val;
	}

	/** @throws NumberFormatException */
	template <class S>
	static int32_t decode(S const* str) {
		const char *buffer = strData(str);
		size_t len = strLen(str);

		int radix = 10;
		size_t index = 0;
		bool negative = false;

		if (len == 0)
			throw NumberFormatException(_HERE_, "Empty string");

		char firstChar = buffer[0];
		if (firstChar == '-') {
			negative = true;
			index++;
		} else if (firstChar == '+')
			index++;

		if (String::startsWith(str, "0x", index) || String::startsWith(str, "0X", index)) {
			radix = 16;
			index += 2;
		} else if (String::startsWith(str, "#", index)) {
			radix = 16;
			index++;
		} else if (String::startsWith(str, "0", index) && (len > 1 + index)) {
			radix = 8;
			index ++;
		}

		if (String::startsWith(str, "-", index) || String::startsWith(str, "+", index))
			throw NumberFormatException("Misplaced sign character");

		int32_t result = 0;

		try {
			result = parseInt(StringView(buffer + index, len - index), radix);
			if (negative)
				result = -result;
		} catch (NumberFormatException const&) {
			result = negative ? parseInt(std::string("-") + (buffer + index), radix) :
								parseInt(buffer + index, radix);
		}

		return result;
	}

	static UPtr<String> toString(int32_t i) {
		std::stringstream stream;
		stream << i;
		return newU<String>(stream.str());
	}

	/**
	 * Returns a string representation of the integer argument as an
	 * unsigned integer in base&nbsp;16.
	 * <p>
	 * The unsigned integer value is the argument plus 2<sup>32</sup>
	 * if the argument is negative; otherwise, it is equal to the
	 * argument. This value is converted to a string of digits
	 * in hexadecimal (base&nbsp;16) with no extra leading
	 * 0s. If the unsigned magnitude is zero, it is
	 * represented by a single zero character <code>'0'</code>.
	 * Otherwise, the first character of the representation of the
	 * unsigned magnitude will not be the zero character.
	 * The hexadecimal digits are lowercase.
	 *
	 * @param i  an integer to be converted to a string.
	 * @return the string representation of the unsigned integer value
	 *         represented by the argument in hexadecimal (base 16).
	 */
	static String toHexString(int32_t i) {
		return String(fmt::format("{:x}", i));
	}

	virtual UPtr<String> toString() const override {
		return toString(_value);
	}
};

/** 32-bit unsigned integer */
class UInt : public Number {
public:
	TYPE_INFO(UInt, CLASS(UInt), INHERITS(Number));
private:
	uint32_t _value;
public:
	static constexpr uint32_t MIN_VALUE = 0;
	static constexpr uint32_t MAX_VALUE = UINT32_MAX;

	UInt()
	:_value(0) {}

	UInt(uint32_t value)
	:_value(value) {}

	UInt(const UInt& other)
	: _value(other._value) {}

	int32_t hashCode() const override {
		return (int)_value;
	}

	UInt& operator =(const UInt& other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	using Object::equals;

	bool equals(const UInt& other) const {
		return (_value == other._value);
	}

	bool equals(SPtr<UInt> const& other) const {
		if (!other)
			return false;
		return (_value == other->_value);
	}

	bool operator ==(const UInt& other) const {
		return equals(other);
	}

	uint32_t uintValue() const {
		return _value;
	}

	operator uint32_t() const {
		return _value;
	}

	virtual int64_t longValue() const override;
	virtual double doubleValue() const override;

	template <class S>
	static uint32_t parseUInt(S const& s, int radix = 10) {
		int64_t val = Long::parseLong(s, radix);
		if ((val < 0) || (val > MAX_VALUE))
			throw NumericOverflowException(_HERE_, "Out of range");
		return (uint32_t)val;
	}

	static UPtr<String> toString(uint32_t i) {
		std::stringstream stream;
		stream << i;
		return newU<String>(stream.str());
	}

	/**
	 * Returns a string representation of the integer argument as an
	 * unsigned integer in base&nbsp;16.
	 * <p>
	 * The unsigned integer value is the argument plus 2<sup>32</sup>
	 * if the argument is negative; otherwise, it is equal to the
	 * argument. This value is converted to a string of digits
	 * in hexadecimal (base&nbsp;16) with no extra leading
	 * 0s. If the unsigned magnitude is zero, it is
	 * represented by a single zero character <code>'0'</code>.
	 * Otherwise, the first character of the representation of the
	 * unsigned magnitude will not be the zero character.
	 * The hexadecimal digits are lowercase.
	 *
	 * @param i  an integer to be converted to a string.
	 * @return the string representation of the unsigned integer value
	 *         represented by the argument in hexadecimal (base 16).
	 */
	static String toHexString(uint32_t i) {
		return String(fmt::format("{:x}", i));
	}

	virtual UPtr<String> toString() const override {
		return toString(_value);
	}
};

class Short : public Number {
public:
	TYPE_INFO(Short, CLASS(Short), INHERITS(Number));
private:
	short _value;
public:
	static constexpr short MIN_VALUE = INT16_MIN;
	static constexpr short MAX_VALUE = INT16_MAX;

	Short()
	:_value(0) {}

	Short(short value)
	:_value(value) {}

	Short(const Short& other)
	: _value(other._value) {}

	virtual int32_t hashCode() const override {
		return (int32_t)_value;
	}

	short shortValue() const {
		return _value;
	}

	virtual int64_t longValue() const override;
	virtual double doubleValue() const override;

	/** @throws NumberFormatException */
	static short parseShort(const char *str, int radix) {
		int i = Integer::parseInt(str, radix);
		if (i < MIN_VALUE || i > MAX_VALUE)
			throw NumericOverflowException(_HERE_, "Out of range");
		return (short)i;
	}

	static UPtr<String> toString(short s) {
		return Integer::toString(s);
	}

	virtual UPtr<String> toString() const override {
		return toString(_value);
	}
};

/** Double value */
class Double : public Number {
public:
	TYPE_INFO(Double, CLASS(Double), INHERITS(Number));
private:
	double _value;
public:
	static constexpr double MIN_VALUE = DBL_MIN;
	static constexpr double MAX_VALUE = DBL_MAX;
protected:
	static uint64_t doubleToLongBits(double value) {
		if (std::isnan(value))
			return 0x7ff8000000000000LL;
		else {
			uint64_t res = 0;
			size_t len = slib_min(sizeof(double), sizeof(uint64_t));
			memcpy(&res, &value, len);
			return res;
		}
	}
public:
	static constexpr int64_t MIN_SAFE_INTEGER = -9007199254740991;
	static constexpr int64_t MAX_SAFE_INTEGER = 9007199254740991;

	Double(double value)
	:_value(value) {}

	/**
	 * Returns a hash code for this <code>Double</code>. The result is
	 * the exclusive OR of the two halves of the primitive
	 * long integer bit representation. That is, the hashcode
	 * is the value of the expression:
	 * <blockquote><pre>
	 * (int)(_value()^((unsigned)_value()&gt;&gt;32))
	 * </pre></blockquote>
	 *
	 * @return a hash code value for this object.
	 */
	int32_t hashCode() const override {
		uint64_t bits = doubleToLongBits(_value);
		return (int32_t)(bits ^ (bits >> 32));
	}

	Double& operator =(const Double& other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	using Object::equals;

	bool equals(const Double& other) const {
		return (doubleToLongBits(_value) == doubleToLongBits(other._value));
	}

	virtual bool equals(Object const& other) const override {
		if (instanceof<Double>(other))
			return _value == Class::constCast<Double>(&other)->_value;
		return false;
	}

	bool equals(SPtr<Double> const& other) const {
		if (!other)
			return false;
		return (doubleToLongBits(_value) == doubleToLongBits(other->_value));
	}

	bool operator ==(const Double& other) const {
		return equals(other);
	}

	static bool isInfinite(double v) {
		return std::isinf(v);
	}

	bool isInfinite() {
		return isInfinite(_value);
	}

	int64_t longValue() const override;
	virtual double doubleValue() const override;

	template <class S>
	static double parseDouble(S const& s) {
		const char *str = strData(CPtr(s));
		if (!str)
			throw NumberFormatException(_HERE_, "null");

		size_t sLen = strLen(CPtr(s));
		double val;
		fast_float::from_chars_result res = fast_float::from_chars(str, str + sLen, val);

		if (res.ec != std::errc())
			throw NumberFormatException(_HERE_, "Double parse error");
		return  val;
	}

	static UPtr<String> toString(double d) {
		return newU<String>(fmt::format("{}", d));
	}

	virtual UPtr<String> toString() const override {
		return toString(_value);
	}
};

/** Boolean value */
class Boolean : virtual public Object {
public:
	TYPE_INFO(Boolean, CLASS(Boolean), INHERITS(Object));
private:
	bool _value;
public:
	Boolean(bool value)
	:_value(value) {}

	virtual ~Boolean() override;

	virtual int32_t hashCode() const override {
		return _value ? 1231 : 1237;
	}

	using Object::equals;

	bool equals(const Boolean& other) const {
		return (_value == other._value);
	}

	bool equals(SPtr<Boolean> const& other) const {
		if (!other)
			return false;
		return (_value == other->_value);
	}

	Boolean& operator =(const Boolean & other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	bool operator ==(const Boolean& other) const {
		return equals(other);
	}

	bool booleanValue() const {
		return _value;
	}

	template <class S>
	static bool parseBoolean(S const& str) {
		return String::equalsIgnoreCase(str, "true"_SV);
	}

	virtual UPtr<String> toString() const override {
		return (_value ? "true"_UPTR : "false"_UPTR);
	}
};

} // namespace

namespace std {
	// for using ULong as key in unordered_map
	template<> struct hash<slib::ULong> {
		std::size_t operator()(const slib::ULong& ul) const {
			return (size_t)ul.hashCode();
		}
	};
}

#endif // H_SLIB_NUMERIC_H

