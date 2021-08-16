/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONINPUTSTREAM_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONINPUTSTREAM_H

#include "slib/text/StringCharacterIterator.h"
#include "slib/lang/StringBuilder.h"
#include "slib/util/expr/Exceptions.h"
#include "slib/util/expr/Value.h"
#include "slib/util/expr/Lambda.h"

#include <cctype>

namespace slib {
namespace expr {

class ExpressionInputStream {
public:
	enum class ReservedWord { NONE, TRUE, FALSE, NIL };
private:
	StringCharacterIterator _iter;
	char _currentChar;
private:
	static bool isSpecialNameChar(char ch) {
		return (ch == '$') || (ch == '#') || (ch == '@');
	}

	UPtr<String> readReal();
public:
	ExpressionInputStream(SPtr<BasicString> const& s)
	:_iter(s) {
		_currentChar = _iter.first();
	}

	void skipBlanks();

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

	void setIndex(ssize_t pos) {
		_iter.setIndex(pos);
		_currentChar = _iter.current();
	}

	static bool isIdentifierStart(char ch) {
		return (ch == '_') || isSpecialNameChar(ch) || std::isalpha(ch);
	}

	/**
	 * Reads a symbol name
	 * @return name
	 * @throws SyntaxErrorException
	 */
	UPtr<String> readName(ReservedWord &reservedWord);

	/** @throws SyntaxErrorException */
	UPtr<Value> readString();

	/** @throws SyntaxErrorException */
	SPtr<Lambda> readArgLambda(char argSep, char argEnd);

	/** @throws EvaluationException */
	UPtr<Value> readNumber();

	ValueDomain readDomain();
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONINPUTSTREAM_H
