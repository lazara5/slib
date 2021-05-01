/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_DEQUE_H
#define H_SLIB_COLLECTIONS_DEQUE_H


#include "slib/collections/Collection.h"

namespace slib {

template <class E>
class Deque : virtual public Collection<E> {
public:
	/**
		 * Inserts the specified element at the front of this deque if it is
		 * possible to do so immediately without violating capacity restrictions.
		 * When using a capacity-restricted deque, it is generally preferable to
		 * use method {@link #offerFirst}.
		 *
		 * @param e the element to add
		 * @throws IllegalStateException if the element cannot be added at this
		 *         time due to capacity restrictions
		 * @throws ClassCastException if the class of the specified element
		 *         prevents it from being added to this deque
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * @throws IllegalArgumentException if some property of the specified
		 *         element prevents it from being added to this deque
		 */
		virtual void addFirst(SPtr<E> const& e) = 0;

		/**
		 * Inserts the specified element at the end of this deque if it is
		 * possible to do so immediately without violating capacity restrictions.
		 * When using a capacity-restricted deque, it is generally preferable to
		 * use method {@link #offerLast}.
		 *
		 * <p>This method is equivalent to {@link #add}.
		 *
		 * @param e the element to add
		 * @throws IllegalStateException if the element cannot be added at this
		 *         time due to capacity restrictions
		 * @throws ClassCastException if the class of the specified element
		 *         prevents it from being added to this deque
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * @throws IllegalArgumentException if some property of the specified
		 *         element prevents it from being added to this deque
		 */
		virtual void addLast(SPtr<E> const& e) = 0;

		/**
		 * Inserts the specified element at the front of this deque unless it would
		 * violate capacity restrictions.  When using a capacity-restricted deque,
		 * this method is generally preferable to the {@link #addFirst} method,
		 * which can fail to insert an element only by throwing an exception.
		 *
		 * @param e the element to add
		 * @return <tt>true</tt> if the element was added to this deque, else
		 *         <tt>false</tt>
		 * @throws ClassCastException if the class of the specified element
		 *         prevents it from being added to this deque
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * @throws IllegalArgumentException if some property of the specified
		 *         element prevents it from being added to this deque
		 */
		virtual bool offerFirst(SPtr<E> const& e) = 0;

		/**
		 * Inserts the specified element at the end of this deque unless it would
		 * violate capacity restrictions.  When using a capacity-restricted deque,
		 * this method is generally preferable to the {@link #addLast} method,
		 * which can fail to insert an element only by throwing an exception.
		 *
		 * @param e the element to add
		 * @return <tt>true</tt> if the element was added to this deque, else
		 *         <tt>false</tt>
		 * @throws ClassCastException if the class of the specified element
		 *         prevents it from being added to this deque
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * @throws IllegalArgumentException if some property of the specified
		 *         element prevents it from being added to this deque
		 */
		virtual bool offerLast(SPtr<E> const& e) = 0;

		/**
		 * Retrieves and removes the first element of this deque.  This method
		 * differs from {@link #pollFirst pollFirst} only in that it throws an
		 * exception if this deque is empty.
		 *
		 * @return the head of this deque
		 * @throws NoSuchElementException if this deque is empty
		 */
		virtual SPtr<E> removeFirst() = 0;

		/**
		 * Retrieves and removes the last element of this deque.  This method
		 * differs from {@link #pollLast pollLast} only in that it throws an
		 * exception if this deque is empty.
		 *
		 * @return the tail of this deque
		 * @throws NoSuchElementException if this deque is empty
		 */
		virtual SPtr<E> removeLast() = 0;

		/**
		 * Retrieves and removes the first element of this deque,
		 * or returns <tt>null</tt> if this deque is empty.
		 *
		 * @return the head of this deque, or <tt>nullptr</tt> if this deque is empty
		 */
		virtual SPtr<E> pollFirst() = 0;

