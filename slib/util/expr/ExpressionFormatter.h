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

#ifndef H_SLIB_UTIL_EXPR_EXPRESSIONFORMATTER_H
#define H_SLIB_UTIL_EXPR_EXPRESSIONFORMATTER_H

#include "slib/lang/StringBuilder.h"
#include "slib/util/expr/Function.h"
#include "slib/text/StringCharacterIterator.h"
#include "slib/lang/Character.h"

namespace slib {
namespace expr {

class CharBuffer : public StringCharacterIterator {
protected:
	ptrdiff_t _offset;
public:
	CharBuffer(SPtr<BasicString> const& text, ptrdiff_t begin, ptrdiff_t end)
	:StringCharacterIterator(text, begin, end, begin)
	,_offset(begin) {}

	CharBuffer(SPtr<BasicString> const& text)
	:StringCharacterIterator(text)
	,_offset(0) {}

	virtual ~CharBuffer() override;

	ptrdiff_t position() const {
		return getIndex() - _offset;
	}

	ptrdiff_t limit() const {
		return getEndIndex() - _offset;
	}

	bool hasRemaining() const {
		return getIndex() < getEndIndex();
	}

	char get() {
		char c = current();
		next();
		return c;
	}

	void position(ptrdiff_t newPosition) {
		if ((newPosition > getEndIndex()) || newPosition < 0)
			throw IllegalArgumentException(_HERE_);
		_pos = newPosition - _offset;
	}

	void rewind() {
		position(0);
	}

	UPtr<CharBuffer> subSequence(ptrdiff_t start, ptrdiff_t end) {
		return std::make_unique<CharBuffer>(_text, start, end);
	}

	UPtr<String> toString() const {
		const char *text = _text->c_str();
		return std::make_unique<String>(text + _pos, getEndIndex() - _pos);
	}
};

class IllegalFormatException : public IllegalArgumentException {
protected:
	IllegalFormatException(const char *where, const char *className, const char *msg)
	:IllegalArgumentException(where, className, msg) {}
};

class DuplicateFormatFlagsException : public IllegalFormatException {
private:
	SPtr<String> _flags;
public:
	DuplicateFormatFlagsException(const char *where, UPtr<String> f)
	:IllegalFormatException(where, "DuplicateFormatFlagsException", fmt::format("Flags = '{}'", *f).c_str())
	,_flags(std::make_shared<String>(*f)) {}
};

class UnknownFormatConversionException : public IllegalFormatException {
private:
	SPtr<String> _s;
public:
	UnknownFormatConversionException(const char *where, UPtr<String> s)
	:IllegalFormatException(where, "UnknownFormatConversionException", fmt::format("Conversion = '{}'", *s).c_str())
	,_s(std::make_shared<String>(*s)) {}
};

class MissingFormatArgumentException : public IllegalFormatException {
private:
	SPtr<String> _s;
public:
	MissingFormatArgumentException(const char *where, const char *s)
	:IllegalFormatException(where, "MissingFormatArgumentException", fmt::format("Format specifier = '{}'", *s).c_str())
	,_s(std::make_shared<String>(s)) {}

