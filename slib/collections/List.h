/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_LIST_H
#define H_SLIB_COLLECTIONS_LIST_H

#include "slib/collections/Collection.h"
#include "slib/Iterator.h"
#include "slib/exception/UnsupportedOperationException.h"

#include <memory>

namespace slib {

template <class E>
class List : public Collection<E> {
public:
	virtual ~List() {}

	static Class const* CLASS() {
		return LISTCLASS();
	}

	virtual size_t size() const override = 0;

	virtual bool isEmpty() const override = 0;

	virtual bool add(std::shared_ptr<E> const& e) override = 0;

	using Collection<E>::add;

	/**
	 * Inserts the specified element at the specified position in this list
	 * (optional operation). Shifts the element currently at that position
	 * and any subsequent elements to the right.
	 *
	 * @param index  index at which to insert the specified element
	 * @param e  element to be inserted ito this list
	 * @throws UnsupportedOperationException if the <b>add</b> operation
	 *         is not supported by this list
	 * @throws IndexOutOfBoundsException if the index is out of range
	 *         (index < 0 || index > size())
	 */
	virtual void add(int index, const E& e) {
		throw UnsupportedOperationException(_HERE_);
	}
	
	virtual int indexOf(const E& o) = 0;

	virtual std::shared_ptr<E> get(size_t index) const = 0;
public:
	virtual ConstIterator<std::shared_ptr<E> > constIterator() const override = 0;
};

} // namespace

#endif // H_SLIB_COLLECTIONS_LIST_H
