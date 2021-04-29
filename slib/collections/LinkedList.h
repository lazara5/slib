/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_LINKEDLIST_H
#define H_SLIB_COLLECTIONS_LINKEDLIST_H

#include "slib/collections/AbstractSequentialList.h"
#include "slib/collections/Deque.h"

#include "fmt/format.h"

namespace slib {

template <class E>
class LinkedList : public AbstractList<E>, public Deque<E> {
using AbstractList<E>::_modCount;
public:
	TYPE_INFO(LinkedList, CLASS(LinkedList<E>), INHERITS(AbstractList<E>), INHERITS(Deque<E>));
private:
	class Node {
	public:
		SPtr<E> _item;
		Node *_next;
		Node *_prev;
	public:
		Node(Node *prev, SPtr<E> const& elem, Node *next)
		: _item(elem)
		, _next(next)
		, _prev(prev) {}
	};

	size_t _size;
	Node *_first;
	Node *_last;
public:
	LinkedList()
	: _size(0)
	, _first(nullptr)
	, _last(nullptr) {}
private:
	void linkFirst(SPtr<E> const& e) {
		Node *f = _first;
		Node *newNode = new Node(nullptr, e, f);
		_first = newNode;
		if (!f)
			_last = newNode;
		else
			f->_prev = newNode;
		_size++;
		_modCount++;
	}

	void linkLast(SPtr<E> const& e) {
		Node *l = _last;
		Node *newNode = new Node(l, e, nullptr);
		_last = newNode;
		if (!l)
			_first = newNode;
		else
			l->_next = newNode;
		_size++;
		_modCount++;
	}

	void linkBefore(SPtr<E> const& e, Node *succ) {
		Node *pred = succ->_prev;
		Node *newNode = new Node(pred, e, succ);
		succ->prev = newNode;
		if (!pred)
			_first = newNode;
		else
			pred->_next = newNode;
		_size++;
		_modCount++;
	}

	SPtr<E> unlinkFirst(Node *f) {
		SPtr<E> elem = std::move(f->_item);
		Node *next = f->_next;
		_first = next;

		if (!next)
			_last = nullptr;
		else
			next->_prev = nullptr;

		delete f;
		_size--;
		_modCount++;
		return elem;
	}

	SPtr<E> unlinkLast(Node *l) {
		SPtr<E> elem = std::move(l->_item);
		Node *prev = l->_prev;
		_last = prev;

		if (!prev)
			_first = nullptr;
		else
			prev->_next = nullptr;

		delete l;
		_size--;
		_modCount++;
		return elem;
	}

	SPtr<E> unlink(Node *x) {
		SPtr<E> elem = std::move(x->_item);
		Node *next = x->_next;
		Node *prev = x->_prev;

		if (!prev)
			_first = next;
		else
			prev->_next = next;

		if (!next)
			_last = prev;
		else
			next->_prev = prev;

		delete x;
		_size--;
		_modCount++;
		return elem;
	}

	/**
	 * Returns the (non-null) Node at the specified element index.
	 */
	Node *nodeAt(size_t index) const {
		if (index < (_size >> 1)) {
			Node *x = _first;
			for (size_t i = 0; i < index; i++)
				x = x->_next;
			return x;
		} else {
			Node *x = _last;
			for (size_t i = _size - 1; i > index; i--)
				x = x->_prev;
			return x;
		}
	}

	/** Checks if the argument is the index of an existing element. */
	bool isElementIndex(size_t index) const {
		return index >= 0 && index < _size;
	}

	void checkElementIndex(size_t index) const {
		if (!isElementIndex(index))
			throw IndexOutOfBoundsException(_HERE_, fmt::format("Index: {:d}, Size: {:d}", index, size()).c_str());
	}

	/**
	 * Checks if the argument is the index of a valid position for an
	 * iterator or an add operation.
	 */
	bool isPositionIndex(size_t index) {
		return index >= 0 && index <= _size;
	}

	void checkPositionIndex(size_t index) {
		if (!isPositionIndex(index))
			throw IndexOutOfBoundsException(_HERE_, fmt::format("Index: {:d}, Size: {:d}", index, size()).c_str());
	}

public:
	virtual ssize_t indexOf(const E& o) {
		ssize_t index = 0;

		for (Node *x = _first; x != nullptr; x = x->_next) {
			if (o == (*x->_item))
				return index;
			index++;
		}

		return -1;
	}

