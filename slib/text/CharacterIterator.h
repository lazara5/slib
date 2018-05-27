/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_TEXT_CHARACTERITERATOR_H
#define H_SLIB_TEXT_CHARACTERITERATOR_H

#include <stddef.h>

namespace slib {

class CharacterIterator {
public:
	/**
	 * Constant that is returned when the iterator has reached either the end
	 * or the beginning of the text */
	static constexpr unsigned char DONE = 0;
public:

	virtual ~CharacterIterator() {}

	/**
	 * Sets the position to getBeginIndex() and returns the character at that position.
	 * @return the first character in the text, or DONE if the text is empty
	 */
	virtual char first() = 0;

	/**
	 * Sets the position to getEndIndex()-1 (or getEndIndex() if the text is empty)
	 * and returns the character at that position.
	 * @return the last character in the text, or DONE if the text is empty
	 */
	virtual char last() = 0;

	/**
	 * Returns the character at the current position.
	 * @return the character at the current position or DONE if the current
	 * position is beyond the end of the text.
	 */
	virtual char current() = 0;

	/**
	 * Increments the iterator's index and returns the character
	 * at the new index. If the resulting index is greater or equal
	 * to getEndIndex(), the current index is set to getEndIndex() and
	 * a value of DONE is returned.
	 * @return the character at the new position or DONE if the new
	 * position is beyond the end of the text.
	 */
	virtual char next() = 0;

	/**
	 * Decrements the iterator's index and returns the character
	 * at the new index. If the current index is getBeginIndex(), the index
	 * remains at getBeginIndex() and a value of DONE is returned.
	 * @return the character at the new position or DONE if the current
	 * position is equal to getBeginIndex().
	 */
	virtual char previous() = 0;

	/**
	 * Sets the position to the specified position in the text and returns that character.
	 * @param position  the position within the text. Valid values are between
	 * getBeginIndex() and getEndIndex().  An IllegalArgumentException is thrown
	 * if an invalid value is provided.
	 * @return the character at the specified position or DONE if the specified position is equal to getEndIndex()
	 * @throws IllegalArgumentException
	 */
	virtual char setIndex(ptrdiff_t position) = 0;

	/**
	 * Returns the start index of the text.
	 * @return the index at which the text begins.
	 */
	virtual ptrdiff_t getBeginIndex() const = 0;

	/**
	 * Returns the end index of the text.  This index is the index of the first
	 * character following the end of the text.
	 * @return the index after the last character in the text
	 */
	virtual ptrdiff_t getEndIndex() const = 0;

	/**
	 * Returns the current index.
	 * @return the current index.
	 */
	virtual ptrdiff_t getIndex() const = 0;
};

} // namespace slib

#endif // H_SLIB_TEXT_CHARACTERITERATOR_H
