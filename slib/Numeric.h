/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_NUMERIC_H
#define H_SLIB_NUMERIC_H

#include "slib/Object.h"
#include "slib/exception/NumericExceptions.h"
#include "slib/String.h"

#include "fmt/format.h"

#include <string>
#include <sstream>

#include <math.h>
#include <limits.h>

#define slib_min(a,b)  (((a) < (b)) ? (a) : (b))

namespace slib {

class Number : virtual public Object {
public:
	virtual ~Number();

	virtual double doubleValue() const = 0;
};

/** 32-bit signed integer */
class Integer : public Number {
private:
	int32_t _value;
public:
	static const int32_t MIN_VALUE = INT32_MIN;
	static const int32_t MAX_VALUE = INT32_MAX;

	Integer()
	:_value(0) {}

	Integer(int32_t value)
	:_value(value) {}

	Integer(const Integer& other)
	: _value(other._value) {}

	virtual Class const& getClass() const override {
		return integerClass;
	}

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

	bool equals(std::shared_ptr<Integer> const& other) const {
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

	virtual double doubleValue() const override;

	static int32_t parseInt(const char *str, int radix) {
		if (str == nullptr)
			throw NumberFormatException(_HERE_, "null");

		char *end;
		errno = 0;
		const long res = strtol(str, &end, radix);

		if (end == str)
			throw NumberFormatException(_HERE_, "Not a decimal number");
		else if (0 != *end)
			throw NumberFormatException(_HERE_, "Extra characters at end of input");
		else if ((LONG_MIN == res || LONG_MAX == res) && ERANGE == errno) 
			throw NumericOverflowException(_HERE_, "Out of range");
		else if (res > INT_MAX) 
			throw NumericOverflowException(_HERE_, "Out of range");
		else if (res < INT_MIN)
			throw NumericOverflowException(_HERE_, "Out of range");

		return (int32_t)res;
	}

	static int32_t parseInt(const char *str) {
		return parseInt(str, 10);
	}

	static int32_t parseInt(const std::string& s) {
		return parseInt(s.c_str(), 10);
	}

	static int32_t parseInt(const std::string& s, int radix) {
		return parseInt(s.c_str(), radix);
	}

	static String toString(int32_t i) {
		std::stringstream stream;
		stream << i;
		return String(stream.str());
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

	virtual String toString() const override {
		return toString(_value);
	}
};

/** 32-bit unsigned integer */
class UInt : public Number {
private:
	uint32_t _value;
public:
	static const uint32_t MIN_VALUE = 0;
	static const uint32_t MAX_VALUE = UINT32_MAX;

	UInt()
	:_value(0) {}

	UInt(uint32_t value)
	:_value(value) {}

	UInt(const UInt& other)
	: _value(other._value) {}

	virtual Class const& getClass() const override {
		return uIntClass;
	}

	int32_t hashCode() const override {
		return (int)_value;
	}

	UInt& operator =(const UInt& other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	bool equals(const UInt& other) const {
		return (_value == other._value);
	}

	bool equals(std::shared_ptr<UInt> const& other) const {
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

	virtual double doubleValue() const override;

	static uint32_t parseUInt(const char *str, int radix) {
		if (str == nullptr)
			throw NumberFormatException(_HERE_, "null");

		char *end;
		errno = 0;
		const unsigned long res = strtoul(str, &end, radix);

		if (end == str)
			throw NumberFormatException(_HERE_, "Not a decimal number");
		else if (0 != *end)
			throw NumberFormatException(_HERE_, "Extra characters at end of input");
		else if (ULONG_MAX == res && ERANGE == errno)
			throw NumericOverflowException(_HERE_, "Out of range");

		return (uint32_t)res;
	}

	static uint32_t parseUInt(const char *str) {
		return parseUInt(str, 10);
	}

	static uint32_t parseUInt(const std::string& s) {
		return parseUInt(s.c_str(), 10);
	}

	static uint32_t parseUInt(const std::string& s, int radix) {
		return parseUInt(s.c_str(), radix);
	}

	static String toString(uint32_t i) {
		std::stringstream stream;
		stream << i;
		return String(stream.str());
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

	virtual String toString() const override {
		return toString(_value);
	}
};

/** 64-bit signed integer */
class Long : public Number {
private:
	int64_t _value;
public:
	static const int64_t MIN_VALUE = INT64_MIN;
	static const int64_t MAX_VALUE = INT64_MAX;

	Long()
	:_value(0) {}

	Long(int64_t value)
	:_value(value) {}

	virtual Class const& getClass() const override {
		return longClass;
	}

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

	bool equals(const Long& other) const {
		return (_value == other._value);
	}

	bool equals(std::shared_ptr<Long> const& other) const {
		if (!other)
			return false;
		return (_value == other->_value);
	}

	bool operator==(const Long& other) const {
		return equals(other);
	}

	int64_t longValue() const {
		return _value;
	}

	virtual double doubleValue() const override;

	static int64_t parseLong(const char *str) {
		if (str == NULL)
			throw NumberFormatException("null");

		char *end;
		errno = 0;
		const int64_t res = strtoll(str, &end, 10);

		if (end == str)
			throw NumberFormatException(_HERE_, "Not a decimal number");
		else if (0 != *end)
			throw NumberFormatException(_HERE_, "Extra characters at end of input");
		else if ((LLONG_MIN == res || LLONG_MAX == res) && ERANGE == errno) 
			throw NumericOverflowException(_HERE_, "Out of range");

		return res;
	}

	static int64_t parseLong(const std::string& str) {
		return parseLong(str.c_str());
	}

	static String toString(int64_t i) {
		std::stringstream stream;
		stream << i;
		return String(stream.str());
	}

	virtual String toString() const override {
		return toString(_value);
	}
};

/** 64-bit unsigned integer */
class ULong : public Number {
private:
	uint64_t _value;
public:
	static const uint64_t MAX_VALUE = UINT64_MAX;

	ULong(uint64_t value)
	:_value(value) {}

	virtual Class const& getClass() const override {
		return uLongClass;
	}

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

	bool equals(const ULong& other) const {
		return (_value == other._value);
	}

	bool equals(std::shared_ptr<ULong> const& other) const {
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

	static uint64_t parseULong(const char *str) {
		if (str == nullptr)
			throw NumberFormatException(_HERE_, "null");

		char *end;
		errno = 0;
		const uint64_t res = strtoull(str, &end, 10);

		if (end == str)
			throw NumberFormatException(_HERE_, "Not a decimal number");
		else if (0 != *end)
			throw NumberFormatException(_HERE_, "Extra characters at end of input");
		else if ((ULONG_MAX == res) && ERANGE == errno) 
			throw NumericOverflowException(_HERE_, "Out of range");

		return res;
	}

	static uint64_t parseULong(const char *str, uint64_t defaultValue) {
		try {
			return parseULong(str);
		} catch (NumberFormatException const&) {
			return defaultValue;
		}
	}

	static uint64_t parseULong(const std::string& str) {
		return parseULong(str.c_str());
	}

	static uint64_t parseULong(const std::string& str, uint64_t defaultValue) {
		return parseULong(str.c_str(), defaultValue);
	}

	struct hash {								///< hash based on mix from Murmur3
		size_t operator()(const uint64_t& key) const {
			uint64_t k = key ^ (key >> 33);
			k *= 0xff51afd7ed558ccd;
			k ^= k >> 33;
			k *= 0xc4ceb9fe1a85ec53;
			k ^= k >> 33;
			return (size_t) k;
		}
	};
};

/** Double value */
class Double : public Number {
private:
	double _value;
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
	Double(double value)
	:_value(value) {}

	virtual Class const& getClass() const override {
		return doubleClass;
	}

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

	Double& operator=(const Double& other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	bool equals(const Double& other) const {
		return (doubleToLongBits(_value) == doubleToLongBits(other._value));
	}

	bool equals(std::shared_ptr<Double> const& other) const {
		if (!other)
			return false;
		return (doubleToLongBits(_value) == doubleToLongBits(other->_value));
	}

	bool operator==(const Double& other) const {
		return equals(other);
	}

	virtual double doubleValue() const override;

	static double parseDouble(const char *str) {
		if (str == nullptr)
			throw NumberFormatException(_HERE_, "null");

		char *end;
		errno = 0;
		const double res = strtod(str, &end);

		if (end == str)
			throw NumberFormatException(_HERE_, "Not a decimal number");
		else if (0 != *end)
			throw NumberFormatException(_HERE_, "Extra characters at end of input");
		else if ((HUGE_VAL == res || -HUGE_VAL == res) && ERANGE == errno) 
			throw NumericOverflowException(_HERE_, "Out of range");

		return res;
	}

	static double parseDouble(const std::string& str) {
		return parseDouble(str.c_str());
	}
};

/** Boolean value */
class Boolean : virtual public Object {
private:
	bool _value;
public:
	Boolean(bool value)
	:_value(value) {}

	virtual int32_t hashCode() const override {
		return _value ? 1231 : 1237;
	}
	
	bool equals(const Boolean& other) const {
		return (_value == other._value);
	}

	bool equals(std::shared_ptr<Boolean> const& other) const {
		if (!other)
			return false;
		return (_value == other->_value);
	}

	Boolean& operator=(const Boolean & other) {
		_value = other._value;
		return *this;	// This will allow assignments to be chained
	}

	bool operator==(const Boolean& other) const {
		return equals(other);
	}

	static bool parseBoolean(const char *str) {
		if (str == nullptr)
			return false;
		return (!strcasecmp(str, "true"));
	}

	static bool parseBoolean(const std::string& str) {
		return parseBoolean(str.c_str());
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