	MissingFormatArgumentException(const char *where, String const& s)
	:IllegalFormatException(where, "MissingFormatArgumentException", fmt::format("Format specifier = '{}'", s).c_str())
	,_s(std::make_shared<String>(s)) {}
};

class MissingFormatWidthException : public IllegalFormatException {
private:
	SPtr<String> _s;
public:
	MissingFormatWidthException(const char *where, const char *s)
	:IllegalFormatException(where, "MissingFormatWidthException", s)
	,_s(std::make_shared<String>(s)) {}
};

class FormatFlagsConversionMismatchException : public IllegalFormatException {
private:
	SPtr<String> _f;
	char _c;
public:
	FormatFlagsConversionMismatchException(const char *where, UPtr<String> f, char c)
	:IllegalFormatException(where, "FormatFlagsConversionMismatchException", fmt::format("Conversion = {}, Flags = {}", c, *f).c_str())
	,_f(std::make_shared<String>(*f))
	,_c(c) {}
};

class IllegalFormatCodePointException : public IllegalFormatException {
private:
	int32_t _c;
public:
	IllegalFormatCodePointException(const char *where, int32_t c)
	:IllegalFormatException(where, "IllegalFormatCodePointException", fmt::format("Code point = {:#x}", c).c_str())
	,_c(c) {}
};

class IllegalFormatConversionException : public IllegalFormatException {
private:
	char _c;
	Class const* _arg;
public:
	IllegalFormatConversionException(const char *where, char c, Class const* arg)
	:IllegalFormatException(where, "IllegalFormatConversionException", fmt::format("{} != {}", c, arg->getName()).c_str())
	,_c(c)
	,_arg(arg) {}
};

class IllegalFormatPrecisionException : public IllegalFormatException {
private:
	int32_t _p;
public:
	IllegalFormatPrecisionException(const char *where, int32_t p)
	:IllegalFormatException(where, "IllegalFormatPrecisionException", Integer::toString(p)->c_str())
	,_p(p) {}
};

class FormatToken {
public:
	static constexpr int32_t LAST_ARGUMENT_INDEX = -2;

	static constexpr int32_t UNSET = -1;

	static constexpr int32_t FLAGS_UNSET = 0;

	static constexpr int32_t DEFAULT_PRECISION = 6;

	static constexpr int32_t FLAG_MINUS = 1;

	static constexpr int32_t FLAG_SHARP = 1 << 1;

	static constexpr int32_t FLAG_ADD = 1 << 2;

	static constexpr int32_t FLAG_SPACE = 1 << 3;

	static constexpr int32_t FLAG_ZERO = 1 << 4;

	static constexpr int32_t FLAG_COMMA = 1 << 5;

	static constexpr int32_t FLAG_PARENTHESIS = 1 << 6;
private:
	static constexpr size_t FLAG_TYPE_COUNT = 6;
	ptrdiff_t _formatStringStartIndex;
	SPtr<String> _plainText;
	UPtr<String> _format;
	int32_t _argIndex;
	int32_t _flags;
	int32_t _width;
	int32_t _precision;
	StringBuilder _strFlags;
	char _conversionType;
public:
	FormatToken()
	:_argIndex(UNSET)
	,_flags(0)
	,_width(UNSET)
	,_precision(UNSET)
	,_strFlags(nullptr, FLAG_TYPE_COUNT)
	,_conversionType(UNSET) {}

	bool isPrecisionSet() {
		return _precision != UNSET;
	}

	bool isWidthSet() {
		return _width != UNSET;
	}

	bool isFlagSet(int flag) {
		return 0 != (_flags & flag);
	}

	int32_t getArgIndex() {
		return _argIndex;
	}

	void setArgIndex(int32_t index) {
		_argIndex = index;
	}

	SPtr<String> getPlainText() const {
		return _plainText;
	}

	void setPlainText(SPtr<String> const& plainText) {
		_plainText = plainText;
	}

	String const* getFormat() const {
		return _format.get();
	}

	void setFormat(UPtr<String> format) {
		_format = std::move(format);
	}

	int32_t getWidth() {
		return _width;
	}

	void setWidth(int32_t width) {
		_width = width;
	}

	int32_t getPrecision() {
		return _precision;
	}

	void setPrecision(int32_t precision) {
		_precision = precision;
	}

	UPtr<String> getStrFlags() {
		return _strFlags.toString();
	}

	int32_t getFlags() {
		return _flags;
	}

	void setFlags(int32_t flags) {
		_flags = flags;
	}

	/**
	 * Sets qualified char as one of the flags. If the char is qualified,
	 * sets it as a flag and returns true, else returns false.
	 */
	bool setFlag(char c);

	ptrdiff_t getFormatStringStartIndex() {
		return _formatStringStartIndex;
	}

