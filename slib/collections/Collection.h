/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_COLLECTION_H
#define H_SLIB_COLLECTIONS_COLLECTION_H

#include "slib/lang/Object.h"
#include "slib/util/Iterator.h"

#include <sys/types.h>
#include <memory>

namespace slib {

template <class E>
class Collection : virtual public Object, public ConstIterable<E> {
public:
	static Class const* CLASS() {
		return COLLECTIONCLASS();
	}

	virtual size_t size() const = 0;

	virtual bool isEmpty() const = 0;

	virtual ConstIterator<SPtr<E>> constIterator() const = 0;

	virtual bool add(SPtr<E> const& e) = 0;

	template <class AVT, typename... A>
	bool emplace(A&&... args) {
		return add(std::make_shared<AVT>(std::forward<A>(args)...));
	}

	template <class AVT>
	bool emplace(AVT const& e) {
		static_assert(std::is_same<AVT, E>::value, "Only use with the exact value type");
		return add(std::make_shared<E>(e));
	}

	virtual bool remove(const E& o) = 0;

	virtual void clear() = 0;
};

} // namespace slib

#endif // H_SLIB_COLLECTIONS_COLLECTION_H
