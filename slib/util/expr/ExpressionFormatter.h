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
	: StringCharacterIterator(text, begin, end, begin)
	, _offset(begin) {}

	CharBuffer(SPtr<BasicString> const& text)
	: StringCharacterIterator(text)
	, _offset(0) {}

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
		return newU<CharBuffer>(_text, start, end);
	}

	UPtr<String> toString() const {
		const char *text = _text->data();
		return newU<String>(text + _pos, getEndIndex() - _pos);
	}
};

class IllegalFormatException : public IllegalArgumentException {
protected:
	IllegalFormatException(const char *where, const char *className, const char *msg)
	: IllegalArgumentException(where, className, msg) {}
};

class DuplicateFormatFlagsException : public IllegalFormatException {
public:
	DuplicateFormatFlagsException(const char *where, UPtr<String> f)
	: IllegalFormatException(where, "DuplicateFormatFlagsException", fmt::format("Flags = '{}'", *f).c_str()) {}
};

class UnknownFormatConversionException : public IllegalFormatException {
public:
	UnknownFormatConversionException(const char *where, UPtr<String> s)
	: IllegalFormatException(where, "UnknownFormatConversionException", fmt::format("Conversion = '{}'", *s).c_str()) {}
};

class MissingFormatArgumentException : public IllegalFormatException {
public:
	MissingFormatArgumentException(const char *where, const char *s)
	: IllegalFormatException(where, "MissingFormatArgumentException", fmt::format("Format specifier = '{}'", *s).c_str()) {}

	MissingFormatArgumentException(const char *where, String const& s)
	: IllegalFormatException(where, "MissingFormatArgumentException", fmt::format("Format specifier = '{}'", s).c_str()) {}
};

class MissingFormatWidthException : public IllegalFormatException {
public:
	MissingFormatWidthException(const char *where, const char *s)
	: IllegalFormatException(where, "MissingFormatWidthException", s) {}
};

class FormatFlagsConversionMismatchException : public IllegalFormatException {
public:
	FormatFlagsConversionMismatchException(const char *where, UPtr<String> f, char c)
	: IllegalFormatException(where, "FormatFlagsConversionMismatchException", fmt::format("Conversion = {}, Flags = {}", c, *f).c_str()) {}
};

class IllegalFormatCodePointException : public IllegalFormatException {
public:
	IllegalFormatCodePointException(const char *where, int32_t c)
	: IllegalFormatException(where, "IllegalFormatCodePointException", fmt::format("Code point = {:#x}", c).c_str()) {}
};

class IllegalFormatConversionException : public IllegalFormatException {
public:
	IllegalFormatConversionException(const char *where, char c, Class const& arg)
	: IllegalFormatException(where, "IllegalFormatConversionException", fmt::format("{} != {}", c, arg.getName()).c_str()) {}
};

class IllegalFormatPrecisionException : public IllegalFormatException {
public:
	IllegalFormatPrecisionException(const char *where, int32_t p)
	: IllegalFormatException(where, "IllegalFormatPrecisionException", Integer::toString(p)->c_str()) {}
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
	: _argIndex(UNSET)
	, _flags(0)
	, _width(UNSET)
	, _precision(UNSET)
	, _strFlags(nullptr, FLAG_TYPE_COUNT)
	, _conversionType(UNSET) {}

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
	SPtr<FormatToken> getNextFormatToken();
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

	void process_ENTRY_STATE();

	void process_START_CONVERSION_STATE();

	void process_FLAGS_STATE();

	void process_WIDTH_STATE();

	void process_PRECISION_STATE();

	void process_CONVERSION_TYPE_STATE();

	void process_EXIT_STATE();

	/**
	 * Parses integer value from the given buffer
	 */
	int32_t parseInt(SPtr<CharBuffer> const& buffer);
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