	void setFormatStringStartIndex(ptrdiff_t index) {
		_formatStringStartIndex = index;
	}

	char getConversionType() {
		return _conversionType;
	}

	void setConversionType(char c) {
		_conversionType = c;
	}

	bool requiresArgument() {
		return (_conversionType != '%') && (_conversionType != 'n');
	}
};

class ParserStateMachine {
private:
	static constexpr char EOS = (char) -1;

	static constexpr int EXIT_STATE = 0;
	static constexpr int ENTRY_STATE = 1;
	static constexpr int START_CONVERSION_STATE = 2;
	static constexpr int FLAGS_STATE = 3;
	static constexpr int WIDTH_STATE = 4;
	static constexpr int PRECISION_STATE = 5;
	static constexpr int CONVERSION_TYPE_STATE = 6;
	static constexpr int SUFFIX_STATE = 7;

	SPtr<FormatToken> _token;
	StringBuilder _rawToken;
	int32_t _state;
	char _currentChar;
	SPtr<CharBuffer> _format;
public:
	ParserStateMachine(SPtr<CharBuffer> const& format)
	:_state(ENTRY_STATE)
	,_currentChar(0)
	,_format(format) {}

	void reset() {
		_currentChar = FormatToken::UNSET;
		_state = ENTRY_STATE;
		_token = nullptr;
		_rawToken.clear();
	}

	/**
	 * Gets the information about the current format token. Information is
	 * recorded in the FormatToken returned and the position of the stream
	 * for the format string will be advanced until the next format token.
	 */
	SPtr<FormatToken> getNextFormatToken() {
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
private:
	char getNextFormatChar() {
		if (_format->hasRemaining())
			return _format->get();
		return EOS;
	}

	UPtr<String> getFormatString() {
		ptrdiff_t end = _format->position();
		_format->rewind();
		UPtr<String> formatString = _format->subSequence(_token->getFormatStringStartIndex(), end)->toString();
		_format->position(end);
		return formatString;
	}

	void process_ENTRY_STATE() {
		if (EOS == _currentChar)
			_state = ParserStateMachine::EXIT_STATE;
		else if ('%' == _currentChar) {
			// change to conversion type state
			_state = START_CONVERSION_STATE;
			_rawToken.add('%');
		}
		// else remains in ENTRY_STATE
	}

	void process_START_CONVERSION_STATE() {
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

	void process_FLAGS_STATE() {
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

	void process_WIDTH_STATE() {
		if ('.' == _currentChar) {
			_state = PRECISION_STATE;
			_rawToken.add('.');
		} else {
			_state = CONVERSION_TYPE_STATE;
			// do not get the next char
			_format->position(_format->position() - 1);
		}
	}

	void process_PRECISION_STATE() {
		if (Character::isDigit(_currentChar)) {
			_token->setPrecision(parseInt(_format));
		} else {
			// the precision is required but not given by the format string
			throw UnknownFormatConversionException(_HERE_, getFormatString());
		}
		_state = CONVERSION_TYPE_STATE;
	}

	void process_CONVERSION_TYPE_STATE() {
		_token->setConversionType(_currentChar);
		_rawToken.add((char)tolower(_currentChar));
		_state = EXIT_STATE;
	}

	void process_EXIT_STATE() {
		_token->setPlainText(getFormatString());
		_token->setFormat(_rawToken.toString());
	}

	/**
	 * Parses integer value from the given buffer
	 */
	int32_t parseInt(SPtr<CharBuffer> const& buffer) {
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
};

/**
 * Originally part of the Apache Harmony project. Adapted and modified to integrate with the
 * ExpressionEvaluator.
 */
class ExpressionFormatter {
public:
	/**
	 * @throws EvaluationException
	 */
	static void format(StringBuilder &out, ArgList const& args, SPtr<Resolver> const& resolver);
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_EXPRESSIONFORMATTER_H
