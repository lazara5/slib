/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/Numeric.h"
#include "slib/util/StringUtils.h"

#include <vector>

namespace slib {

Number::~Number() {}

static UPtr<String> getMantissa(UPtr<String> const& str, size_t stopPos) {
	char firstChar = str->charAt(0);
	bool hasSign = (firstChar == '-' || firstChar == '+');

	return hasSign ? str->substring(1, stopPos) : str->substring(0, stopPos);
}

static UPtr<String> getMantissa(UPtr<String> const& str) {
	return getMantissa(str, str->length());
}

static bool isAllZeros(UPtr<String> const& str) {
	if (!str)
		return true;

	for (ssize_t i = (ssize_t)str->length() - 1; i >= 0; i--) {
		if (str->charAt((size_t)i) != '0')
			return false;
	}
	return (str->length() > 0);
}

static bool isDigits(UPtr<String> const& str) {
	if (StringUtils::isEmpty(CPtr(str)))
		return false;
	for (size_t i = 0; i < str->length(); i++) {
		if (!std::isdigit(str->charAt(i)))
			return false;
	}
	return true;
}

/** adapted from Apache Commons Lang3 Numberutils, under Apache License-2.0 */
UPtr<Number> Number::createNumber(UPtr<String> const& str) {
	if (!str)
		return nullptr;
	if (StringUtils::isBlank(str))
		throw NumberFormatException(_HERE_, "Cannot convert blank string to number");

	static const std::vector<std::string> hexPrefixes {"0x", "0X", "-0x", "-0X", "#", "-#"};

	size_t prefixLen = 0;
	for (std::string const& prefix : hexPrefixes) {
		if (str->startsWith(prefix)) {
			prefixLen += prefix.length();
			break;
		}
	}
	if (prefixLen > 0) {
		// Hex
		char firstNzDigit = 0;
		for (size_t i = prefixLen; i < str->length(); i++) {
			firstNzDigit = str->charAt(i);
			if (firstNzDigit == '0')
				prefixLen++;
			else
				break;
		}

		size_t hexDigits = str->length() - prefixLen;
		if (hexDigits > 16 || ((hexDigits == 16) && (firstNzDigit > '7'))) {
			// cannot fit into a Long, we don't have a BigInteger yet, abort
			throw NumberFormatException("Overflow");
		}
		if (hexDigits > 8 || (hexDigits == 8 && firstNzDigit > '7'))
			return newU<Long>(Long::decode(CPtr(str)));
		return newU<Integer>(Integer::decode(CPtr(str)));
	}

	char lastChar = str->charAt(str->length() - 1);

	UPtr<String> mantissa;
	UPtr<String> decimal;
	UPtr<String> exponent;

	ptrdiff_t decPos = str->indexOf('.');
	ptrdiff_t expPos = str->indexOf('e') + str->indexOf('E') + 1; // assumes both not present
	// if both e and E are present, this is caught by the checks on expPos and the parsing which
	// will detect if e or E appear in a number due to using the wrong offset

	size_t numDecimals = 0;
	if (decPos > -1) {
		if (expPos > -1) {
			if ((expPos < decPos) || ((size_t)expPos > str->length()))
				throw NumberFormatException(_HERE_, "Invalid number");
			decimal = str->substring((size_t)decPos + 1, (size_t)expPos);
		} else
			decimal = str->substring((size_t)decPos + 1);
		mantissa = getMantissa(str, decPos);
		numDecimals = decimal->length();
	} else {
		if (expPos > -1) {
			if ((size_t)expPos > str->length())
				throw NumberFormatException(_HERE_, "Invalid number");
			mantissa = getMantissa(str, expPos);
		} else
			mantissa = getMantissa(str);
	}

	if ((!std::isdigit(lastChar)) && (lastChar != '.')) {
		if ((expPos > -1) && (expPos < (ptrdiff_t)str->length() - 1))
			exponent = str->substring((size_t)expPos + 1, str->length() - 1);

		UPtr<String> numeric = str->substring(0, str->length() - 1);
		bool allZeros = isAllZeros(mantissa) && isAllZeros(exponent);
		switch (lastChar) {
			case 'l':
			case 'L':
				if ((!decimal) && (!exponent) &&
					(((numeric->charAt(0) == '-') && (isDigits(numeric->substring(1)))) || isDigits(numeric))) {
					return newU<Long>(Long::decode(CPtr(str)));
					//we don't have a BigInteger yet, if we crash so be it
				}
				throw NumberFormatException(_HERE_, "Invalid number");
			case 'f':
			case 'F':
				// we don't have a Float yet, fallback to Double
			case 'd':
			case 'D':
				{
					double d = Double::parseDouble(numeric);
					if (!(Double::isInfinite(d) || (d == 0.0 && !allZeros)))
						return newU<Double>(d);
				}
				//we don't have a BigInteger yet, if we crash so be it
				/* fall through */
			default:
				throw NumberFormatException(_HERE_, "Invalid number");
		}
	}

	if ((expPos > -1) && (expPos < (ptrdiff_t)str->length() - 1))
		exponent = str->substring((size_t)expPos + 1, str->length());
	if ((!decimal) && (!exponent)) {
		try {
			return newU<Integer>(Integer::decode(CPtr(str)));
		} catch (NumberFormatException const&) {
			// ignore
		}
		return newU<Long>(Long::decode(CPtr(str)));
		//we don't have a BigInteger yet, if we crash so be it
	}

	bool allZeros = isAllZeros(mantissa) && isAllZeros(exponent);
	if (numDecimals <= 16) {
		double d = Double::parseDouble(str);
		if (!(Double::isInfinite(d) || (d == 0.0 && !allZeros)))
			return newU<Double>(d);
		throw NumberFormatException(_HERE_, "Invalid number");
	}

	throw NumberFormatException(_HERE_, "Overflow");
}

/** adapted from Apache Commons Lang3 Numberutils, under Apache License-2.0 */
UPtr<Number> Number::createLongOrDouble(UPtr<String> const& str) {
	if (!str)
		return nullptr;
	if (StringUtils::isBlank(str))
		throw NumberFormatException(_HERE_, "Cannot convert blank string to number");

	static const std::vector<std::string> hexPrefixes {"0x", "0X", "-0x", "-0X", "#", "-#"};

	size_t prefixLen = 0;
	for (std::string const& prefix : hexPrefixes) {
		if (str->startsWith(prefix)) {
			prefixLen += prefix.length();
			break;
		}
	}
	if (prefixLen > 0) {
		// Hex
		char firstNzDigit = 0;
		for (size_t i = prefixLen; i < str->length(); i++) {
			firstNzDigit = str->charAt(i);
			if (firstNzDigit == '0')
				prefixLen++;
			else
				break;
		}

		size_t hexDigits = str->length() - prefixLen;
		if (hexDigits > 16 || ((hexDigits == 16) && (firstNzDigit > '7'))) {
			// cannot fit into a Long, we don't have a BigInteger yet, abort
			throw NumberFormatException("Overflow");
		}
		return newU<Long>(Long::decode(CPtr(str)));
	}

	char lastChar = str->charAt(str->length() - 1);

	UPtr<String> mantissa;
	UPtr<String> decimal;
	UPtr<String> exponent;

	ptrdiff_t decPos = str->indexOf('.');
	ptrdiff_t expPos = str->indexOf('e') + str->indexOf('E') + 1; // assumes both not present
	// if both e and E are present, this is caught by the checks on expPos and the parsing which
	// will detect if e or E appear in a number due to using the wrong offset

	size_t numDecimals = 0;
	if (decPos > -1) {
		if (expPos > -1) {
			if ((expPos < decPos) || ((size_t)expPos > str->length()))
				throw NumberFormatException(_HERE_, "Invalid number");
			decimal = str->substring((size_t)decPos + 1, (size_t)expPos);
		} else
			decimal = str->substring((size_t)decPos + 1);
		mantissa = getMantissa(str, decPos);
		numDecimals = decimal->length();
	} else {
		if (expPos > -1) {
			if ((size_t)expPos > str->length())
				throw NumberFormatException(_HERE_, "Invalid number");
			mantissa = getMantissa(str, expPos);
		} else
			mantissa = getMantissa(str);
	}

	if ((!std::isdigit(lastChar)) && (lastChar != '.')) {
		if ((expPos > -1) && (expPos < (ptrdiff_t)str->length() - 1))
			exponent = str->substring((size_t)expPos + 1, str->length() - 1);

		UPtr<String> numeric = str->substring(0, str->length() - 1);
		bool allZeros = isAllZeros(mantissa) && isAllZeros(exponent);
		switch (lastChar) {
			case 'l':
			case 'L':
				if ((!decimal) && (!exponent) &&
					(((numeric->charAt(0) == '-') && (isDigits(numeric->substring(1)))) || isDigits(numeric))) {
					return newU<Long>(Long::decode(CPtr(str)));
				}
				throw NumberFormatException(_HERE_, "Invalid number");
			case 'f':
			case 'F':
			case 'd':
			case 'D':
				{
					double d = Double::parseDouble(numeric);
					if (!(Double::isInfinite(d) || (d == 0.0 && !allZeros)))
						return newU<Double>(d);
				}
				/* fall through */
			default:
				throw NumberFormatException(_HERE_, "Invalid number");
		}
	}

	if ((expPos > -1) && (expPos < (ptrdiff_t)str->length() - 1))
		exponent = str->substring((size_t)expPos + 1, str->length());
	if ((!decimal) && (!exponent)) {
		return newU<Long>(Long::decode(CPtr(str)));
	}

	bool allZeros = isAllZeros(mantissa) && isAllZeros(exponent);
	if (numDecimals <= 16) {
		double d = Double::parseDouble(str);
		if (!(Double::isInfinite(d) || (d == 0.0 && !allZeros)))
			return newU<Double>(d);
		throw NumberFormatException(_HERE_, "Invalid number");
	}

	throw NumberFormatException(_HERE_, "Overflow");
}

bool Number::isMathematicalInteger(double val) {
	return std::trunc(val) == val;
}

int64_t Integer::longValue() const {
	return (int64_t)_value;
}

double Integer::doubleValue() const {
	return (double)_value;
}

int64_t UInt::longValue() const {
	return (int64_t)_value;
}

double UInt::doubleValue() const {
	return (double)_value;
}

int64_t Short::longValue() const {
	return (int64_t)_value;
}

double Short::doubleValue() const {
	return (double)_value;
}

int64_t Long::longValue() const {
	return _value;
}

double Long::doubleValue() const {
	return (double)_value;
}

double ULong::doubleValue() const {
	return (double)_value;
}

int64_t Double::longValue() const {
	return (int64_t)_value;
}

double Double::doubleValue() const {
	return _value;
}

Boolean::~Boolean() {}

} // namespace slib
