/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionInputStream.h"
#include "slib/lang/Numeric.h"

namespace slib {
namespace expr {

void ExpressionInputStream::skipBlanks() {
	enum class BSSTATE { SCAN, SLASH, MLC, MLCSTAR, SLC };

	BSSTATE state = BSSTATE::SCAN;

	ptrdiff_t beforeSlash = 0;

	while (true) {
		switch (state) {
			case BSSTATE::SCAN:
				switch (_currentChar) {
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;
					case '/':
						beforeSlash = _iter.getIndex();
						state = BSSTATE::SLASH;
						break;
					default:
						return;
				}
				break;
			case BSSTATE::SLASH:
				switch (_currentChar) {
					case '*':
						state = BSSTATE::MLC;
						break;
					case '/':
						state = BSSTATE::SLC;
						break;
					default:
						_iter.setIndex(beforeSlash);
						_currentChar = _iter.current();
						return;
				}
				break;
			case BSSTATE::MLC:
				switch (_currentChar) {
					case CharacterIterator::DONE:
						throw SyntaxErrorException(_HERE_, "Unexpected end of stream");
					case '*':
						state = BSSTATE::MLCSTAR;
						break;
					default:
						break;
				}
				break;
			case BSSTATE::MLCSTAR:
				switch (_currentChar) {
					case CharacterIterator::DONE:
						throw SyntaxErrorException(_HERE_, "Unexpected end of stream");
					case '/':
						state = BSSTATE::SCAN;
						break;
					default:
						state = BSSTATE::MLC;
						break;
				}
				break;
			case BSSTATE::SLC:
				switch (_currentChar) {
					case CharacterIterator::DONE:
						return;
					case '\n':
						// Single line comments do NOT swallow \n !
						return;
					default:
						break;
				}
				break;
		}

		_currentChar = _iter.next();
	}
}

UPtr<String> ExpressionInputStream::readName() {
	skipBlanks();
	char ch = peek();
	if (!isIdentifierStart(ch))
		throw SyntaxErrorException(_HERE_, fmt::format("Identifier start expected, got '{}'", ch).c_str());
	StringBuilder str;
	while ((ch != CharacterIterator::DONE) &&
		   ((std::isalnum(ch) || ch == '_' || isSpecialNameChar(ch)))) {
		str.add(readChar());
		ch = peek();
	}
	return str.toString();
}

UPtr<String> ExpressionInputStream::readDottedNameRemainder() {
	skipBlanks();
	char ch = peek();
	StringBuilder str;
	while ((ch != CharacterIterator::DONE) &&
		   ((std::isalnum(ch) || (ch == '_') || ch == '.'))) {
		str.add(readChar());
		ch = peek();
	}
	return str.toString();
}

/** @throws SyntaxErrorException */
UPtr<Value> ExpressionInputStream::readString() {
	enum class SSMODE { SCAN, ESCAPE, ESCAPE2 };

	// read ' or "
	char delimiter = readChar();
	StringBuilder str;
	bool complete = false;
	SSMODE mode = SSMODE::SCAN;
	do {
		char ch = readChar();
		switch (mode) {
			case SSMODE::SCAN:
				{
					if (ch == delimiter)
						complete = true;
					else if (ch == '\\')
						mode = SSMODE::ESCAPE;
					else if (ch == '`')
						mode = SSMODE::ESCAPE2;
					else if (ch == CharacterIterator::DONE)
						throw SyntaxErrorException(_HERE_, "Unexpected EOS reading string");
					else
						str.add(ch);
				}
				break;
			case SSMODE::ESCAPE:
				{
					switch (ch) {
						case '\'':
						case '\\':
						case '"':
							str.add(ch);
							mode = SSMODE::SCAN;
							break;
						case CharacterIterator::DONE:
							throw SyntaxErrorException(_HERE_, "Unexpected EOS reading string escape sequence");
						default:
							throw SyntaxErrorException(_HERE_, fmt::format("Unknown escape sequence: \\{}", ch).c_str());
					}
				}
				break;
			case SSMODE::ESCAPE2:
				{
					switch (ch) {
						case '`':
							str.add(ch);
							mode = SSMODE::SCAN;
							break;
						case '\\':
						case 'B':
							str.add('\\');
							mode = SSMODE::SCAN;
							break;
						case '"':
						case 'D':
							str.add('\"');
							mode = SSMODE::SCAN;
							break;
						case '\'':
						case 'Q':
							str.add('\'');
							mode = SSMODE::SCAN;
							break;
						case CharacterIterator::DONE:
							throw SyntaxErrorException(_HERE_, "Unexpected EOS reading string escape sequence");
						default:
							throw SyntaxErrorException(_HERE_, fmt::format("Unknown escape sequence: \\{}", ch).c_str());
					}
				}
				break;
		}
	} while (!complete);
	return Value::of(str.toString());
}

ValueDomain ExpressionInputStream::readDomain() {
	ValueDomain domain = ValueDomain::DEFAULT;

	char ch = peek();
	if (ch == ':') {
		domain = ValueDomain::LOCAL;
		readChar();
	}

	return domain;
}

UPtr<Value> ExpressionInputStream::readNumber() {
	UPtr<String> str = readReal();
	try {
		return Value::of(Number::createLongOrDouble(str));
	} catch (NumberFormatException const& e) {
		throw EvaluationException(_HERE_, "Error parsing numeric value", e);
	}
}

UPtr<String> ExpressionInputStream::readReal() {
	StringBuilder s;
	skipBlanks();
	char ch = peek();
	if (ch == CharacterIterator::DONE)
		return ""_UPTR;
	if ((ch == '-') || (ch == '+')) {
		s.add(readChar());
		skipBlanks();
		ch = peek();
	}
	bool hex = false;
	if (ch == '0') {
		s.add(readChar());
		ch = peek();
		if (ch == 'x') {
			hex = true;
			s.add(readChar());
		}
	}
	while ((ch >= '0' && ch <= '9') ||
		   (hex && ch >= 'a' && ch <= 'f') ||
		   (hex && ch >= 'A' && ch <= 'F')) {
		s.add(readChar());
		ch = peek();
	}
	if (ch == '.') {
		s.add(readChar());
		ch = peek();
		while (ch >= '0' && ch <= '9') {
			s.add(readChar());
			ch = peek();
		}
	}
	if (ch == 'E' || ch == 'e') {
		s.add(readChar());
		ch = peek();
		if (ch == '-' || ch == '+') {
			s.add(readChar());
			ch = peek();
		}
		while (ch >= 0 && ch <= '9') {
			s.add(readChar());
			ch = peek();
		}
	}
	return s.toString();
}

SPtr<Lambda> ExpressionInputStream::readArgLambda(char argSep, char argEnd) {
	enum class LSMODE { SCAN, STRING, ESCAPE };

	char delimiter = '\1';

	int argDepth = 0;
	char argClose = '\1';
	std::vector<char> argStack;
	argStack.push_back(argClose);

	StringBuilder str;
	bool complete = false;
	LSMODE mode = LSMODE::SCAN;

	do {
		char ch = peek();
		switch (mode) {
			case LSMODE::SCAN:
				{
					if (((ch == argSep) || (ch == argEnd)) && (argDepth == 0))
						complete = true;
					else {
						readChar();
						if (ch == CharacterIterator::DONE)
							throw SyntaxErrorException(_HERE_, "Unexpected EOS reading argument");
						str.add(ch);
						if (ch == '"' || ch == '\'') {
							delimiter = ch;
							mode = LSMODE::STRING;
						} else if (ch == '(') {
							argStack.push_back(argClose);
							argClose = ')';
							argDepth++;
						} else if (ch == '{') {
							argStack.push_back(argClose);
							argClose = '}';
							argDepth++;
						} else if (ch == '[') {
							argStack.push_back(argClose);
							argClose = ']';
							argDepth++;
						} else if (ch == argClose) {
							if (argStack.empty())
								throw SyntaxErrorException(_HERE_, fmt::format("Unbalanced closing bracket: '{}'", ch).c_str());
							argClose = argStack.back();
							argStack.pop_back();
							argDepth--;
						}
					}
				}
				break;
			case LSMODE::STRING:
				{
					readChar();
					if (ch == CharacterIterator::DONE)
						throw SyntaxErrorException(_HERE_, "Unexpected EOS reading argument string");
					str.add(ch);
					if (ch == '\\' || ch == '`') {
						mode = LSMODE::ESCAPE;
					} else if (ch == delimiter) {
						delimiter = '\1';
						mode = LSMODE::SCAN;
					}
				}
				break;
			case LSMODE::ESCAPE:
				{
					readChar();
					if (ch == CharacterIterator::DONE)
						throw SyntaxErrorException(_HERE_, "Unexpected EOS reading string escape sequence");
					str.add(ch);
					mode = LSMODE::STRING;
				}
				break;
		}
	} while (!complete);
	return newS<Lambda>(str.toString());
}

} // namespace expr
} // namespace slib
