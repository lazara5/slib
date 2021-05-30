/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_LINKEDLIST_H
#define H_SLIB_COLLECTIONS_LINKEDLIST_H

#include "slib/collections/AbstractList.h"
#include "slib/collections/Deque.h"
#include "slib/exception/IllegalStateException.h"

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
	Node *_head;
	Node *_tail;
public:
	LinkedList()
	: _size(0)
	, _head(nullptr)
	, _tail(nullptr) {}

	virtual ~LinkedList() {
		clear();
	}
private:
	void linkHead(SPtr<E> const& elem) {
		Node *h = _head;
		Node *newNode = new Node(nullptr, elem, h);
		_head = newNode;
		if (!h)
			_tail = newNode;
		else
			h->_prev = newNode;
		_size++;
		_modCount++;
	}

	void linkTail(SPtr<E> const& elem) {
		Node *t = _tail;
		Node *newNode = new Node(t, elem, nullptr);
		_tail = newNode;
		if (!t)
			_head = newNode;
		else
			t->_next = newNode;
		_size++;
		_modCount++;
	}

	void linkBefore(SPtr<E> const& elem, Node *succ) {
		Node *pred = succ->_prev;
		Node *newNode = new Node(pred, elem, succ);
		succ->_prev = newNode;
		if (!pred)
			_head = newNode;
		else
			pred->_next = newNode;
		_size++;
		_modCount++;
	}

	SPtr<E> unlinkHead() {
		Node *head = _head;
		SPtr<E> elem = std::move(head->_item);
		Node *next = head->_next;
		_head = next;

		if (!next)
			_tail = nullptr;
		else
			next->_prev = nullptr;

		delete head;
		_size--;
		_modCount++;
		return elem;
	}

	SPtr<E> unlinkTail() {
		Node *tail = _tail;
		SPtr<E> elem = std::move(tail->_item);
		Node *prev = tail->_prev;
		_tail = prev;

		if (!prev)
			_head = nullptr;
		else
			prev->_next = nullptr;

		delete tail;
		_size--;
		_modCount++;
		return elem;
	}

	SPtr<E> unlink(Node *x) {
		SPtr<E> elem = std::move(x->_item);
		Node *next = x->_next;
		Node *prev = x->_prev;

		if (!prev)
			_head = next;
		else
			prev->_next = next;

		if (!next)
			_tail = prev;
		else
			next->_prev = prev;

		delete x;
		_size--;
		_modCount++;
		return elem;
	}

	/** Returns the (non-null) Node at the specified element index. */
	Node *nodeAt(size_t index) const {
		if (index < (_size >> 1)) {
			Node *node = _head;
			for (size_t i = 0; i < index; i++)
				node = node->_next;
			return node;
		} else {
			Node *node = _tail;
			for (size_t i = _size - 1; i > index; i--)
				node = node->_prev;
			return node;
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

		for (Node *node = _head; node != nullptr; node = node->_next) {
			if (o == (*node->_item))
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
		if (!_head)
			throw NoSuchElementException(_HERE_);
		return _head->_item;
	}

	/**
	 * Returns the last element in this list.
	 *
	 * @return the last element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	SPtr<E> getLast() {
		if (!_tail)
			throw NoSuchElementException(_HERE_);
		return _tail->_item;
	}

	/**
	 * Removes and returns the first element from this list.
	 *
	 * @return the first element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	SPtr<E> removeFirst() {
		if (!_head)
			throw NoSuchElementException(_HERE_);
		return unlinkHead();
	}

	/**
	 * Removes and returns the last element from this list.
	 *
	 * @return the last element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	SPtr<E> removeLast() {
		if (!_tail)
			throw NoSuchElementException(_HERE_);
		return unlinkTail();
	}

	/**
	 * Inserts the specified element at the beginning of this list.
	 *
	 * @param e the element to add
	 */
	void addFirst(SPtr<E> const& e) {
		linkHead(e);
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 */
	void addLast(SPtr<E> const& e) {
		linkTail(e);
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
	 * @return \c true (as specified by {@link Collection#add})
	 */
	bool add(SPtr<E> const& e) override {
		linkTail(e);
		return true;
	}

	/**
	 * Removes the first occurrence of the specified element from this list,
	 * if it is present. If this list does not contain the element, it is
	 * unchanged. Returns \c true if this list
	 * contained the specified element (or equivalently, if this list
	 * changed as a result of the call).
	 *
	 * @param o element to be removed from this list, if present
	 * @return \c true if this list contained the specified element
	 */
	bool remove(const E& o) override {
		for (Node *node = _head; node != nullptr; node = node->_next) {
			if (o == (*node->_item)) {
				unlink(node);
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
		for (Node *node = _head; node != nullptr; ) {
			Node *next = node->_next;
			delete node;
			node = next;
		}
		_head = _tail = nullptr;
		_size = 0;
		_modCount++;
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException
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
			linkTail(element);
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
	 * @return the head of this list, or \c nullptr if this list is empty
	 */
	virtual SPtr<E> peek() override {
		return _head ? _head->_item : nullptr;
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
	 * @return the head of this list, or \c nullptr if this list is empty
	 */
	virtual SPtr<E> poll() override {
		return _head ? unlinkHead() : nullptr;
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
	 * or returns \c nullptr if this list is empty.
	 *
	 * @return the first element of this list, or {@code nullptr}
	 *         if this list is empty
	 */
	virtual SPtr<E> peekFirst() override {
		Node *f = _head;
		return f ? f->_item : nullptr;
	}

	/**
	 * Retrieves, but does not remove, the last element of this list,
	 * or returns {@code nullptr} if this list is empty.
	 *
	 * @return the last element of this list, or \c nullptr
	 *         if this list is empty
	 */
	virtual SPtr<E> peekLast() override {
		return _tail ? _tail->_item : nullptr;
	}

	/**
	 * Retrieves and removes the first element of this list,
	 * or returns \c nullptr if this list is empty.
	 *
	 * @return the first element of this list, or \c nullptr if
	 *         this list is empty
	 */
	virtual SPtr<E> pollFirst() override {
		return _head ? unlinkHead() : nullptr;
	}

	/**
	 * Retrieves and removes the last element of this list,
	 * or returns \c nullptr if this list is empty.
	 *
	 * @return the last element of this list, or \c nullptr if
	 *     this list is empty
	 */
	virtual SPtr<E> pollLast() {
		return _tail ? unlinkTail() : nullptr;
	}

	/**
	 * Pushes an element onto the stack represented by this list, i. e.
	 * inserts the element at the front of this list.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 */
	virtual void push(SPtr<E> const& e) override {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this list, i. e.
	 * removes and returns the first element of this list.
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
	class ConstLinkedListIterator : public ConstListIteratorImpl<SPtr<E>> {
	private:
		const LinkedList *_list;
	protected:
		Node *_lastReturned;
		Node *_next;
		size_t _nextIndex;
		int _expectedModCount;
	public:
		ConstLinkedListIterator(const LinkedList *list, size_t index)
		: _list(list)
		, _lastReturned(nullptr) {
			_next = (index == _list->_size) ? nullptr : _list->nodeAt(index);
			_nextIndex = index;
			_expectedModCount = list->_modCount;
		}
	protected:
		ConstLinkedListIterator(ConstLinkedListIterator *other) {
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
		virtual bool hasNext() override {
			return _nextIndex < _list->_size;
		}

		virtual SPtr<E> const& next() override {
			checkForComodification(_HERE_);
			if (!hasNext())
				throw NoSuchElementException(_HERE_);

			_lastReturned = _next;
			_next = _next->_next;
			_nextIndex++;
			return _lastReturned->_item;
		}

		virtual bool hasPrevious() override {
			return _nextIndex > 0;
		}

		virtual SPtr<E> const& previous() override {
			checkForComodification(_HERE_);
			if (!hasPrevious())
				throw NoSuchElementException(_HERE_);

			_lastReturned = _next = _next ? _next->_prev : _list->_tail;
			_nextIndex--;
			return _lastReturned->_item;
		}

		size_t nextIndex() const override {
			return _nextIndex;
		}

		ssize_t previousIndex() const override {
			return _nextIndex - 1;
		}

		virtual ConstListIteratorImpl<SPtr<E>> *clone() {
			return new ConstLinkedListIterator(this);
		}
	};

	class LinkedListIterator : public ListIteratorImpl<SPtr<E>>, public ConstLinkedListIterator {
	private:
		LinkedList *_ncList;
	protected:
		LinkedListIterator(LinkedListIterator *other)
		: ConstLinkedListIterator(other) {
			_ncList = other->_ncList;
		}
	public:
		LinkedListIterator(LinkedList *list, size_t index)
		: ConstLinkedListIterator(list, index) {
			_ncList = list;
		}

		virtual void remove() override {
			this->checkForComodification(_HERE_);
			if (!this->_lastReturned)
				throw IllegalStateException(_HERE_);

			Node *lastNext = this->_lastReturned->_next;
			_ncList->unlink(this->_lastReturned);
			if (this->_next == this->_lastReturned)
				this->_next = lastNext;
			else
				this->_nextIndex--;
			this->_lastReturned = nullptr;
			this->_expectedModCount++;
		}

		virtual void add(SPtr<E> const& e) override {
			this->checkForComodification(_HERE_);
			this->_lastReturned = nullptr;
			if (!this->_next)
				_ncList->linkTail(e);
			else
				_ncList->linkBefore(e, this->_next);
			this->_nextIndex++;
			this->_expectedModCount++;
		}

		virtual bool hasNext() override {
			return ConstLinkedListIterator::hasNext();
		}

		virtual SPtr<E> const& next() override {
			return ConstLinkedListIterator::next();
		}

		virtual bool hasPrevious() override {
			return ConstLinkedListIterator::hasPrevious();
		}

		virtual SPtr<E> const& previous() override {
			return ConstLinkedListIterator::previous();
		}

		size_t nextIndex() const override {
			return ConstLinkedListIterator::nextIndex();
		}

		ssize_t previousIndex() const override {
			return ConstLinkedListIterator::previousIndex();
		}

		virtual ListIteratorImpl<SPtr<E>> *clone() {
			return new LinkedListIterator(this);
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
	virtual UPtr<ConstIterator<SPtr<E>>> constIterator() const override {
		return newU<ConstIteratorWrapper<SPtr<E>>>(new ConstLinkedListIterator(this, 0));
	}

	virtual UPtr<ListIterator<SPtr<E>>> listIterator() {
		return newU<ListIteratorWrapper<SPtr<E>>>(new LinkedListIterator(this, 0));
	}

	virtual UPtr<ConstListIterator<SPtr<E>>> constListIterator() const {
		return newU<ConstListIteratorWrapper<SPtr<E>>>(new ConstLinkedListIterator(this, 0));
	}
};

} // namespace

#endif // H_SLIB_COLLECTIONS_LINKEDLIST_H
