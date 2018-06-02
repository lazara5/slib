/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/util/expr/ExpressionInputStream.h"
#include "slib/lang/Numeric.h"

namespace slib {
namespace expr {

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

enum class SSMODE { SCAN, ESCAPE, ESCAPE2 };

/** @throws SyntaxErrorException */
SPtr<Value> ExpressionInputStream::readString() {
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
	return std::make_shared<Value>(str.toString());
}

std::shared_ptr<Value> ExpressionInputStream::readNumber() {
	std::unique_ptr<String> str = readReal();
	try {
		return std::make_shared<Value>(Number::createNumber(str));
	} catch (NumberFormatException const& e) {
		throw EvaluationException(_HERE_, "Error parsing numeric value", e);
	}
}

std::unique_ptr<String> ExpressionInputStream::readReal() {
	StringBuilder s;
	skipBlanks();
	char ch = peek();
	if (ch == CharacterIterator::DONE)
		return std::make_unique<String>("");
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

enum class ASMODE { SCAN, STRING, ESCAPE };

std::shared_ptr<Expression> ExpressionInputStream::readArg() {
	char delimiter = '\1';
	int argDepth = 0;
	StringBuilder str;
	bool complete = false;
	ASMODE mode = ASMODE::SCAN;
	do {
		char ch = peek();
		switch (mode) {
			case ASMODE::SCAN:
				{
					if (((ch == ',') || (ch == ')')) && (argDepth == 0))
						complete = true;
					else {
						readChar();
						if (ch == CharacterIterator::DONE)
							throw SyntaxErrorException(_HERE_, "Unexpected EOS reading argument");
						str.add(ch);
						if (ch == '"' || ch == '\'') {
							delimiter = ch;
							mode = ASMODE::STRING;
						} else if (ch == '(')
							argDepth++;
						else if (ch == ')')
							argDepth--;
					}
				}
				break;
			case ASMODE::STRING:
				{
					readChar();
					if (ch == CharacterIterator::DONE)
						throw SyntaxErrorException(_HERE_, "Unexpected EOS reading argument string");
					str.add(ch);
					if (ch == '\\' || ch == '`') {
						mode = ASMODE::ESCAPE;
					} else if (ch == delimiter) {
						delimiter = '\1';
						mode = ASMODE::SCAN;
					}
				}
				break;
			case ASMODE::ESCAPE:
				{
					readChar();
					if (ch == CharacterIterator::DONE)
						throw SyntaxErrorException(_HERE_, "Unexpected EOS reading string escape sequence");
					str.add(ch);
					mode = ASMODE::STRING;
				}
				break;
		}
	} while (!complete);
	return std::make_shared<Expression>(str.toString());
}

} // namespace expr
} // namespace slib
