/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "slib/util/expr/ExpressionFormatter.h"
#include "slib/lang/Character.h"
#include "slib/lang/Numeric.h"

#include "fmt/printf.h"

namespace slib {
namespace expr {

CharBuffer::~CharBuffer() {}

//--- FormatToken ----------------------------------------------------------------------
//--------------------------------------------------------------------------------------

bool FormatToken::setFlag(char c) {
	int32_t newFlag;
	switch (c) {
		case '-': {
			newFlag = FLAG_MINUS;
			break;
		}
		case '#': {
			newFlag = FLAG_SHARP;
			break;
		}
		case '+': {
			newFlag = FLAG_ADD;
			break;
		}
		case ' ': {
			newFlag = FLAG_SPACE;
			break;
		}
		case '0': {
			newFlag = FLAG_ZERO;
			break;
		}
		case ',': {
			newFlag = FLAG_COMMA;
			break;
		}
		case '(': {
			newFlag = FLAG_PARENTHESIS;
			break;
		}
		default:
			return false;
	}

	if (0 != (_flags & newFlag))
		throw DuplicateFormatFlagsException(_HERE_, String::valueOf(c));

	_flags = (_flags | newFlag);
	_strFlags.add(c);
	return true;
}

//--- ParserStateMachine ---------------------------------------------------------------
//--------------------------------------------------------------------------------------

SPtr<FormatToken> ParserStateMachine::getNextFormatToken() {
	_token = std::make_shared<FormatToken>();
	_token->setFormatStringStartIndex(_format->position());

	// FINITE STATE MACHINE
	while (true) {
		if (ParserStateMachine::EXIT_STATE != _state) {
			// exit state does not need to get next char
			_currentChar = getNextFormatChar();
			if (EOS == _currentChar && ParserStateMachine::ENTRY_STATE != _state) {
				throw UnknownFormatConversionException(_HERE_, getFormatString());
			}
		}

		switch (_state) {
		// exit state
		case ParserStateMachine::EXIT_STATE: {
			process_EXIT_STATE();
			return _token;
		}
			// plain text state, not yet applied converter
		case ParserStateMachine::ENTRY_STATE: {
			process_ENTRY_STATE();
			break;
		}
			// begins converted string
		case ParserStateMachine::START_CONVERSION_STATE: {
			process_START_CONVERSION_STATE();
			break;
		}
		case ParserStateMachine::FLAGS_STATE: {
			process_FLAGS_STATE();
			break;
		}
		case ParserStateMachine::WIDTH_STATE: {
			process_WIDTH_STATE();
			break;
		}
		case ParserStateMachine::PRECISION_STATE: {
			process_PRECISION_STATE();
			break;
		}
		case ParserStateMachine::CONVERSION_TYPE_STATE: {
			process_CONVERSION_TYPE_STATE();
			break;
		}
		}
	}
}

void ParserStateMachine::process_ENTRY_STATE() {
	if (EOS == _currentChar)
		_state = ParserStateMachine::EXIT_STATE;
	else if ('%' == _currentChar) {
		// change to conversion type state
		_state = START_CONVERSION_STATE;
		_rawToken.add('%');
	}
	// else remains in ENTRY_STATE
}

void ParserStateMachine::process_START_CONVERSION_STATE() {
	if (Character::isDigit(_currentChar)) {
		ptrdiff_t position = _format->position() - 1;
		int32_t number = parseInt(_format);
		char nextChar = 0;
		if (_format->hasRemaining())
			nextChar = _format->get();
		if ('$' == nextChar) {
			// the digital sequence stands for the argument
			// index.
			int argIndex = number;
			// k$ stands for the argument whose index is k-1 except that
			// 0$ and 1$ both stands for the first element.
			if (argIndex > 0)
				_token->setArgIndex(argIndex - 1);
			else if (argIndex == FormatToken::UNSET) {
				throw MissingFormatArgumentException(_HERE_, *getFormatString());
			}
			_state = FLAGS_STATE;
		} else {
			// the digital zero stands for one format flag.
			if ('0' == _currentChar) {
				_state = FLAGS_STATE;
				_format->position(position);
			} else {
				// the digital sequence stands for the width.
				_state = WIDTH_STATE;
				// do not get the next char.
				_format->position(_format->position() - 1);
				_token->setWidth(number);
			}
		}
		_currentChar = nextChar;
	} else if ('<' == _currentChar) {
		_state = FLAGS_STATE;
		_token->setArgIndex(FormatToken::LAST_ARGUMENT_INDEX);
	} else {
		_state = FLAGS_STATE;
		// do not get the next char.
		_format->position(_format->position() - 1);
	}
}

void ParserStateMachine::process_FLAGS_STATE() {
	if (_token->setFlag(_currentChar)) {
		// remains in FLAGS_STATE
	} else if (Character::isDigit(_currentChar)) {
		_token->setWidth(parseInt(_format));
		_state = WIDTH_STATE;
	} else if ('.' == _currentChar) {
		_state = PRECISION_STATE;
		_rawToken.add('.');
	} else {
		_state = CONVERSION_TYPE_STATE;
		// do not get the next char.
		_format->position(_format->position() - 1);
	}
}

void ParserStateMachine::process_WIDTH_STATE() {
	if ('.' == _currentChar) {
		_state = PRECISION_STATE;
		_rawToken.add('.');
	} else {
		_state = CONVERSION_TYPE_STATE;
		// do not get the next char
		_format->position(_format->position() - 1);
	}
}

void ParserStateMachine::process_PRECISION_STATE() {
	if (Character::isDigit(_currentChar)) {
		_token->setPrecision(parseInt(_format));
	} else {
		// the precision is required but not given by the format string
		throw UnknownFormatConversionException(_HERE_, getFormatString());
	}
	_state = CONVERSION_TYPE_STATE;
}

void ParserStateMachine::process_CONVERSION_TYPE_STATE() {
	_token->setConversionType(_currentChar);
	_rawToken.add((char)tolower(_currentChar));
	_state = EXIT_STATE;
}

void ParserStateMachine::process_EXIT_STATE() {
	_token->setPlainText(getFormatString());
	_token->setFormat(_rawToken.toString());
}

int32_t ParserStateMachine::parseInt(const SPtr<CharBuffer> &buffer) {
	ptrdiff_t start = buffer->position() - 1;
	ptrdiff_t end = buffer->limit();
	while (buffer->hasRemaining()) {
		if (!Character::isDigit(buffer->get())) {
			end = buffer->position() - 1;
			break;
		}
	}
	buffer->position(0);
	UPtr<String> intStr = buffer->subSequence(start, end)->toString();
	buffer->position(end);
	try {
		int32_t res = Integer::parseInt(CPtr(intStr));
		_rawToken.add(*intStr);
		return res;
	} catch (NumberFormatException e) {
		return FormatToken::UNSET;
	}
}

static SPtr<Object> getArgument(ArgList const& args, int32_t index, SPtr<FormatToken> const& token, SPtr<Object> const& lastArgument, bool hasLastArgumentSet, bool allowNil) {
	if (index == FormatToken::LAST_ARGUMENT_INDEX && !hasLastArgumentSet)
		throw MissingFormatArgumentException(_HERE_, "<");

	if (index >= (ptrdiff_t)args.size() - 1) {
		throw MissingFormatArgumentException(_HERE_, *token->getPlainText());
	}

	if (index == FormatToken::LAST_ARGUMENT_INDEX) {
		return lastArgument;
	}

	return (allowNil ? args.getNullable((size_t)index + 1) : args.get((size_t)index + 1));
}

static SPtr<String> padding(SPtr<FormatToken> const& token, StringBuilder &source, int32_t startIndex) {
	int32_t start = startIndex;
	bool paddingRight = token->isFlagSet(FormatToken::FLAG_MINUS);
	char paddingChar = ' '; // space as padding char.
	if (token->isFlagSet(FormatToken::FLAG_ZERO))
		paddingChar = '0';
	else {
		// if padding char is space, always pad from the head location.
		start = 0;
	}
	int32_t width = token->getWidth();
	int32_t precision = token->getPrecision();

	size_t length = source.length();
	if (precision >= 0) {
		length = std::min(length, (size_t)precision);
		source.remove(length, source.length());
	}
	if (width > 0)
		width = std::max((int32_t)source.length(), width);
	if ((int32_t)length >= width)
		return source.toString();

	std::string insertString((size_t)width - length, paddingChar);

	if (paddingRight) {
		source.add(insertString);
	} else {
		source.insert((size_t)start, CPtr(insertString));
	}
	return source.toString();
}

static SPtr<String> formatBool(SPtr<FormatToken> const& token, SPtr<Object> const& arg) {
	StringBuilder result;
	int startIndex = 0;
	int32_t flags = token->getFlags();

	if (token->isFlagSet(FormatToken::FLAG_MINUS) && !token->isWidthSet())
		throw MissingFormatWidthException(_HERE_, fmt::format("-{}", token->getConversionType()).c_str());

	// only '-' is valid for flags
	if (FormatToken::FLAGS_UNSET != flags && FormatToken::FLAG_MINUS != flags)
		throw FormatFlagsConversionMismatchException(_HERE_, token->getStrFlags(), token->getConversionType());

	if (!arg) {
		result.add("false");
	} else if (instanceof<Boolean>(arg)) {
		result.add(arg);
	} else {
		result.add("true");
	}
	return padding(token, result, startIndex);
}

static SPtr<String> formatString(SPtr<FormatToken> const& token, SPtr<Object> const& arg) {
	StringBuilder result;
	int startIndex = 0;
	int32_t flags = token->getFlags();

	if (token->isFlagSet(FormatToken::FLAG_MINUS) && !token->isWidthSet())
		throw MissingFormatWidthException(_HERE_, fmt::format("-{}", token->getConversionType()).c_str());

	// only '-' is valid for flags
	if (FormatToken::FLAGS_UNSET != flags && FormatToken::FLAG_MINUS != flags)
		throw FormatFlagsConversionMismatchException(_HERE_, token->getStrFlags(), token->getConversionType());

	result.add(arg);
	return padding(token, result, startIndex);
}

static SPtr<String> formatCharacter(SPtr<FormatToken> const& token, SPtr<Object> const& arg) {
	StringBuilder result;

	int32_t startIndex = 0;
	int32_t flags = token->getFlags();

	if (token->isFlagSet(FormatToken::FLAG_MINUS) && !token->isWidthSet())
		throw MissingFormatWidthException(_HERE_, fmt::format("-{}", token->getConversionType()).c_str());

	// only '-' is valid for flags
	if (FormatToken::FLAGS_UNSET != flags && FormatToken::FLAG_MINUS != flags)
		throw FormatFlagsConversionMismatchException(_HERE_, token->getStrFlags(), token->getConversionType());

	if (token->isPrecisionSet())
		throw IllegalFormatPrecisionException(_HERE_, token->getPrecision());

	if (!arg)
		result.add("null");
	else {
		if (instanceof<Character>(arg)) {
			result.add(arg);
		} else if (instanceof<Short>(arg)) {
			short s = Class::cast<Short>(arg)->shortValue();
			if (!std::isprint(s))
				throw IllegalFormatCodePointException(_HERE_, s);
			result.add((char) s);
		} else if (instanceof<Integer>(arg)) {
			int codePoint = Class::cast<Integer>(arg)->intValue();
			if (!std::isprint(codePoint))
				throw IllegalFormatCodePointException(_HERE_, codePoint);
			result.add((char)codePoint);
		} else {
			// argument of other class is not acceptable.
			throw IllegalFormatConversionException(_HERE_, token->getConversionType(), arg->getClass());
		}
	}
	return padding(token, result, startIndex);
}

static SPtr<String> formatPercent(SPtr<FormatToken> const& token) {
	StringBuilder result("%");

	int32_t startIndex = 0;
	int32_t flags = token->getFlags();

	if (token->isFlagSet(FormatToken::FLAG_MINUS) && !token->isWidthSet())
		throw MissingFormatWidthException(_HERE_, fmt::format("-{}", token->getConversionType()).c_str());

	if (FormatToken::FLAGS_UNSET != flags && FormatToken::FLAG_MINUS != flags)
		throw FormatFlagsConversionMismatchException(_HERE_, token->getStrFlags(), token->getConversionType());

	if (token->isPrecisionSet())
		throw IllegalFormatPrecisionException(_HERE_, token->getPrecision());

	return padding(token, result, startIndex);
}

static SPtr<String> formatNull(SPtr<FormatToken> const& token) {
	token->setFlags(token->getFlags() & (~FormatToken::FLAG_ZERO));
	StringBuilder result("null");
	return padding(token, result, 0);
}

static SPtr<String> formatInteger(SPtr<FormatToken> const& token, SPtr<Object> const& arg) {
	if (!arg)
		return formatNull(token);

	int64_t value;

	if (instanceof<Double>(arg))
		value = Class::cast<Double>(arg)->longValue();
	else if (instanceof<Long>(arg))
		value = Class::cast<Long>(arg)->longValue();
	else if (instanceof<ULong>(arg))
		value = Class::cast<ULong>(arg)->longValue();
	else if (instanceof<Integer>(arg))
		value = Class::cast<Integer>(arg)->longValue();
	else if (instanceof<UInt>(arg))
		value = Class::cast<UInt>(arg)->longValue();
	else if (instanceof<Short>(arg))
		value = Class::cast<Short>(arg)->longValue();
	else
		throw IllegalFormatConversionException(_HERE_, token->getConversionType(), arg->getClass());

	return std::make_shared<String>(fmt::sprintf(token->getFormat()->c_str(), value));
}

static SPtr<String> formatFloat(SPtr<FormatToken> const& token, SPtr<Object> const& arg) {
	if (!arg)
		return formatNull(token);

	double value;

	if (instanceof<Double>(arg))
		value = Class::cast<Double>(arg)->doubleValue();
	else if (instanceof<Long>(arg))
		value = Class::cast<Long>(arg)->doubleValue();
	else if (instanceof<ULong>(arg))
		value = Class::cast<ULong>(arg)->doubleValue();
	else if (instanceof<Integer>(arg))
		value = Class::cast<Integer>(arg)->doubleValue();
	else if (instanceof<UInt>(arg))
		value = Class::cast<UInt>(arg)->doubleValue();
	else if (instanceof<Short>(arg))
		value = Class::cast<Short>(arg)->doubleValue();
	else
		throw IllegalFormatConversionException(_HERE_, token->getConversionType(), arg->getClass());

	return std::make_shared<String>(fmt::sprintf(token->getFormat()->c_str(), value));
}

static SPtr<String> formatArg(SPtr<FormatToken> const& token, SPtr<Object> const& arg) {
	SPtr<String> result;
	switch (token->getConversionType()) {
		case 'B':
		case 'b':
			result = formatBool(token, arg);
			break;
		case 'S':
		case 's':
			result = formatString(token, arg);
			break;
		case 'C':
		case 'c':
			result = formatCharacter(token, arg);
			break;
		case 'd':
		case 'o':
		case 'x':
		case 'X':
			result = formatInteger(token, arg);
			break;
		case 'e':
		case 'E':
		case 'g':
		case 'G':
		case 'f':
		case 'a':
		case 'A':
			result = formatFloat(token, arg);
			break;
		case '%':
			result = formatPercent(token);
			break;
		default:
			throw UnknownFormatConversionException(_HERE_, String::valueOf(token->getConversionType()));
	}
	if (Character::isUpperCase(token->getConversionType()))
		if (result)
			result = result->toUpperCase();
	return result;
}

void ExpressionFormatter::format(StringBuilder &out, ArgList const& args, Resolver const& resolver) {
	SPtr<String> format = args.get<String>(0);
	SPtr<CharBuffer> formatBuffer = std::make_shared<CharBuffer>(format);
	ParserStateMachine parser(formatBuffer);

	bool allowNil = false;

	int32_t currentObjectIndex = 0;
	SPtr<Object> lastArgument;
	bool hasLastArgumentSet = false;
	while (formatBuffer->hasRemaining()) {
		parser.reset();
		SPtr<FormatToken> token = parser.getNextFormatToken();
		SPtr<String> result;
		SPtr<String> plainText = token->getPlainText();
		if (token->getConversionType() == FormatToken::UNSET)
			result = plainText;
		else {
			plainText = plainText->substring(0, (size_t)plainText->indexOf('%'));
			SPtr<Object> argument;
			if (token->requiresArgument()) {
				int32_t index = (token->getArgIndex() == FormatToken::UNSET) ? currentObjectIndex++ : token->getArgIndex();
				argument = getArgument(args, index, token, lastArgument, hasLastArgumentSet, allowNil);
				lastArgument = argument;
				hasLastArgumentSet = true;
			}
			result = formatArg(token, argument);
			result = result ? ((StringBuilder(*plainText) + (*result)).toString()) : plainText;
		}
		if (result)
			out.add(*result);
	}
}

} // namespace expr
} // namespace slib
