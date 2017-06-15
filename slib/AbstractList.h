/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SLIB_ABSTRACTLIST_H__
#define __SLIB_ABSTRACTLIST_H__

#include "slib/List.h"

namespace slib {

template <class E>
class AbstractList : public List<E> {
protected:
	/**
	 * The number of times this list has been structurally modified.
	 * Structural modifications are those that modify the list so that
	 * iterations in progress may yield incorrect results.
	 *
	 * <p>This field is used by the iterator and list iterator
	 * implementations returned by the <i>iterator</i> methods.
	 * If the value of this field changes unexpectedly, the iterator will 
	 * throw a ConcurrentModificationException in response to the 
	 * <i>next</i>, <i>remove</i>, <i>previous</i>, <i>set</i> or <i>add</i> 
	 * operations. This provides <i>fail-fast</i> behavior, rather than 
	 * non-deterministic behavior in the situation of concurrent 
	 * modification during iteration.
	 *
	 * <p>Use of this field by subclasses is optional. If a subclass
	 * wants to provide fail-fast iterators, then it only has to increment 
	 * this field in its <i>add(int, E)</i> and <i>remove(int)</i> methods 
	 * (and any other methods that it overrides that result in structural 
	 * modifications to the list). A single call to <i>add(int, E)</i> 
	 * or <i>remove(int)</i> must add no more than one to this field, or 
	 * the iterators will throw bogus ConcurrentModificationExceptions.
	 */
	volatile int _modCount;

public:
	AbstractList() {
		_modCount = 0;
	}

	virtual ~AbstractList() {}
};

} // namespace

#endif
