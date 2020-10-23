/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_PROPERTIES_H
#define H_SLIB_COLLECTIONS_PROPERTIES_H

#include "slib/collections/LinkedHashMap.h"
#include "slib/io/InputStream.h"
#include "slib/exception/ValueException.h"

namespace slib {

class Properties : public LinkedHashMap<String, String> {
public:
	static constexpr StringView _className {"Properties"_SV};
	typedef typename inherits<LinkedHashMap<String, String>>::types _classInherits;
public:
	class LineProcessor {
	public:
		typedef Map<String, String> Props;
	public:
		virtual ~LineProcessor();
		virtual UPtr<String> processLine(SPtr<String> const& name, SPtr<String> const& rawProperty) = 0;
	};

private:
	class LineReader {
	private:
		Properties *_props;
		InputStream &_inStream;

		ByteBuffer _buffer;
		ptrdiff_t _available;
		int _offset;
	private:
		void refill();
		void trimTrailingWhitespace(std::string &line, size_t *trailingWhitespace);
	public:
		LineReader(Properties *props, InputStream &inStream);

		ptrdiff_t readLine(std::string& line);
	};

	void internalLoad(LineReader *lr,
					  LineProcessor *lineProcessor);
	UPtr<String> unescape(std::string const& in, size_t offset, size_t len);

	virtual void setVariableProperty(SPtr<String> const& name, SPtr<String> const& value,
									 LineProcessor *lineProcessor);
protected:
	/** @throws NumberFormatException */
	template<class T, T MIN, T MAX>
	static const T& rangeCheck(const char *where, const T& value) {
		if ((value < MIN) || (value > MAX))
			throw NumberFormatException(where, "Value out of range");
		return value;
	}
public:
	virtual ~Properties() override {}

	virtual Class const& getClass() const override {
		return classOf<Properties>::_class();
	}

	/**
	 * Searches for the property with the specified key in this property list.
	 * The method returns the default value argument if the property is not found.
	 * @param name the key
	 */
	virtual SPtr<String> getProperty(String const& name) const {
		return get(name);
	}

	virtual SPtr<String> getProperty(String const& name,
									 SPtr<String> defaultValue) const {
		SPtr<String> value = get(name);
		if (value)
			return value;
		return defaultValue;
	}

	virtual SPtr<String> getProperty(String const& name,
									 String const& defaultValue) const {
		SPtr<String> value = get(name);
		if (value)
			return value;
		return std::make_shared<String>(defaultValue);
	}

	/**
	 * Inserts a new value in the property list.
	 * @param name the key
	 * @param value the value corresponding to the key
	 */
	virtual void setProperty(String const& name, String const& value) {
		put(std::make_shared<String>(name), std::make_shared<String>(value));
	}

	/**
	 * Inserts a new value in the property list.
	 * @param name the key
	 * @param value the value corresponding to the key
	 */
	virtual void setProperty(SPtr<String> const& name, SPtr<String> const& value) {
		put(name, value);
	}

