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

namespace slib {
namespace expr {

CharBuffer::~CharBuffer() {}

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

static SPtr<Object> getArgument(ArgList const& args, int32_t index, SPtr<FormatToken> const& token, SPtr<Object> const& lastArgument, bool hasLastArgumentSet, bool allowNil) {
	if (index == FormatToken::LAST_ARGUMENT_INDEX && !hasLastArgumentSet)
		throw MissingFormatArgumentException(_HERE_, "<");

	if (index >= (ptrdiff_t)args.size() - 1) {
		throw MissingFormatArgumentException(_HERE_, token->getPlainText());
	}

	if (index == FormatToken::LAST_ARGUMENT_INDEX) {
		return lastArgument;
	}

	return (allowNil ? args.getNullable((size_t)index + 1) : args.get((size_t)index + 1));
}

static SPtr<String> padding(SPtr<FormatToken> const& token, StringBuilder &source, int startIndex) {
	int start = startIndex;
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
		source.insert((size_t)start, Ptr(insertString));
	}
	return source.toString();
}

static SPtr<String> formatBool(SPtr<FormatToken> const& token, SPtr<Object> const& arg) {
	StringBuilder result;
	int startIndex = 0;
	int flags = token->getFlags();

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
	int flags = token->getFlags();

	if (token->isFlagSet(FormatToken::FLAG_MINUS) && !token->isWidthSet())
		throw MissingFormatWidthException(_HERE_, fmt::format("-{}", token->getConversionType()).c_str());

	// only '-' is valid for flags
	if (FormatToken::FLAGS_UNSET != flags && FormatToken::FLAG_MINUS != flags)
		throw FormatFlagsConversionMismatchException(_HERE_, token->getStrFlags(), token->getConversionType());

	result.add(arg);
	return padding(token, result, startIndex);
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
	}
	return result;
}

void ExpressionFormatter::format(ArgList const& args, SPtr<Resolver> const& resolver) {
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
			_out->add(*result);
	}

}

} // namespace expr
} // namespace slib
