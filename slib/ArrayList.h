/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_ARRAYLIST_H__
#define __SLIB_ARRAYLIST_H__

#include "slib/AbstractList.h"
#include "slib/exception/IllegalStateException.h"
#include "slib/exception/IllegalArgumentException.h"
#include "slib/Numeric.h"

#include "fmt/format.h"

namespace slib {

template <class E>
class ArrayList : public AbstractList<E> {
using AbstractList<E>::_modCount;
public:
	static const int DEFAULT_CAPACITY = 10;
private:
	E **_elementData;
	int _arraySize;
	int _size;
private:
	static const int MAX_ARRAY_SIZE = Integer::MAX_VALUE - 1;

	/**
	 * Increases the capacity to ensure that we can hold at least the
	 * number of elements specified by the minimum capacity argument.
	 *
	 * @param minCapacity the desired minimum capacity
	 */
	void grow(int minCapacity) {
		int crtCapacity = _arraySize;
		int newCapacity = crtCapacity + (crtCapacity >> 1);
		if (newCapacity - minCapacity < 0)
			newCapacity = minCapacity;
		if (newCapacity - MAX_ARRAY_SIZE > 0)
			newCapacity = MAX_ARRAY_SIZE;

		E **newArray = (E**)malloc(newCapacity * sizeof(E*));
        if (!newArray)
            throw OutOfMemoryError(_HERE_);
		memcpy(newArray, _elementData, crtCapacity * sizeof(E*));
		free(_elementData);
		_elementData = newArray;
		_arraySize = newCapacity;
	}

	void ensureCapacity(int minCapacity) {
		_modCount++;
		
		if (_elementData == nullptr)
			minCapacity = (DEFAULT_CAPACITY > minCapacity) ? DEFAULT_CAPACITY : minCapacity;

		if (minCapacity - _arraySize > 0)
			grow(minCapacity);
	}

	/** Checks if the given index is in range. If not, throws IndexOutOfBoundsException */
	void accessRangeCheck(int index) const {
		if (index < 0 || index >= _size)
			throw IndexOutOfBoundsException(_HERE_, fmt::format("Index: {:d}, Size: {:d}", index, _size).c_str());
	}

	/** Special version of rangeCheck used by add() */
	void addRangeCheck(int index) const {
		if (index > _size || index < 0)
			throw IndexOutOfBoundsException(_HERE_, fmt::format("Index: {:d}, Size: {:d}", index, _size).c_str());
	}

	/**
	 * Skips bounds checking and does not
	 * return the value removed.
	 */
	void internalRemove(int index) {
		_modCount++;
		delete _elementData[index];
		int numMoved = _size - index - 1;
		if (numMoved > 0) {
			memmove(_elementData + index, _elementData + index + 1, numMoved * sizeof(E*));
		}
		_elementData[--_size] = NULL; 
	}
public:
	ArrayList(int initialCapacity) {
		if (initialCapacity < 0)
			throw IllegalArgumentException(_HERE_, fmt::format("Illegal Capacity: {:d}", initialCapacity).c_str());
		_elementData = (E**)malloc(initialCapacity * sizeof(E*));
		if (!_elementData)
			throw OutOfMemoryError(_HERE_);
		_arraySize = initialCapacity;
		_size = 0;
	}

	ArrayList() {
		_elementData = NULL;
		_arraySize = 0;
		_size = 0;
	}

	virtual void clear() {
		_modCount++;

		for (int i = 0; i <_size; i++) {
			E *e = _elementData[i];
			if (e != NULL)
				delete e;
			_elementData[i] = NULL;
		}
		_size = 0;
	}

	virtual ~ArrayList() {
		if (_elementData != NULL) {
			clear();
			free(_elementData);
		}
		_elementData = NULL;
	}

	/**
     * Returns the number of elements in this list.
     * @return the number of elements in this list
     */
	int size() const {
		return _size;
	}

	bool isEmpty() const {
		return _size == 0;
	}

	/**
     * Appends the specified element to the end of this list.
     *
     * @param e element to be appended to this list
     * @return <i>true</i>
     */
	bool add(const E& e) {
		ensureCapacity(_size + 1); // Increments _modCount!
		E *elem = new E(e);
		_elementData[_size++] = elem;
		return true;
	}
	
