/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_PRIORITYQUEUE_H
#define H_SLIB_COLLECTIONS_PRIORITYQUEUE_H

#include "slib/collections/AbstractQueue.h"
#include "slib/exception/IllegalArgumentException.h"
#include "slib/exception/NullPointerException.h"

#include "fmt/format.h"

#include <vector>

namespace slib {

template <class E, class Cmp = std::less<E>, class Eq = std::equal_to<E> >
class PriorityQueue : public AbstractQueue<E> {
public:
	TYPE_INFO(PriorityQueue, CLASS(PriorityQueue<E, Cmp, Eq>), INHERITS(AbstractQueue<E>));
private:
	static const int DEFAULT_CAPACITY = 10;

	std::vector<SPtr<E>> _queue;

	/** The number of elements in the priority queue */
	size_t _size;

	volatile int _modCount;
private:
	void percolate(size_t childIndex) {
		Cmp less;
		Eq eq;
		SPtr<E> target = std::move(_queue[childIndex]);
		size_t parentIndex;
		while (childIndex > 0) {
			parentIndex = (childIndex - 1) / 2;
			if (less(*target, *_queue[parentIndex]) || eq(*target, *_queue[parentIndex]))
				break;
			_queue[childIndex] = std::move(_queue[parentIndex]);
			childIndex = parentIndex;
		}
		_queue[childIndex] = std::move(target);
	}

	void siftDown(size_t rootIndex) {
		Cmp less;
		Eq eq;
		SPtr<E> target = _queue[rootIndex];
		size_t childIndex;
		while ((childIndex = rootIndex * 2 + 1) < _size) {
			if (childIndex + 1 < _size && less(*_queue[childIndex + 1], *_queue[childIndex]))
				childIndex++;
			if (less(*target, *_queue[childIndex]) || eq(*target, *_queue[childIndex]))
				break;

			_queue[rootIndex] = std::move(_queue[childIndex]);
			rootIndex = childIndex;
		}
		_queue[rootIndex] = std::move(target);
	}

	void removeAt(size_t index) {
		_size--;
		_queue[index] = std::move(_queue[_size]);
		siftDown(index);
	}

	void grow(size_t size) {
		if (size > _queue.capacity())
			_queue.reserve(size);
	}
public:
	PriorityQueue(int initialCapacity)
	:_size(0)
	,_modCount(0) {
		if (initialCapacity < 0)
			throw IllegalArgumentException(_HERE_, fmt::format("Illegal capacity: {:d}", initialCapacity).c_str());
		_queue.reserve(initialCapacity);
	}

	virtual size_t size() const override {
		return _size;
	}

	using AbstractQueue<E>::isEmpty;

	using Collection<E>::add;

	virtual bool offer(SPtr<E> const& e) override {
		if (!e)
			throw NullPointerException(_HERE_);

		_modCount++;
		grow(_size + 1);
		_queue.push_back(e);
		percolate(_size++);
		return true;
	}

	SPtr<E> poll() override {
		if (isEmpty())
			return nullptr;
		SPtr<E> result = _queue[0];
		removeAt(0);
		return result;
	}

	virtual bool remove(const E& o) override {
		if (_size == 0)
			return false;

		Eq eq;
		for (size_t i = 0; i < _size; i++) {
			if (eq(o, *_queue[i])) {
				removeAt(i);
				return true;
			}
		}
		return false;
	}

private:

	// iterator
	class ConstPriorityIterator : public ConstIterator<SPtr<E> >::ConstIteratorImpl {
	private:
		const PriorityQueue<E> *_pq;
	protected:
		ssize_t _crtIndex;
		int _expectedModCount;
	protected:
		ConstPriorityIterator(ConstPriorityIterator *other) {
			_pq = other->_pq;
			_crtIndex = other->_crtIndex;
			_expectedModCount = other->_expectedModCount;
		}

		void checkForComodification(const char *where) {
			if (_pq->_modCount != _expectedModCount)
				throw ConcurrentModificationException(where);
		}
	public:
		ConstPriorityIterator(const PriorityQueue *pq) {
			_pq = pq;
			_crtIndex = -1;
			_expectedModCount = _pq->_modCount;
		}

		virtual bool hasNext() {
			return _crtIndex < _pq->size() - 1;
		}

		virtual SPtr<E> const& next() {
			checkForComodification(_HERE_);
			if (!hasNext())
				throw NoSuchElementException(_HERE_);
			return _pq->_queue[++_crtIndex];
		}

		virtual typename ConstIterator<SPtr<E> >::ConstIteratorImpl *clone() {
			return new ConstPriorityIterator(this);
		}
	};

public:

	virtual ConstIterator<SPtr<E>> constIterator() const {
		return ConstIterator<SPtr<E>>(new ConstPriorityIterator(this));
	}
};

} // namespace slib

#endif // H_SLIB_COLLECTIONS_PRIORITYQUEUE_H