	/**
	 * Reads a Java-style property list (key and element pairs) from the input
	 * character stream. The rules are the same as for Java property files,
	 * except that trailing whitespace in values is stripped (see below).
	 * <p>
	 * Properties are processed on a line-by-line basis. A line is defined as
	 * a line of characters that is terminated either by a set of line terminator
	 * characters (\r \n or \n \r) or by the end of the stream. A line may be
	 * either a blank line, a comment line or hold a key-element pair. A line
	 * containing a key-element pair may be spread across several lines by escaping
	 * the line terminator sequence with a backslash character.
	 * Note that a comment line cannot be extended in this manner.
	 * <p>
	 * A line that contains only white space characters is considered blank
	 * and is ignored.  A comment line has an ASCII '#' or '!' as its first
	 * non-whitespace character; comment lines are also ignored. In addition to
	 * line terminators, the characters space ' ', tab '\t' and form feed '\f'
	 * are considered whitespace. Escaped whitespace characters are <b>not</b>
	 * considered whitespace.
	 * <p>
	 * If a logical line is spread across several lines, the backslash escaping
	 * the line terminator sequence, the line terminators and any whitespace at
	 * the start of the following line are not part of the key or element values.
	 * <p>
	 * The key contains all of the characters in the line starting with the first
	 * non-whitespace character and up to, but not including, the first unescaped
	 * '=', ':' or whitespace character other than a line terminator. All of these
	 * key termination characters may be included in the key by escaping them with
	 * a preceding backslash character. Line terminator characters can be included
	 * using \r and \n escape sequences. Any whitespace after the key is skipped.
	 * If the first non-whitespace character after the key is '=' or ':', then it is
	 * ignored and any whitespace characters after it as well. All remaining characters
	 * on the line become part of the associated value, except for any trailing
	 * whitespace characters; if there are no remaining characters, the value is
	 * the empty string. The raw character sequences constituting the key and element
	 * are then unescaped according to the following specifications.
	 * <p>
	 * Characters in keys and elements can be represented by escape sequences similar
	 * to those used for character and string literals (see sections 3.3 and 3.10.6 of
	 * "The Java&trade; Language Specification"), with the following differences:
	 *
	 * <ul>
	 * <li> Octal escapes are not recognized.
	 *
	 * <li> The character sequence \b does <b>not</b> represent a backspace character.
	 *
	 * <li> A backslash character '\' before a non-valid escape character is not
	 * treated as an error; instead, the backslash is silently dropped.
	 *
	 * <li> Escapes are not necessary for single and double quotes.
	 *
	 * <li> Only a single 'u' character is allowed in a Unicode escape sequence.
	 * Note: Unicode escapes are <b>not yet supported</b> !.
	 *
	 * </ul>
	 *
	 * @param inStream  the input character stream.
	 * @param lineProcessor  external line processor called for each key-element line
	 * @throws IOException  if an error occurred when reading from the input stream.
	 */
	virtual void load(InputStream &inStream,
					  LineProcessor *lineProcessor = nullptr) {
		LineReader lr(this, inStream);
		internalLoad(&lr, lineProcessor);
	}

//--- typed getters -------------------------------------------------------------------------------

	/** @throws MissingValueException */
	SPtr<String> getString(String const& name) const {
		SPtr<String> val = getProperty(name);
		if (val)
			return val;
		throw MissingValueException(_HERE_, name.c_str());
	}

	SPtr<String> getString(String const& name, String const& defaultValue) const {
		try {
			return getString(name);
		} catch (MissingValueException const&) {
			return std::make_shared<String>(defaultValue);
		}
	}

	/**
	 * @throws MissingValueException
	 * @throws NumberFormatException
	 */
	template <int32_t MIN = slib::Integer::MIN_VALUE, int32_t MAX = slib::Integer::MAX_VALUE>
	int32_t getInt(const std::string& name) const {
		try {
			return rangeCheck<int32_t, MIN, MAX>(_HERE_, Integer::parseInt(CPtr(getString(name))));
		} catch (NumberFormatException const& e) {
			throw NumberFormatException(_HERE_, fmt::format("Invalid integer value: {} ({})", name, e.getMessage()).c_str());
		}
	}

	/** @throws NumberFormatException */
	template <int32_t MIN = slib::Integer::MIN_VALUE, int32_t MAX = slib::Integer::MAX_VALUE>
	int32_t getInt(const std::string& name, int32_t defaultValue) const {
		try {
			return getInt<MIN, MAX>(name);
		} catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	/**
	 * @throws MissingValueException
	 * @throws NumberFormatException
	 */
	template <uint32_t MIN = 0, uint32_t MAX = slib::UInt::MAX_VALUE>
	uint32_t getUInt(const std::string& name) const {
		try {
			return rangeCheck<uint32_t, MIN, MAX>(_HERE_, UInt::parseUInt(getString(name)));
		} catch (NumberFormatException const& e) {
			throw NumberFormatException(_HERE_, fmt::format("Invalid integer value: {} ({})", name, e.getMessage()).c_str());
		}
	}

	/** @throws NumberFormatException */
	template <uint32_t MIN = 0, uint32_t MAX = slib::UInt::MAX_VALUE>
	uint32_t getUInt(const std::string& name, uint32_t defaultValue) const {
		try {
			return getUInt<MIN, MAX>(name);
		} catch (MissingValueException const&) {
			return defaultValue;
		}
	}

	/** @throws MissingValueException */
	bool getBool(const std::string& name) const {
		return Boolean::parseBoolean(CPtr(getString(name)));
	}

	bool getBool(const std::string& name, bool defaultValue) const {
		try {
			return getBool(name);
		} catch (MissingValueException const&) {
			return defaultValue;
		}
	}
};

} // namespace slib

#endif
