/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONINPUTSTREAM_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONINPUTSTREAM_H

#include "slib/text/StringCharacterIterator.h"
#include "slib/StringBuilder.h"
#include "slib/util/expr/Exceptions.h"
#include "slib/util/expr/Value.h"

#include <cctype>

namespace slib {
namespace expr {

class ExpressionInputStream {
private:
	StringCharacterIterator _iter;
	char _currentChar;
private:
	static bool isSpecialNameChar(char ch) {
		return (ch == '$') || (ch == '#') || (ch == '?') || (ch == '@');
	}

	std::unique_ptr<String> readReal();
public:
	ExpressionInputStream(std::shared_ptr<String> const& s)
	:_iter(s) {
		_currentChar = _iter.first();
	}

	void skipBlanks() {
		while ((_currentChar != CharacterIterator::DONE) && (std::isspace(_currentChar)))
			_currentChar = _iter.next();
	}

	void reset() {
		_currentChar = _iter.first();
	}

	char peek() {
		return _currentChar;
	}

	char readChar() {
		char val = _currentChar;
		if (_currentChar != CharacterIterator::DONE)
			_currentChar = _iter.next();
		return val;
	}

	ssize_t getIndex() {
		return _iter.getIndex();
	}

	static bool isIdentifierStart(char ch) {
		return (ch == '_') || isSpecialNameChar(ch) || std::isalpha(ch);
	}

	/**
	 * Reads a symbol name
	 * @return name
	 * @throws SyntaxErrorException
	 */
	std::unique_ptr<String> readName();

	std::unique_ptr<String> readDottedNameRemainder();

	/** @throws SyntaxErrorException */
	std::shared_ptr<Value> readString();

	/** @throws EvaluationException */
	std::shared_ptr<Value> readNumber();
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONINPUTSTREAM_H
