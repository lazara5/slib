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
						state = BSSTATE::SCAN;
						break;
					default:
						break;
				}
				break;
		}

		_currentChar = _iter.next();
	}
}

UPtr<String> ExpressionInputStream::readName(ReservedWord &reservedWord) {
	enum class NSTATE {
		SCAN, READ,
		FALSE_F, FALSE_A, FALSE_L, FALSE_S, FALSE_E,
		TRUE_T, TRUE_R, TRUE_U, TRUE_E,
		NIL_N, NIL_I, NIL_L
	};

	reservedWord = ReservedWord::NONE;

	skipBlanks();

	NSTATE state = NSTATE::SCAN;
	StringBuilder str;

	char ch = peek();
	while (true) {
		switch (state) {
			case NSTATE::SCAN:
				switch (ch) {
					case '_':
					case '$':
					case '#':
					case '@':
					case '0' ... '9':
					case 'a' ... 'e':
					case 'g' ... 'm':
					case 'o' ... 's':
					case 'u' ... 'z':
					case 'A' ... 'Z':
						state = NSTATE::READ;
						str.add(readChar());
						break;
					case 'f':
						state = NSTATE::FALSE_F;
						str.add(readChar());
						break;
					case 'n':
						state = NSTATE::NIL_N;
						str.add(readChar());
						break;
					case 't':
						state = NSTATE::TRUE_T;
						str.add(readChar());
						break;
					default:
						throw SyntaxErrorException(_HERE_, fmt::format("Identifier start expected, got '{}'", ch).c_str());

				}
				break;
			case NSTATE::READ:
				switch (ch) {
					case '_':
					case '0' ... '9':
					case 'a' ... 'z':
					case 'A' ... 'Z':
						str.add(readChar());
						break;
					default:
						return str.toString();
				}
				break;
			case NSTATE::FALSE_F:
				switch (ch) {
					case 'a':
						state = NSTATE::FALSE_A;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::FALSE_A:
				switch (ch) {
					case 'l':
						state = NSTATE::FALSE_L;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::FALSE_L:
				switch (ch) {
					case 's':
						state = NSTATE::FALSE_S;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::FALSE_S:
				switch (ch) {
					case 'e':
						state = NSTATE::FALSE_E;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::FALSE_E:
				switch (ch) {
					case '_':
					case '0' ... '9':
					case 'a' ... 'z':
					case 'A' ... 'Z':
						str.add(readChar());
						state = NSTATE::READ;
						break;
					default:
						reservedWord = ReservedWord::FALSE;
						return str.toString();
				}
				break;
			case NSTATE::TRUE_T:
				switch (ch) {
					case 'r':
						state = NSTATE::TRUE_R;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::TRUE_R:
				switch (ch) {
					case 'u':
						state = NSTATE::TRUE_U;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::TRUE_U:
				switch (ch) {
					case 'e':
						state = NSTATE::TRUE_E;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::TRUE_E:
				switch (ch) {
					case '_':
					case '0' ... '9':
					case 'a' ... 'z':
					case 'A' ... 'Z':
						str.add(readChar());
						state = NSTATE::READ;
						break;
					default:
						reservedWord = ReservedWord::TRUE;
						return str.toString();
				}
				break;
			case NSTATE::NIL_N:
				switch (ch) {
					case 'i':
						state = NSTATE::NIL_I;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::NIL_I:
				switch (ch) {
					case 'l':
						state = NSTATE::NIL_L;
						str.add(readChar());
						break;
					default:
						state = NSTATE::READ;
						break;
				}
				break;
			case NSTATE::NIL_L:
				switch (ch) {
					case '_':
					case '0' ... '9':
					case 'a' ... 'z':
					case 'A' ... 'Z':
						str.add(readChar());
						state = NSTATE::READ;
						break;
					default:
						reservedWord = ReservedWord::NIL;
						return str.toString();
				}
				break;
		}
		ch = peek();
	}
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
		ch = peek();
		if (ch == ':') {
			domain = ValueDomain::GLOBAL;
			readChar();
		}
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
