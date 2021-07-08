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
public:
	TYPE_INFO(ArrayList, CLASS(ArrayList<E>), INHERITS(AbstractList<E>));
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
	ArrayList(int initialCapacity, bool init = false)
	try: _elements() {
		if (initialCapacity < 0)
			throw IllegalArgumentException(_HERE_, fmt::format("Illegal capacity: {:d}", initialCapacity).c_str());
		_elements.reserve(initialCapacity);
		if (init)
			_elements.resize(initialCapacity);
	} catch (std::bad_alloc const &) {
		throw OutOfMemoryError(_HERE_);
	}

	ArrayList(bool init = false)
	: ArrayList(DEFAULT_CAPACITY, init) {}

	virtual void clear() override {
		_modCount++;

		_elements.clear();
	}

	virtual ~ArrayList() {
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

	virtual ssize_t indexOf(const E& o) override {
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

	SPtr<E> operator [](size_t index) const {
		if (index >= size())
			throw ArrayIndexOutOfBoundsException(_HERE_, index);
		return _elements[index];
	}

	SPtr<E> remove(int index) {
		accessRangeCheck(index);
		return internalRemove(index);
	}

private:
	// iterator
	class ConstArrayListIterator : public ConstListIteratorImpl<SPtr<E>> {
	private:
		const ArrayList *_list;
	protected:
		size_t _cursor;
		ptrdiff_t _lastRet;
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
		ConstArrayListIterator(const ArrayList *list, size_t index)
		: _list(list) {
			_cursor = index;
			_lastRet = -1;
			_expectedModCount = _list->_modCount;
		}

		virtual bool hasNext() override {
			return _cursor != _list->size();
		}

		virtual SPtr<E> const& next() override {
			checkForComodification(_HERE_);
			size_t i = _cursor;
			if (i >= _list->size())
				throw NoSuchElementException(_HERE_);
			_cursor = i + 1;
			_lastRet = i;
			return _list->_elements[i];
		}

		virtual bool hasPrevious() override {
			return _cursor != 0;
		}

		virtual SPtr<E> const& previous() override {
			checkForComodification(_HERE_);
			ptrdiff_t i = _cursor - 1;
			if (i < 0)
				throw NoSuchElementException(_HERE_);
			if ((size_t)i >= _list->size())
				throw NoSuchElementException(_HERE_);
			_cursor = i;
			_lastRet = i;
			return _list->_elements[i];
		}

		size_t nextIndex() const override {
			return _cursor;
		}

		ssize_t previousIndex() const override {
			return _cursor - 1;
		}

		virtual ConstListIteratorImpl<SPtr<E>> *clone() override {
			return new ConstArrayListIterator(this);
		}
	};

	class ArrayListIterator : public ListIteratorImpl<SPtr<E>>, public ConstArrayListIterator {
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

		virtual bool hasNext() override {
			return ConstArrayListIterator::hasNext();
		}

		virtual const SPtr<E>& next() override {
			return ConstArrayListIterator::next();
		}

		virtual void add(SPtr<E> const& e) override {
			this->checkForComodification(_HERE_);

			try {
				size_t i = this->_cursor;
				_ncList->add(i, e);
				this->_cursor = i + 1;
				this->_lastRet = -1;
				this->_expectedModCount = _ncList->_modCount;
			} catch (IndexOutOfBoundsException const&) {
				throw ConcurrentModificationException(_HERE_);
			}
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

		virtual ListIteratorImpl<SPtr<E>> *clone() override {
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
	virtual UPtr<ConstIterator<SPtr<E>>> constIterator() const {
		return newU<ConstIteratorWrapper<SPtr<E>>>(new ConstArrayListIterator(this, 0));
	}

	virtual UPtr<Iterator<SPtr<E>>> iterator() {
		return newU<IteratorWrapper<SPtr<E>>>(new ArrayListIterator(this, 0));
	}

	virtual UPtr<ConstListIterator<SPtr<E>>> constListIterator() const {
		return newU<ConstListIteratorWrapper<SPtr<E>>>(new ConstArrayListIterator(this, 0));
	}
};

} // namespace

#endif // H_SLIB_COLLECTIONS_ARRAYLIST_H
