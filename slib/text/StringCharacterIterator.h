/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_TEXT_STRINGCHARACTERITERATOR_H
#define H_SLIB_TEXT_STRINGCHARACTERITERATOR_H

#include "slib/text/CharacterIterator.h"
#include "slib/lang/String.h"
#include "slib/exception/IllegalArgumentException.h"

namespace slib {

class StringCharacterIterator : public CharacterIterator {
protected:
	SPtr<BasicString> _text;
	ptrdiff_t _pos;
private:
	const char *_buffer;
	ptrdiff_t _begin;
	ptrdiff_t _end;
public:
	StringCharacterIterator(SPtr<BasicString> const& text, ptrdiff_t begin, ptrdiff_t end, ptrdiff_t pos);

	StringCharacterIterator(SPtr<BasicString> const& text)
	:StringCharacterIterator(text, 0, (ssize_t)text->length(), 0) {}

	virtual char first() override;

	virtual char last() override;

	virtual char setIndex(ssize_t position) override;

	virtual char current() override {
		if ((_pos >= _begin) && (_pos < _end))
			return _buffer[(size_t)_pos];

		return DONE;
	}

	virtual char next() override;

	virtual char previous() override;

	virtual ptrdiff_t getBeginIndex() const override {
		return _begin;
	}


	virtual ptrdiff_t getEndIndex() const override {
		return _end;
	}

	virtual ptrdiff_t getIndex() const override {
		return _pos;
	}
};

} // namespace slib

#endif // H_SLIB_TEXT_STRINGCHARACTERITERATOR_H

