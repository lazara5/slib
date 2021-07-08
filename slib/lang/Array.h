/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LANG_ARRAY_H
#define H_SLIB_LANG_ARRAY_H

#include <slib/util/TemplateUtils.h>

#include <type_traits>
#include <vector>

namespace slib {

template <class T, bool p = std::is_arithmetic<T>::value>
class Array;

template <class T>
class Array<T, true> {
private:
	std::vector<T> _data;
public:
	Array(size_t n)
	: _data(n) {}

	size_t length() const {
		return _data.size();
	}

	T& operator [](size_t i) {
		return _data[i];
	}

	T *data() {
		return _data.data();
	}

	T const* constData() const {
		return _data.data();
	}

	void resize(size_t new_size) {
		_data.resize(new_size);
	}
};

template <class T>
class Array<T, false> {
private:
	std::vector<SPtr<T>> _data;
public:
	Array(size_t n)
	: _data(n) {}

	size_t length() const {
		return _data.size();
	}

	SPtr<T>& operator [](size_t i) {
		return _data[i];
	}
};

} // namespace slib

#endif // H_SLIB_LANG_ARRAY_H
