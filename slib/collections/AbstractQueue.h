/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_ABSTRACTQUEUE_H
#define H_SLIB_COLLECTIONS_ABSTRACTQUEUE_H

#include "slib/collections/Queue.h"
#include "slib/exception/IllegalStateException.h"

namespace slib {

template <class E>
class AbstractQueue: public Queue<E> {
public:
	virtual ssize_t size() const override = 0;

	virtual bool isEmpty() const override {
		return size() == 0;
	}

	using Queue<E>::offer;

	virtual bool add(std::shared_ptr<E> const& e) override {;
		if (offer(e))
			return true;
		else
			throw new IllegalStateException("Queue full");
	}

	virtual std::shared_ptr<E> remove() override {
		std::shared_ptr<E> e = poll();
		if (e)
			return e;

		throw new NoSuchElementException(_HERE_);
	}

	virtual void clear() override {
		while (poll());
	}

	std::shared_ptr<E> poll() override = 0;
};

} // namespace slib

#endif // H_SLIB_COLLECTIONS_ABSTRACTQUEUE_H
