/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/collections/Properties.h"
#include "slib/String.h"

namespace slib {

Properties::LineProcessor::~LineProcessor() {}

Properties::LineReader::LineReader(Properties *props, InputStream &inStream)
:_inStream(inStream)
,_buffer(8192) {
	_props = props;

	_available = 0;
	_offset = 0;
}

void Properties::LineReader::refill() {
	_buffer.clear();
	_available = _inStream.read(_buffer);
	_offset = 0;
}

void Properties::LineReader::trimTrailingWhitespace(std::string &line, size_t *trailingWhitespace) {
	line.erase(line.length() - *trailingWhitespace);
	*trailingWhitespace = 0;
}

ptrdiff_t Properties::LineReader::readLine(std::string &line) {
	char c = 0;

	bool isComment = false;
	bool skippingWhitespace = true;
	bool skippingLF = false;
	bool isNewLine = true;
	bool isMultiLine = false;
	bool isEscape = false;
	bool isWhitespace = false;

	size_t trailingWhitespace = 0;

	line.clear();

	while (true) {
		// refill buffer if necessary
		if (_offset >= _available) {
			refill();
			if (_available <= 0) {
				if (line.empty() || isComment)
					return -1;
				trimTrailingWhitespace(line, &trailingWhitespace);
				return (ptrdiff_t)line.length();
			}
		}

		// fetch next char
		c = (char)_buffer[_offset++];

		if (skippingLF) {
			skippingLF = false;
			if (c == '\n')
				continue;
		}

		isWhitespace = (c == ' ' || c == '\t' || c == '\f') && (!isEscape);

		if (skippingWhitespace) {
			if (isWhitespace)
				continue;
			if (!isMultiLine && (c == '\r' || c == '\n'))
				continue;
			skippingWhitespace = false;
			isMultiLine = false;
		}

		if (isNewLine) {
			isNewLine = false;
			if (c == '#' || c == '!') {
				isComment = true;
				continue;
			}
		}

		if (c != '\n' && c != '\r') {
			if (isWhitespace)
				trailingWhitespace++;
			else
				trailingWhitespace = 0;

			line.push_back(c);

			if (c == '\\')
				isEscape = !isEscape;
			else
				isEscape = false;
		} else {
			// EOL reached
			if (isComment || line.empty()) {
				isComment = false;
				isNewLine = true;
				skippingWhitespace = true;
				line.clear();
				continue;
			}

			// refill buffer if necessary
			if (_offset >= _available) {
				refill();
				if (_available <= 0) {
					trimTrailingWhitespace(line, &trailingWhitespace);
					return (ptrdiff_t)line.length();
				}
			}
			if (isEscape) {
				line.erase(line.length() - 1);
				//skip leading whitespace in the following line
				skippingWhitespace = true;
				isMultiLine = true;
				isEscape = false;
				if (c == '\r')
					skippingLF = true;
			} else {
				trimTrailingWhitespace(line, &trailingWhitespace);
				return (ptrdiff_t)line.length();
			}
		}
	}
}

void Properties::internalLoad(LineReader *lr,
							  LineProcessor *lineProcessor) {
	ptrdiff_t limit;
	size_t keyLen;
	size_t valueStart;
	char c;
	bool haveSeparator;
	bool isEscape;

	std::string line;

	while ((limit = lr->readLine(line)) >= 0) {
		c = 0;
		keyLen = 0;
		valueStart = (size_t)limit;
		haveSeparator = false;
		isEscape = false;

		while (keyLen < (size_t)limit) {
			c = line[keyLen];
			if ((c == '=' ||  c == ':') && !isEscape) {
				valueStart = keyLen + 1;
				haveSeparator = true;
				break;
			} else if ((c == ' ' || c == '\t' ||  c == '\f') && !isEscape) {
				valueStart = keyLen + 1;
				break;
			}
			if (c == '\\')
				isEscape = !isEscape;
			else
				isEscape = false;
			keyLen++;
		}

		while (valueStart < (size_t)limit) {
			c = line[valueStart];
			if (c != ' ' && c != '\t' &&  c != '\f') {
				if (!haveSeparator && (c == '=' ||  c == ':'))
					haveSeparator = true;
				else
					break;
			}
			valueStart++;
		}
		std::string key = unescape(line, 0, keyLen);
		std::string value = unescape(line, valueStart, (size_t)limit - valueStart);
		setVariableProperty(key, value, lineProcessor);
	}
}

std::string Properties::unescape(std::string const& in, size_t offset, size_t len) {
	size_t end = offset + len;
	char c;
	std::string out;

	while (offset < end) {
		c = in[offset++];
		if (c == '\\') {
			c = in[offset++];
			if (c == 'u') {
				//TODO: read Unicode value
			} else {
				if (c == 't')
					c = '\t';
				else if (c == 'r')
					c = '\r';
				else if (c == 'n')
					c = '\n';
				else if (c == 'f')
					c = '\f';
				out.push_back(c);
			}
		} else
			out.push_back(c);
	}

	return out;
}

void Properties::setVariableProperty(std::string const& name, std::string const& value,
									 LineProcessor *lineProcessor) {
	if ((!lineProcessor))
		put(name, value);
	else {
		std::shared_ptr<std::string> finalVal;
		if (lineProcessor)
			finalVal = lineProcessor->processLine(name, value);
		else
			finalVal = std::make_shared<std::string>(value);

		if (finalVal)
			put(name, finalVal);
	}
}

} // namespace slib
