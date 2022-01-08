/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LANG_ARRAY_H
#define H_SLIB_LANG_ARRAY_H

#include "slib/lang/Object.h"
#include <slib/util/TemplateUtils.h>

#include <type_traits>
#include <vector>

namespace slib {

namespace reflect {
	struct Array;
}

class IArray : virtual public Object {
friend struct reflect::Array;
public:
	TYPE_INFO(IArray, CLASS(IArray), INHERITS(Object));
public:
	virtual size_t length() const = 0;
};

template <class T, bool p = std::is_arithmetic<T>::value>
class Array;

template <class T>
class Array<T, true> : public IArray {
public:
	TYPE_INFO(Array, CLASS(Array<T, true>), INHERITS(IArray));
private:
	std::vector<T> _data;
public:
	Array(size_t n = 0)
	: _data(n) {}

	virtual size_t length() const override {
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
class Array<T, false> : public IArray {
public:
	TYPE_INFO(Array, CLASS(Array<T, false>), INHERITS(Object));
private:
	std::vector<SPtr<T>> _data;
public:
	Array(size_t n = 0)
	: _data(n) {}

	virtual size_t length() const override {
		return _data.size();
	}

	SPtr<T>& operator [](size_t i) {
		return _data[i];
	}
};

} // namespace slib

#endif // H_SLIB_LANG_ARRAY_H