	/**
	 * Returns the first element in this list.
	 *
	 * @return the first element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	SPtr<E> getFirst() {
		Node *f = _first;
		if (!f)
			throw NoSuchElementException(_HERE_);
		return f->_item;
	}

	/**
	 * Returns the last element in this list.
	 *
	 * @return the last element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	SPtr<E> getLast() {
		Node *l = _last;
		if (!l)
			throw NoSuchElementException(_HERE_);
		return l->_item;
	}

	/**
	 * Removes and returns the first element from this list.
	 *
	 * @return the first element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	SPtr<E> removeFirst() {
		Node *f = _first;
		if (!f)
			throw NoSuchElementException(_HERE_);
		return unlinkFirst(f);
	}

	/**
	 * Removes and returns the last element from this list.
	 *
	 * @return the last element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	SPtr<E> removeLast() {
		Node *l = _last;
		if (!l)
			throw NoSuchElementException(_HERE_);
		return unlinkLast(l);
	}

	/**
	 * Inserts the specified element at the beginning of this list.
	 *
	 * @param e the element to add
	 */
	void addFirst(SPtr<E> const& e) {
		linkFirst(e);
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 */
	void addLast(SPtr<E> const& e) {
		linkLast(e);
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	virtual size_t size() const override {
		return _size;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #addLast}.
	 *
	 * @param e element to be appended to this list
	 * @return {@code true} (as specified by {@link Collection#add})
	 */
	bool add(SPtr<E> const& e) override {
		linkLast(e);
		return true;
	}

	/**
	 * Removes the first occurrence of the specified element from this list,
	 * if it is present.  If this list does not contain the element, it is
	 * unchanged.  More formally, removes the element with the lowest index
	 * {@code i} such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>
	 * (if such an element exists).  Returns {@code true} if this list
	 * contained the specified element (or equivalently, if this list
	 * changed as a result of the call).
	 *
	 * @param o element to be removed from this list, if present
	 * @return {@code true} if this list contained the specified element
	 */
	bool remove(const E& o) override {
		for (Node *x = _first; x != nullptr; x = x->_next) {
			if (o == (*x->_item)) {
				unlink(x);
				return true;
			}
		}
		return false;
	}

	/**
	 * Removes all of the elements from this list.
	 * The list will be empty after this call returns.
	 */
	void clear() override {
		for (Node *x = _first; x != nullptr; ) {
			Node *next = x->_next;
			delete x;
			x = next;
		}
		_first = _last = nullptr;
		_size = 0;
		_modCount++;
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual SPtr<E> get(size_t index) const override {
		checkElementIndex(index);
		return nodeAt(index)->_item;
	}

	/**
	 * Inserts the specified element at the specified position in this list.
	 * Shifts the element currently at that position (if any) and any
	 * subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	void add(size_t index, SPtr<E> const& element) {
		checkPositionIndex(index);

		if (index == _size)
			linkLast(element);
		else
			linkBefore(element, nodeAt(index));
	}

	/**
	 * Removes the element at the specified position in this list.  Shifts any
	 * subsequent elements to the left (subtracts one from their indices).
	 * Returns the element that was removed from the list.
	 *
	 * @param index the index of the element to be removed
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	SPtr<E> remove(size_t index) {
		checkElementIndex(index);
		return unlink(nodeAt(index));
	}

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 *
	 * @return the head of this list, or {@code nullptr} if this list is empty
	 */
	virtual SPtr<E> peek() override {
		Node *f = _first;
		return f ? f->_item : nullptr;
	}

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 *
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual SPtr<E> element() override {
		return getFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list.
	 *
	 * @return the head of this list, or {@code nullptr} if this list is empty
	 */
	virtual SPtr<E> poll() override {
		Node *f = _first;
		return f ? unlinkFirst(f) : nullptr;
	}

	/**
	 * Retrieves and removes the head (first element) of this list.
	 *
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual SPtr<E> remove() override {
		return removeFirst();
	}

	/**
	 * Adds the specified element as the tail (last element) of this list.
	 *
	 * @param e the element to add
	 * @return {@code true} (as specified by {@link Deqeue#offer})
	 */
	virtual bool offer(SPtr<E> const& e) override {
		return add(e);
	}

	/**
	 * Inserts the specified element at the front of this list.
	 *
	 * @param e the element to insert
	 * @return {@code true} (as specified by {@link Deque#offerFirst})
	 */
	virtual bool offerFirst(SPtr<E> const& e) override {
		addFirst(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this list.
	 *
	 * @param e the element to insert
	 * @return {@code true} (as specified by {@link Deque#offerLast})
	 */
	virtual bool offerLast(SPtr<E> const& e) override {
		addLast(e);
		return true;
	}

	/**
	 * Retrieves, but does not remove, the first element of this list,
	 * or returns {@code nullptr} if this list is empty.
	 *
	 * @return the first element of this list, or {@code nullptr}
	 *         if this list is empty
	 */
	virtual SPtr<E> peekFirst() override {
		Node *f = _first;
		return f ? f->_item : nullptr;
	}

	/**
	 * Retrieves, but does not remove, the last element of this list,
	 * or returns {@code nullptr} if this list is empty.
	 *
	 * @return the last element of this list, or {@code nullptr}
	 *         if this list is empty
	 */
	virtual SPtr<E> peekLast() override {
		Node *l = _last;
		return l ? l->_item : nullptr;
	}

	/**
	 * Retrieves and removes the first element of this list,
	 * or returns {@code nullptr} if this list is empty.
	 *
	 * @return the first element of this list, or {@code nullptr} if
	 *         this list is empty
	 */
	virtual SPtr<E> pollFirst() override {
		Node *f = _first;
		return f ? unlinkFirst(f) : nullptr;
	}

	/**
	 * Retrieves and removes the last element of this list,
	 * or returns {@code nullptr} if this list is empty.
	 *
	 * @return the last element of this list, or {@code nullptr} if
	 *     this list is empty
	 */
	virtual SPtr<E> pollLast() {
		Node *l = _last;
		return l ? unlinkLast(l) : nullptr;
	}

	/**
	 * Pushes an element onto the stack represented by this list.  In other
	 * words, inserts the element at the front of this list.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 */
	virtual void push(SPtr<E> const& e) override {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this list.  In other
	 * words, removes and returns the first element of this list.
	 *
	 * <p>This method is equivalent to {@link #removeFirst()}.
	 *
	 * @return the element at the front of this list (which is the top
	 *         of the stack represented by this list)
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual SPtr<E> pop() override {
		return removeFirst();
	}

private:
	class ConstListItr : public ConstListIterator<SPtr<E>>::ConstListIteratorImpl {
	private:
		const LinkedList *_list;
	protected:
		Node *_lastReturned;
		Node *_next;
		size_t _nextIndex;
		int _expectedModCount;
	public:
		ConstListItr(const LinkedList *list, size_t index)
		: _list(list)
		, _lastReturned(nullptr) {
			_next = (index == _list->_size) ? nullptr : _list->nodeAt(index);
			_nextIndex = index;
			_expectedModCount = list->_modCount;
		}
	protected:
		ConstListItr(ConstListItr *other) {
			_lastReturned = other->_lastReturned;
			_next = other->_next;
			_nextIndex = other->_nextIndex;
			_expectedModCount = other->_expectedModCount;
		}

		void checkForComodification(const char *where) {
			if (_list->_modCount != _expectedModCount)
				throw ConcurrentModificationException(where);
		}
	public:
		virtual bool hasNext() const override {
			return _nextIndex < _list->_size;
		}

		virtual SPtr<E> const& next() {
			checkForComodification(_HERE_);
			if (!hasNext())
				throw NoSuchElementException(_HERE_);

			_lastReturned = _next;
			_next = _next->_next;
			_nextIndex++;
			return _lastReturned->_item;
		}

		virtual bool hasPrevious() const override {
			return _nextIndex > 0;
		}

		virtual SPtr<E> const& previous() override {
			checkForComodification(_HERE_);
			if (!hasPrevious())
				throw NoSuchElementException(_HERE_);

			_lastReturned = _next = _next ? _next->_prev : _list->_last;
			_nextIndex--;
			return _lastReturned->_item;
		}

		size_t nextIndex() const override {
			return _nextIndex;
		}

		ssize_t previousIndex() const override {
			return _nextIndex - 1;
		}

		/*void remove() {
			checkForComodification(_HERE_);
			if (lastReturned == null)
				throw IllegalStateException(_HERE_);

			Node<E> lastNext = lastReturned.next;
			unlink(lastReturned);
			if (next == lastReturned)
				next = lastNext;
			else
				nextIndex--;
			lastReturned = null;
			expectedModCount++;
		}


		public void add(E e) {
			checkForComodification(_HERE_);
			_lastReturned = nullptr;
			if (!next)
				linkLast(e);
			else
				linkBefore(e, next);
			_nextIndex++;
			_expectedModCount++;
		}*/

		virtual typename ConstListIterator<SPtr<E>>::ConstListIteratorImpl *clone() {
			return new ConstListItr(this);
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
		return ConstIterator<SPtr<E>>(new ConstListItr(this, 0));
	}
};

} // namespace

#endif // H_SLIB_COLLECTIONS_LINKEDLIST_H
