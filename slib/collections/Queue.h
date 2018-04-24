/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_QUEUE_H
#define H_SLIB_COLLECTIONS_QUEUE_H

#include "slib/collections/Collection.h"

namespace slib {

template <class E>
class Queue : public Collection<E> {
public:
	virtual bool offer(std::shared_ptr<E> const& e) = 0;

	virtual bool offer(const E& e) {
		return offer(std::make_shared<E>(e));
	}

	/** @throws NoSuchElementException */
	virtual std::shared_ptr<E> remove() = 0;

	virtual std::shared_ptr<E> poll() = 0;
};

} // namespace slib

#endif // H_SLIB_COLLECTIONS_QUEUE_H
