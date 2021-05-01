/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_COLLECTION_H
#define H_SLIB_COLLECTIONS_COLLECTION_H

#include "slib/lang/Object.h"
#include "slib/util/Iterator.h"
#include "slib/lang/StringBuilder.h"

#include <sys/types.h>
#include <memory>

namespace slib {

/**
 * The root interface in the <i>collection hierarchy</i>.  A collection
 * represents a group of objects, known as its <i>elements</i>.
 */
template <class E>
class Collection : public ConstIterable<E> {
public:
	TYPE_INFO(Collection, CLASS(Collection<E>), INHERITS(ConstIterable<E>));
public:
	virtual size_t size() const = 0;

	virtual bool isEmpty() const = 0;

	virtual UPtr<ConstIterator<SPtr<E>>> constIterator() const = 0;

	virtual bool add(SPtr<E> const& e) = 0;

	template <class AVT, typename... A>
	bool emplace(A&&... args) {
		return add(newS<AVT>(std::forward<A>(args)...));
	}

	template <class AVT>
	bool emplace(AVT const& e) {
		static_assert(std::is_same<AVT, E>::value, "Only use with the exact value type");
		return add(newS<E>(e));
	}

	virtual bool remove(const E& o) = 0;

	virtual void clear() = 0;

	virtual UPtr<String> toString() const override {
		UPtr<ConstIterator<SPtr<E>>> i = constIterator();
		if (!i->hasNext())
			return "[]"_UPTR;

		StringBuilder sb;
		sb.add('[');
		do {
			SPtr<E> const& e = i->next();

			E const* obj = e.get();
			if (instanceof<Collection<E>>(obj) && Class::constCast<Collection<E>>(obj) == this)
				sb.add("(this Collection)");
			else
				sb.add(slib::toString(obj));

			if (!i->hasNext())
				return sb.add(']').toString();
			sb.add(", ");
		} while (true);
	}
};

} // namespace slib

#endif // H_SLIB_COLLECTIONS_COLLECTION_H
