/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/text/StringCharacterIterator.h"
#include "slib/exception/IllegalArgumentException.h"

namespace slib {

StringCharacterIterator::StringCharacterIterator(SPtr<IString> const& text, ptrdiff_t begin, ptrdiff_t end, ptrdiff_t pos)
:_text(text)
,_buffer(text->data()) {
	if (!text)
		throw NullPointerException(_HERE_);

	if ((begin < 0) || (begin > end) || (end > (ssize_t)text->length()))
		throw IllegalArgumentException(_HERE_, "Invalid range");
	if ((pos < begin) || (pos > end))
		throw IllegalArgumentException(_HERE_, "Invalid position");

	_begin = begin;
	_end = end;
	_pos = pos;
}

char StringCharacterIterator::first() {
	_pos = _begin;
	return current();
}

char StringCharacterIterator::last() {
	if (_end != _begin)
		_pos = _end - 1;
	else
		_pos = _end;
	return current();
}

char StringCharacterIterator::setIndex(ssize_t position) {
	if (position < _begin || position > _end)
		throw IllegalArgumentException(_HERE_, "Invalid index");
	_pos = position;
	return current();
}

char StringCharacterIterator::next() {
	if (_pos < _end - 1) {
		_pos++;
		return _buffer[(size_t)_pos];
	} else {
		_pos = _end;
		return DONE;
	}
}

char StringCharacterIterator::previous() {
	if (_pos > _begin) {
		_pos--;
		return _buffer[(size_t)_pos];
	} else
		return DONE;
}

} // namespace slib