	void add(int index, const E& e) {
		addRangeCheck(index);
		
		ensureCapacity(_size + 1); // Increments _modCount!
		memmove(_elementData + index + 1, _elementData + index, (_size - index) * sizeof(E*));
		E *elem = new E(e);
		_elementData[index] = elem;
		_size++;
	}

	bool remove(const E& o) {
		for (int index = 0; index < _size; index++) {
			if (o == (*_elementData[index])) {
				internalRemove(index);
				return true;
			}
		}

		return false;
	}
	
	int indexOf(const E& o) {
		for (int index = 0; index < _size; index++) {
			if (o == (*_elementData[index]))
				return index;
		}

		return -1;
	}

	// Positional Access Operations

	E& get(int index) const {
		accessRangeCheck(index);
		return *_elementData[index];
	}

	E remove(int index) {
		accessRangeCheck(index);

		E oldValue = *_elementData[index];
		internalRemove(index);

		return oldValue;
	}

private:
	// iterator
	class ConstArrayListIterator : public ConstIterator<E>::ConstIteratorImpl {
	private:
		const ArrayList *_list;
	protected:
		int _cursor;
		int _lastRet;
		int _expectedModCount;
	protected:
		ConstArrayListIterator(ConstArrayListIterator *other) {
			_list = other->_list;
			_cursor = other->_cursor;
			_lastRet = other->_lastRet;
			_expectedModCount = other->_expectedModCount;
		}

		void checkForComodification(const char *where) {
			if (_list->_modCount != _expectedModCount)
				throw ConcurrentModificationException(where);
		}
	public:
		ConstArrayListIterator(const ArrayList *list, int index) {
			_list = list;
			_cursor = index;
			_lastRet = -1;
			_expectedModCount = _list->_modCount;
		}

		virtual bool hasNext() {
			return _cursor != _list->_size;
		}

		virtual const E& next() {
			checkForComodification(_HERE_);
			int i = _cursor;
			if (i >= _list->_size)
				throw NoSuchElementException(_HERE_);
			if (i >= _list->_arraySize)
				throw ConcurrentModificationException(_HERE_);
			_cursor = i + 1;
			return *(_list->_elementData[_lastRet = i]);
		}

		virtual typename ConstIterator<E>::ConstIteratorImpl *clone() {
			return new ConstArrayListIterator(this);
		}
	};

	class ArrayListIterator : public Iterator<E>::IteratorImpl, public ConstArrayListIterator {
	private:
		ArrayList *_ncList;
	protected:
		ArrayListIterator(ArrayListIterator *other) 
		: ConstArrayListIterator(other) {
			_ncList = other->_ncList;
		}
	public:
		ArrayListIterator(ArrayList *list, int index)
		:ConstArrayListIterator(list, index) {
			_ncList = list;
		}

		virtual bool hasNext() {
			return ConstArrayListIterator::hasNext();
		}

		virtual const E& next() {
			return ConstArrayListIterator::next();
		}

		virtual void remove() {
			if (this->_lastRet < 0)
				throw IllegalStateException(_HERE_);
			this->checkForComodification(_HERE_);

			try {
				_ncList->remove(this->_lastRet);
				this->_cursor = this->_lastRet;
				this->_lastRet = -1;
				this->_expectedModCount = _ncList->_modCount;
			} catch (IndexOutOfBoundsException const&) {
				throw ConcurrentModificationException(_HERE_);
			}
		}

		virtual typename Iterator<E>::IteratorImpl *clone() {
			return new ArrayListIterator(this);
		}
	};

public:
	/**
     * Returns an iterator over the elements in this list in proper sequence.
     *
     * <p>The returned iterator is <i>fail-fast</i>.
     *
     * @return an iterator over the elements in this list in proper sequence
     */
	virtual ConstIterator<E> constIterator() const {
		return ConstIterator<E>(new ConstArrayListIterator(this, 0));
	}

	virtual Iterator<E> iterator() {
		return Iterator<E>(new ArrayListIterator(this, 0));
	}
};

} // namespace

#endif