		/**
		 * Retrieves and removes the last element of this deque,
		 * or returns <tt>null</tt> if this deque is empty.
		 *
		 * @return the tail of this deque, or <tt>nullptr</tt> if this deque is empty
		 */
		virtual SPtr<E> pollLast() = 0;

		/**
		 * Retrieves, but does not remove, the first element of this deque.
		 *
		 * This method differs from {@link #peekFirst peekFirst} only in that it
		 * throws an exception if this deque is empty.
		 *
		 * @return the head of this deque
		 * @throws NoSuchElementException if this deque is empty
		 */
		virtual SPtr<E> getFirst() = 0;

		/**
		 * Retrieves, but does not remove, the last element of this deque.
		 * This method differs from {@link #peekLast peekLast} only in that it
		 * throws an exception if this deque is empty.
		 *
		 * @return the tail of this deque
		 * @throws NoSuchElementException if this deque is empty
		 */
		virtual SPtr<E> getLast() = 0;

		/**
		 * Retrieves, but does not remove, the first element of this deque,
		 * or returns <tt>nullptr</tt> if this deque is empty.
		 *
		 * @return the head of this deque, or <tt>null</tt> if this deque is empty
		 */
		virtual SPtr<E> peekFirst() = 0;

		/**
		 * Retrieves, but does not remove, the last element of this deque,
		 * or returns <tt>null</tt> if this deque is empty.
		 *
		 * @return the tail of this deque, or <tt>null</tt> if this deque is empty
		 */
		virtual SPtr<E> peekLast() = 0;

		/**
		 * Inserts the specified element into the queue represented by this deque
		 * (in other words, at the tail of this deque) if it is possible to do so
		 * immediately without violating capacity restrictions, returning
		 * <tt>true</tt> upon success and throwing an
		 * <tt>IllegalStateException</tt> if no space is currently available.
		 * When using a capacity-restricted deque, it is generally preferable to
		 * use {@link #offer(Object) offer}.
		 *
		 * <p>This method is equivalent to {@link #addLast}.
		 *
		 * @param e the element to add
		 * @return <tt>true</tt> (as specified by {@link Collection#add})
		 * @throws IllegalStateException if the element cannot be added at this
		 *         time due to capacity restrictions
		 * @throws ClassCastException if the class of the specified element
		 *         prevents it from being added to this deque
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * @throws IllegalArgumentException if some property of the specified
		 *         element prevents it from being added to this deque
		 */
		virtual bool add(SPtr<E> const& e) = 0;

		/**
		 * Inserts the specified element into the queue represented by this deque
		 * (in other words, at the tail of this deque) if it is possible to do so
		 * immediately without violating capacity restrictions, returning
		 * <tt>true</tt> upon success and <tt>false</tt> if no space is currently
		 * available.  When using a capacity-restricted deque, this method is
		 * generally preferable to the {@link #add} method, which can fail to
		 * insert an element only by throwing an exception.
		 *
		 * <p>This method is equivalent to {@link #offerLast}.
		 *
		 * @param e the element to add
		 * @return <tt>true</tt> if the element was added to this deque, else
		 *         <tt>false</tt>
		 * @throws ClassCastException if the class of the specified element
		 *         prevents it from being added to this deque
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * @throws IllegalArgumentException if some property of the specified
		 *         element prevents it from being added to this deque
		 */
		virtual bool offer(SPtr<E> const& e) = 0;

		/**
		 * Retrieves and removes the head of the queue represented by this deque
		 * (in other words, the first element of this deque).
		 * This method differs from {@link #poll poll} only in that it throws an
		 * exception if this deque is empty.
		 *
		 * <p>This method is equivalent to {@link #removeFirst()}.
		 *
		 * @return the head of the queue represented by this deque
		 * @throws NoSuchElementException if this deque is empty
		 */
		virtual SPtr<E> remove() = 0;

