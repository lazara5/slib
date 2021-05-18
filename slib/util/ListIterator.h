/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_LIST_ITERATOR_H
#define H_SLIB_LISTITERATOR_H

#include "slib/util/Iterator.h"

namespace slib {

template <class T>
class ConstListIteratorBase : virtual public ConstIteratorBase<T> {
public:
	virtual bool hasPrevious() = 0;
	virtual const T& previous() = 0;
	virtual size_t nextIndex() const = 0;
	virtual ssize_t previousIndex() const = 0;
};

template <class T>
class ConstListIteratorImpl : virtual public ConstListIteratorBase<T>, virtual public ConstIteratorImpl<T> {
public:
	virtual ~ConstListIteratorImpl() {}

	virtual ConstListIteratorImpl *clone() = 0;
};

template <class T>
class ConstListIterator: virtual public ConstListIteratorBase<T>, virtual public StdIterable<ConstListIteratorImpl<T>, T> {
};

template <class T>
class ConstListIteratorWrapper: public internal::BaseIteratorWrapper<ConstListIteratorImpl<T>, T>, virtual public ConstListIterator<T> {
public:
	ConstListIteratorWrapper(ConstListIteratorImpl<T> *instance)
	: internal::BaseIteratorWrapper<ConstListIteratorImpl<T>, T>(instance) {}

	ConstListIteratorWrapper(ConstListIteratorWrapper const& other)
	: internal::BaseIteratorWrapper<ConstListIteratorImpl<T>, T>(other) {}

	ConstListIteratorWrapper& operator=(const ConstListIteratorWrapper& other) {
		assign(other);
		return *this;
	}

	virtual bool hasNext() override {
		return this->_instance->hasNext();
	}

	virtual const T& next() override {
		return this->_instance->next();
	}

	virtual bool hasPrevious() override {
		return this->_instance->hasPrevious();
	}

	virtual const T& previous() override {
		return this->_instance->next();
	}

	virtual size_t nextIndex() const override {
		return this->_instance->nextIndex();
	}

	virtual ssize_t previousIndex() const override {
		return this->_instance->previousIndex();
	}
public:
	virtual StdConstIterator<ConstListIteratorImpl<T>, T> begin() override {
		return StdConstIterator<ConstListIteratorImpl<T>, T>(this->_instance);
	}

	virtual StdConstIterator<ConstListIteratorImpl<T>, T> end() override {
		return StdConstIterator<ConstListIteratorImpl<T>, T>();
	}
};

template <class T>
class ListIteratorBase : virtual public IteratorBase<T>, virtual public ConstListIteratorBase<T> {
public:
	virtual void remove() = 0;
	virtual void add(T const& e) = 0;
};

template <class T>
class ListIteratorImpl : virtual public ListIteratorBase<T>, virtual public ConstListIteratorImpl<T>, virtual public IteratorImpl<T> {
public:
	virtual ~ListIteratorImpl() {}

	virtual ListIteratorImpl *clone() = 0;
};

template <class T>
class ListIterator : virtual public ListIteratorBase<T>, virtual public StdIterable<ListIteratorImpl<T>, T> {
};

template <class T>
class ListIteratorWrapper : virtual public ListIterator<T>, public internal::BaseIteratorWrapper<ListIteratorImpl<T>, T> {
public:
	ListIteratorWrapper(ListIteratorImpl<T> *instance)
	: internal::BaseIteratorWrapper<ListIteratorImpl<T>, T>(instance) {}

	ListIteratorWrapper(ListIteratorWrapper const& other)
	: internal::BaseIteratorWrapper<ListIteratorImpl<T>, T>(other) {}

	ListIteratorWrapper& operator=(const ListIteratorWrapper& other) {
		assign(other);
		return *this;
	}

	virtual bool hasNext() override {
		return this->_instance->hasNext();
	}

	virtual const T& next() override {
		return this->_instance->next();
	}

	virtual bool hasPrevious() override {
		return this->_instance->hasPrevious();
	}

	virtual const T& previous() override {
		return this->_instance->previous();
	}

	virtual size_t nextIndex() const override {
		return this->_instance->nextIndex();
	}

	virtual ssize_t previousIndex() const override {
		return this->_instance->previousIndex();
	}

	virtual void remove() override {
		this->_instance->remove();
	}

	virtual void add(T const& e) override {
		this->_instance->add(e);
	}
public:
	virtual StdConstIterator<ListIteratorImpl<T>, T> begin() override {
		return StdConstIterator<ListIteratorImpl<T>, T>(this->_instance);
	}

	virtual StdConstIterator<ListIteratorImpl<T>, T> end() override {
		return StdConstIterator<ListIteratorImpl<T>, T>();
	}
};

} // namespace

#endif // H_SLIB_LISTITERATOR_H
