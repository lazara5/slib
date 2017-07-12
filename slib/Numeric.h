/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_NUMERIC_H
#define H_SLIB_NUMERIC_H

#include "slib/exception/NumericExceptions.h"

#include "fmt/format.h"

#include <string>
#include <sstream>

#include <math.h>
#include <limits.h>

#define slib_min(a,b)  (((a) < (b)) ? (a) : (b))

namespace slib {

/** 32-bit signed integer */
class Integer {
private:
	int32_t _value;
	bool _isNull;
protected:
	Integer(int32_t value, bool makeNull)
	:_value(value)
	,_isNull(makeNull) {}
public:
	static const int MIN_VALUE = INT32_MIN;
	static const int MAX_VALUE = INT32_MAX;

	Integer()
	:_value(0)
	,_isNull(false) {
	}

	Integer(int32_t value)
	:_value(value)
	,_isNull(false) {
	}

	Integer(const Integer& other)
	: _value(other._value)
	, _isNull(other._isNull) {
	}

	int hashCode() const {
		return _value;
	}

	Integer& operator =(const Integer& other) {
		_value = other._value;
		_isNull = other._isNull;
		return *this;	// This will allow assignments to be chained
	}

	bool equals(const Integer& other) const {
		if (_isNull)
			return (other._isNull);
		if (other._isNull)
			return _isNull;
		return (_value == other._value);
	}

	bool operator ==(const Integer& other) const {
		return equals(other);
	}

	int32_t intValue() const {
		return _value;
	}

	static Integer getNull() { return Integer(0, true); }

	bool isNull() const { return _isNull; }

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

	static std::string toString(int32_t i) {
		std::stringstream stream;
		stream << i;
		return stream.str();
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
	static std::string toHexString(int32_t i) {
		return fmt::format("{:x}", i);
	}
};

/** 64-bit signed integer */
class Long {
private:
	int64_t _value;
	bool _isNull;
protected:
	Long(int64_t value, bool makeNull)
	:_value(value)
	, _isNull(makeNull) {
	}
public:
	static const int64_t MIN_VALUE = INT64_MIN;
	static const int64_t MAX_VALUE = INT64_MAX;

	Long()
	:_value(0)
	,_isNull(false) {
	}

	Long(int64_t value)
	:_value(value)
	,_isNull(false) {
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
	int hashCode() const {
		return (int)(_value ^ (((uint64_t)_value) >> 32));
	} 

	Long& operator =(const Long& other) {
		_value = other._value;
		_isNull = other._isNull;
		return *this;	// This will allow assignments to be chained
	}

	bool equals(const Long& other) const {
		if (_isNull)
			return (other._isNull);
		if (other._isNull)
			return _isNull;
		return (_value == other._value);
	}

	bool operator==(const Long& other) const {
		return equals(other);
	}

	int64_t longValue() const {
		return _value;
	}

	static Long getNull() { return Long(0, true); }

	bool isNull() const { return _isNull; }

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

	static std::string toString(int64_t i) {
		std::stringstream stream;
		stream << i;
		return stream.str();
	}
};

/** 64-bit unsigned integer */
class ULong {
private:
	uint64_t _value;
	bool _isNull;
protected:
	ULong(uint64_t value, bool makeNull)
	:_value(value)
	, _isNull(makeNull) {
	}
public:
	static const uint64_t MAX_VALUE = UINT64_MAX;

	ULong(uint64_t value)
	:_value(value)
	,_isNull(false) {
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
	int hashCode() const {
		return (int)(_value ^ (_value >> 32));
	}

	ULong& operator =(const ULong& other) {
		_value = other._value;
		_isNull = other._isNull;
		return *this;	// This will allow assignments to be chained
	}

	bool equals(const ULong& other) const {
		if (_isNull) 
			return (other._isNull);
		if (other._isNull)
			return _isNull;
		return (_value == other._value);
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

	static ULong getNull() { return ULong(0, true); }

	bool isNull() const { return _isNull; }

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
class Double {
private:
	double _value;
	bool _isNull;
protected:
	Double(double value, bool makeNull)
	:_value(value)
	, _isNull(makeNull) {
	}

	static uint64_t doubleToLongBits(double value) {
		if (isnan(value))
			return 0x7ff8000000000000LL;
		else {
			uint64_t res = 0;
			int len = slib_min(sizeof(double), sizeof(uint64_t));
			memcpy(&res, &value, len);
			return res;
		}
	}
public:
	Double(double value)
	:_value(value)
	,_isNull(false) {
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
	int hashCode() const {
		uint64_t bits = doubleToLongBits(_value);
		return (int)(bits ^ (bits >> 32));
	}

	Double& operator=(const Double& other) {
		_value = other._value;
		_isNull = other._isNull;
		return *this;	// This will allow assignments to be chained
	}

	bool equals(const Double& other) const {
		if (_isNull) 
			return (other._isNull);
		if (other._isNull)
			return _isNull;
		return (doubleToLongBits(_value) == doubleToLongBits(other._value));
	}

	bool operator==(const Double& other) const {
		return equals(other);
	}

	static Double getNull() { return Double(0, true); }

	bool isNull() const { return _isNull; }

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
class Boolean {
private:
	bool _value;
	bool _isNull;
protected:
	Boolean(bool value, bool makeNull)
	:_value(value)
	, _isNull(makeNull) {
	}
public:
	Boolean(bool value)
	:_value(value)
	,_isNull(false) {
	}
	
	bool equals(const Boolean& other) const {
		if (_isNull) 
			return (other._isNull);
		if (other._isNull)
			return _isNull;
		return (_value == other._value);
	}

	Boolean& operator =(const Boolean & other) {
		_value = other._value;
		_isNull = other._isNull;
		return *this;	// This will allow assignments to be chained
	}

	bool operator==(const Boolean& other) const {
		return equals(other);
	}

	static Boolean getNull() { return Boolean(false, true); }

	bool isNull() const { return _isNull; }

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

