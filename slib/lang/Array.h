/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LANG_ARRAY_H
#define H_SLIB_LANG_ARRAY_H

#include "slib/lang/Object.h"
#include "slib/util/TemplateUtils.h"
#include "slib/exception/IllegalArgumentException.h"
#include "slib/exception/UnsupportedOperationException.h"

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
private:
	virtual void resize(size_t newSize) = 0;
	virtual ObjRef getRef(size_t index) = 0;
	virtual void setRef(size_t index, ObjRef const& value) = 0;
	virtual bool hasIndex(size_t index) const = 0;
};

template <class T, bool p = std::is_arithmetic<T>::value>
class Array;

template <class T>
class Array<T, true> final : public IArray {
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

private:
	virtual void resize(size_t newSize) override {
		_data.resize(newSize);
	}

	virtual ObjRef getRef(size_t index) override {
		if (index >= _data.size())
			THROW(ArrayIndexOutOfBoundsException, index);
		T *data = _data.data();
		return ObjRef(&data[index], RefType::INSTANCE, classOf<T>::_class());
	}

	virtual void setRef(size_t index, ObjRef const& value) override {
		THROW(UnsupportedOperationException);
	}

	virtual bool hasIndex(size_t index) const override {
		return index < _data.size();
	}
};

template <>
class Array<bool, true> final : public IArray {
public:
	TYPE_INFO(Array, CLASS(Array<bool, false>), INHERITS(IArray));
private:
	bool *_data;
	size_t _len;
public:
	Array(size_t n = 0) {
		if (n == 0) {
			_data = nullptr;
			_len = 0;
		} else {
			_data = (bool *)malloc(n * sizeof(bool));
			if (!_data)
				THROW(OutOfMemoryError);
			_len = n;
		}
	}

	virtual ~Array() {
		free(_data);
	}

	virtual size_t length() const override {
		return _len;
	}

	bool& operator [](size_t i) {
		return _data[i];
	}

	bool *data() {
		return _data;
	}

	bool const* constData() const {
		return _data;
	}

private:
	virtual void resize(size_t newSize) override {
		if (newSize == 0) {
			free(_data);
			_len = 0;
			return;
		}

		bool *newData = (bool *)realloc(_data, newSize * sizeof(bool));
		if (!newData)
			THROW(OutOfMemoryError);

		 memset(newData + _len, 0, (newSize - _len) * sizeof(bool));
		 _data = newData;
		 _len = newSize;
	}

	virtual ObjRef getRef(size_t index) override {
		if (index >= _len)
			THROW(ArrayIndexOutOfBoundsException, index);
		return ObjRef(&_data[index], RefType::INSTANCE, classOf<bool>::_class());
	}

	virtual void setRef(size_t index, ObjRef const& value) override {
		THROW(UnsupportedOperationException);
	}

	virtual bool hasIndex(size_t index) const override {
		return index < _len;
	}
};

template <class T>
class Array<T, false> final : public IArray {
public:
	TYPE_INFO(Array, CLASS(Array<T, false>), INHERITS(IArray));
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

private:
	void resize(size_t newSize) override {
		_data.resize(newSize);
	}

	static void refDeleter(void *ref, RefType refType SLIB_UNUSED) {
		SPtr<T> *elemRef = (SPtr<T> *)ref;
		delete elemRef;
	}

	virtual ObjRef getRef(size_t index) override {
		if (index >= _data.size())
			THROW(ArrayIndexOutOfBoundsException, index);

		return ObjRef(new std::shared_ptr<T>(_data[index]), RefType::SPTR, classOf<T>::_class(), refDeleter);
	}

	virtual void setRef(size_t index, ObjRef const& value) override {
		if (classOf<T>::_class() != value._class)
			THROW(IllegalArgumentException);
		if (index >= _data.size())
			THROW(ArrayIndexOutOfBoundsException, index);

		switch (value._refType) {
			case RefType::SPTR:
				_data[index] = *(SPtr<T>*)value._ref;
				break;
			default:
				THROW(UnsupportedOperationException, "Not yet implemented");
		}
	}

	virtual bool hasIndex(size_t index) const override {
		if (index >= _data.size())
			return false;
		return _data[index].get() != nullptr;
	}
};

} // namespace slib

#endif // H_SLIB_LANG_ARRAY_H
