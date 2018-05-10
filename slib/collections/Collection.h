/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_COLLECTION_H
#define H_SLIB_COLLECTIONS_COLLECTION_H

#include "slib/Object.h"
#include "slib/Iterator.h"

#include <sys/types.h>
#include <memory>

namespace slib {

template <class E>
class Collection : virtual public Object, public ConstIterable<E> {
public:
	virtual size_t size() const = 0;

	virtual bool isEmpty() const = 0;

	virtual ConstIterator<std::shared_ptr<E> > constIterator() const = 0;

	virtual bool add(std::shared_ptr<E> const& e) = 0;

	virtual bool add(E const& e) {
		return add(std::make_shared<E>(e));
	}

	virtual bool remove(const E& o) = 0;

	virtual void clear() = 0;
};

} // namespace slib

#endif // H_SLIB_COLLECTIONS_COLLECTION_H
