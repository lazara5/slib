/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_ARRAYLIST_H
#define H_SLIB_COLLECTIONS_ARRAYLIST_H

#include "slib/collections/AbstractList.h"
#include "slib/exception/IllegalStateException.h"
#include "slib/exception/IllegalArgumentException.h"
#include "slib/lang/Numeric.h"

#include "fmt/format.h"

#include <vector>

namespace slib {

template <class E>
class ArrayList : public AbstractList<E> {
using AbstractList<E>::_modCount;
private:
	static const int DEFAULT_CAPACITY = 10;

	std::vector<SPtr<E>> _elements;

	static const int MAX_ARRAY_SIZE = Integer::MAX_VALUE - 1;

	/** Checks if the given index is in range. If not, throws IndexOutOfBoundsException */
	void accessRangeCheck(size_t index) const {
		if (index >= size())
			throw IndexOutOfBoundsException(_HERE_, fmt::format("Index: {:d}, Size: {:d}", index, size()).c_str());
	}

	/** Special version of rangeCheck used by add() */
	void addRangeCheck(size_t index) const {
		if (index > size())
			throw IndexOutOfBoundsException(_HERE_, fmt::format("Index: {:d}, Size: {:d}", index, size()).c_str());
	}

	/** Skips bounds checking */
	SPtr<E> internalRemove(int index) {
		_modCount++;
		SPtr<E> removed = _elements[index];
		_elements.erase(_elements.begin() + index);
		return removed;
	}
public:
	ArrayList(int initialCapacity)
	try: _elements() {
		if (initialCapacity < 0)
			throw IllegalArgumentException(_HERE_, fmt::format("Illegal capacity: {:d}", initialCapacity).c_str());
		_elements.reserve(initialCapacity);
	} catch (std::bad_alloc const &) {
		throw OutOfMemoryError(_HERE_);
	}

	ArrayList()
	: ArrayList(DEFAULT_CAPACITY) {}

	virtual void clear() override {
		_modCount++;

		_elements.clear();
	}

	virtual ~ArrayList() {
	}

	virtual Class const* getClass() const override {
		return ARRAYLISTCLASS();
	}

	/**
	 * Returns the number of elements in this list.
	 * @return the number of elements in this list
	 */
	size_t size() const override {
		return _elements.size();
	}

	bool isEmpty() const {
		return _elements.empty();
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * @param e element to be appended to this list
	 */
	virtual bool add(SPtr<E> const& e) override {
		_modCount++;
		try {
			_elements.push_back(e);
			return true;
		} catch (std::bad_alloc const &) {
			throw OutOfMemoryError(_HERE_);
		}
	}

	using Collection<E>::add;

	void add(int index, SPtr<E> const& e) override {
		addRangeCheck(index);
		_modCount++;
		try {
			_elements.insert(_elements.begin() + index, e);
		} catch (std::bad_alloc const &) {
			throw OutOfMemoryError(_HERE_);
		}
	}

	bool remove(const E& o) override {
		for (size_t index = 0; index < size(); index++) {
			if (_elements[index] && (o == (*_elements[index]))) {
				internalRemove(index);
				return true;
			}
		}

		return false;
	}

	int indexOf(const E& o) {
		for (size_t index = 0; index < size(); index++) {
			if (_elements[index] && (o == (*_elements[index])))
				return index;
		}

		return -1;
	}

	// Positional Access Operations

	virtual SPtr<E> get(size_t index) const override {
		accessRangeCheck(index);
		return _elements[index];
	}

	SPtr<E> remove(int index) {
		accessRangeCheck(index);
		return internalRemove(index);
	}

private:
	// iterator
	class ConstArrayListIterator : public ConstIterator<SPtr<E>>::ConstIteratorImpl {
	private:
		const ArrayList *_list;
	protected:
		size_t _cursor;
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
		ConstArrayListIterator(const ArrayList *list, size_t index) {
			_list = list;
			_cursor = index;
			_lastRet = -1;
			_expectedModCount = _list->_modCount;
		}

		virtual bool hasNext() {
			return _cursor != _list->size();
		}

		virtual SPtr<E> const& next() {
			checkForComodification(_HERE_);
			size_t i = _cursor;
			if (i >= _list->size())
				throw NoSuchElementException(_HERE_);
			_cursor = i + 1;
			return _list->_elements[_lastRet = i];
		}

		virtual typename ConstIterator<SPtr<E>>::ConstIteratorImpl *clone() {
			return new ConstArrayListIterator(this);
		}
	};

	class ArrayListIterator : public Iterator<SPtr<E>>::IteratorImpl, public ConstArrayListIterator {
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

		virtual const std::shared_ptr<E>& next() {
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

		virtual typename Iterator<SPtr<E>>::IteratorImpl *clone() {
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
	virtual ConstIterator<SPtr<E>> constIterator() const {
		return ConstIterator<SPtr<E>>(new ConstArrayListIterator(this, 0));
	}

	virtual Iterator<SPtr<E>> iterator() {
		return Iterator<SPtr<E>>(new ArrayListIterator(this, 0));
	}
};

} // namespace

#endif // H_SLIB_COLLECTIONS_ARRAYLIST_H