		/**
		 * Retrieves and removes the head of the queue represented by this deque
		 * (in other words, the first element of this deque), or returns
		 * <tt>null</tt> if this deque is empty.
		 *
		 * <p>This method is equivalent to {@link #pollFirst()}.
		 *
		 * @return the first element of this deque, or <tt>null</tt> if
		 *         this deque is empty
		 */
		virtual SPtr<E> poll() = 0;

		/**
		 * Retrieves, but does not remove, the head of the queue represented by
		 * this deque (in other words, the first element of this deque).
		 * This method differs from {@link #peek peek} only in that it throws an
		 * exception if this deque is empty.
		 *
		 * <p>This method is equivalent to {@link #getFirst()}.
		 *
		 * @return the head of the queue represented by this deque
		 * @throws NoSuchElementException if this deque is empty
		 */
		virtual SPtr<E> element() = 0;

		/**
		 * Retrieves, but does not remove, the head of the queue represented by
		 * this deque (in other words, the first element of this deque), or
		 * returns <tt>null</tt> if this deque is empty.
		 *
		 * <p>This method is equivalent to {@link #peekFirst()}.
		 *
		 * @return the head of the queue represented by this deque, or
		 *         <tt>null</tt> if this deque is empty
		 */
		virtual SPtr<E> peek() = 0;

		/**
		 * Pushes an element onto the stack represented by this deque (in other
		 * words, at the head of this deque) if it is possible to do so
		 * immediately without violating capacity restrictions, returning
		 * <tt>true</tt> upon success and throwing an
		 * <tt>IllegalStateException</tt> if no space is currently available.
		 *
		 * <p>This method is equivalent to {@link #addFirst}.
		 *
		 * @param e the element to push
		 * @throws IllegalStateException if the element cannot be added at this
		 *         time due to capacity restrictions
		 * @throws ClassCastException if the class of the specified element
		 *         prevents it from being added to this deque
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * @throws IllegalArgumentException if some property of the specified
		 *         element prevents it from being added to this deque
		 */
		virtual void push(SPtr<E> const& e) = 0;

		/**
		 * Pops an element from the stack represented by this deque.  In other
		 * words, removes and returns the first element of this deque.
		 *
		 * <p>This method is equivalent to {@link #removeFirst()}.
		 *
		 * @return the element at the front of this deque (which is the top
		 *         of the stack represented by this deque)
		 * @throws NoSuchElementException if this deque is empty
		 */
		virtual SPtr<E> pop() = 0;

		/**
		 * Removes the first occurrence of the specified element from this deque.
		 * If the deque does not contain the element, it is unchanged.
		 * More formally, removes the first element <tt>e</tt> such that
		 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>
		 * (if such an element exists).
		 * Returns <tt>true</tt> if this deque contained the specified element
		 * (or equivalently, if this deque changed as a result of the call).
		 *
		 * <p>This method is equivalent to {@link #removeFirstOccurrence}.
		 *
		 * @param o element to be removed from this deque, if present
		 * @return <tt>true</tt> if an element was removed as a result of this call
		 * @throws ClassCastException if the class of the specified element
		 *         is incompatible with this deque
		 * (<a href="Collection.html#optional-restrictions">optional</a>)
		 * @throws NullPointerException if the specified element is null and this
		 *         deque does not permit null elements
		 * (<a href="Collection.html#optional-restrictions">optional</a>)
		 */
		virtual bool remove(const E& o) = 0;

		/**
		 * Returns the number of elements in this deque.
		 *
		 * @return the number of elements in this deque
		 */
		virtual size_t size() const = 0;

		/**
		 * Returns an iterator over the elements in this deque in proper sequence.
		 * The elements will be returned in order from first (head) to last (tail).
		 *
		 * @return an iterator over the elements in this deque in proper sequence
		 */
		virtual UPtr<ConstIterator<SPtr<E>>> constIterator() const = 0;
};

} // namespace

#endif // H_SLIB_COLLECTIONS_DEQUE_H
