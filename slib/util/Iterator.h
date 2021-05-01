/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_ITERATOR_H
#define H_SLIB_ITERATOR_H

#include "slib/lang/Object.h"
#include "slib/exception/UnsupportedOperationException.h"

namespace slib {

template <class T>
class ConstIteratorWrapper;

template <class T>
class ConstIteratorBase {
public:
	virtual ~ConstIteratorBase() {}
public:
	/**
	 * Returns <i>true</i> if the iteration has more elements. (In other
	 * words, returns <i>true</i> if <i>next()</i> would return an element
	 * rather than throwing an exception.)
	 *
	 * @return <i>true</i> if the iterator has more elements.
	 */
	virtual bool hasNext() const = 0;

	/**
	 * Returns the next element in the iteration.
	 *
	 * @return the next element in the iteration.
	 * @exception NoSuchElementException iteration has no more elements.
	 */
	virtual const T& next() = 0;
};

template <class T>
class ConstIteratorImpl : virtual public ConstIteratorBase<T> {
public:
	virtual ~ConstIteratorImpl() {};

	virtual ConstIteratorImpl *clone() = 0;
};

template <class I, class T>
class StdConstIterator {
protected:
	I *_instance;
	const T* _crt;
public:
	StdConstIterator(I *iter) {
		//printf("I %p\n", this);
		_instance = iter->clone();
		if (_instance->hasNext())
			_crt = &_instance->next();
		else
			_crt = nullptr;
	}

	StdConstIterator(const StdConstIterator& other) {
		//printf("& %p %p\n", this, &other);
		if (other._instance)
			_instance = other._instance->clone();
		else
			_instance = nullptr;
		_crt = other._crt;
	}

	StdConstIterator() {
		//printf("N %p\n", this);
		_instance = nullptr;
		_crt = nullptr;
	}

	virtual ~StdConstIterator() {
		//printf("~ %p\n", this);
		if (_instance != nullptr)
			delete _instance;
	}

	StdConstIterator& operator=(const StdConstIterator& other) {
		//printf("= %p %p\n", this, &other);
		if (this == &other)
			return *this;
		if (_instance != nullptr)
			delete _instance;
		if (other._instance)
			_instance = other._instance->clone();
		else
			_instance = nullptr;
		_crt = other._crt;
		return *this;
	}

	StdConstIterator operator ++() {
		//printf("+ %p\n", this);
		if (_instance) {
			if (_instance->hasNext())
				_crt = &_instance->next();
			else
				_crt = nullptr;
		}
		return *this;
	}

	bool operator !=(const StdConstIterator &other) const {
		//printf("!= %p %p\n", this, &other);
		return (_crt != other._crt);
	}

	const T& operator*() {
		return *_crt;
	}
};

namespace internal {

template <class I, class T>
class BaseIteratorWrapper {
protected:
	I *_instance;
protected:
	BaseIteratorWrapper(I *instance)
	: _instance(instance) {}

	BaseIteratorWrapper(BaseIteratorWrapper const& other) {
		if (other._instance)
			_instance = other._instance->clone();
		else
			_instance = nullptr;
	}

	virtual ~BaseIteratorWrapper() {
		if (_instance)
			delete _instance;
	}

	void assign(BaseIteratorWrapper const& other) {
		if (this == &other)
			return;
		if (_instance != nullptr)
			delete _instance;
		if (other._instance)
			_instance = other._instance->clone();
		else
			_instance = nullptr;
	}
};

} // namespace

template <class I, class T>
class StdIterable {
public:
	virtual StdConstIterator<I, T> begin() = 0;
	virtual StdConstIterator<I, T> end() = 0;
};

/** Base class for iterators that do not modify the container */
template <class T>
class ConstIterator : virtual public ConstIteratorBase<T>, virtual public StdIterable<ConstIteratorImpl<T>, T> {
};

template <class T>
class ConstIteratorWrapper: public internal::BaseIteratorWrapper<ConstIteratorImpl<T>, T>, virtual public ConstIterator<T> {
public:
	ConstIteratorWrapper(ConstIteratorImpl<T> *instance)
	: internal::BaseIteratorWrapper<ConstIteratorImpl<T>, T>(instance) {}

	ConstIteratorWrapper(ConstIteratorWrapper const& other)
	: internal::BaseIteratorWrapper<ConstIteratorImpl<T>, T>(other) {}

	ConstIteratorWrapper& operator=(const ConstIteratorWrapper& other) {
		assign(other);
		return *this;
	}

	virtual bool hasNext() const override {
		return this->_instance->hasNext();
	}

	virtual const T& next() override {
		return this->_instance->next();
	}

public:
	virtual StdConstIterator<ConstIteratorImpl<T>, T> begin() override {
		return StdConstIterator<ConstIteratorImpl<T>, T>(this->_instance);
	}

	virtual StdConstIterator<ConstIteratorImpl<T>, T> end() override {
		return StdConstIterator<ConstIteratorImpl<T>, T>();
	}
};

/** Base class for iterators that may modify the container */
template <class T>
class IteratorBase : virtual public ConstIteratorBase<T> {
public:
	virtual ~IteratorBase() {}
public:
	/**
	 * Removes from the underlying collection the last element returned by the
	 * iterator (optional operation). This method can be called only once per
	 * call to <i>next()</i>. The behavior of an iterator is unspecified if
	 * the underlying collection is modified while the iteration is in
	 * progress in any way other than by calling this method.
	 *
	 * @exception UnsupportedOperationException if the <i>remove()</i>
	 *		operation is not supported by this Iterator.

	 * @exception IllegalStateException if the <i>next()</i> method has not
	 *		yet been called or the <i>remove()</i> method has already
	 *		been called after the last call to the <i>next()</i> method.
	 */
	virtual void remove() = 0; /* {
		throw UnsupportedOperationException(_HERE_, "Iterator::remove()");
	}*/
};

template <class T>
class IteratorImpl : virtual public IteratorBase<T>, virtual public ConstIteratorImpl<T> {
public:
	virtual ~IteratorImpl() {}

	virtual IteratorImpl *clone() = 0;
};

template <class T>
class Iterator: virtual public IteratorBase<T>, virtual public StdIterable<IteratorImpl<T>, T> {
};

template <class T>
class IteratorWrapper: public internal::BaseIteratorWrapper<IteratorImpl<T>, T>, virtual public Iterator<T> {
public:
	IteratorWrapper(IteratorImpl<T> *instance)
	: internal::BaseIteratorWrapper<IteratorImpl<T>, T>(instance) {}

	IteratorWrapper(IteratorWrapper const& other)
	: internal::BaseIteratorWrapper<IteratorImpl<T>, T>(other) {}

	IteratorWrapper& operator=(const IteratorWrapper& other) {
		assign(other);
		return *this;
	}

	virtual bool hasNext() const override {
		return this->_instance->hasNext();
	}

	virtual const T& next() override {
		return this->_instance->next();
	}

	virtual void remove() override {
		this->_instance->remove();
	}

public:
	virtual StdConstIterator<IteratorImpl<T>, T> begin() override {
		return StdConstIterator<IteratorImpl<T>, T>(this->_instance);
	}

	virtual StdConstIterator<IteratorImpl<T>, T> end() override {
		return StdConstIterator<IteratorImpl<T>, T>();
	}
};

template <class T>
class ConstListIteratorBase : virtual public ConstIteratorBase<T> {
public:
	virtual bool hasPrevious() const = 0;
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

	virtual bool hasNext() const override {
		return this->_instance->hasNext();
	}

	virtual const T& next() override {
		return this->_instance->next();
	}

	virtual bool hasPrevious() const override {
		return this->_instance->hasPrevious();
	}

	virtual const T& previous() override {
		return this->_instance->next();
	}

	virtual size_t nextIndex() const override {
		return this->_instance->previous();
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
class ListIteratorImpl : virtual public ListIteratorBase<T>, virtual public ConstListIteratorImpl<T> {
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

	virtual bool hasNext() const override {
		return this->_instance->hasNext();
	}

	virtual const T& next() override {
		return this->_instance->next();
	}

	virtual bool hasPrevious() const override {
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

template <class T>
class ConstIterable : virtual public Object {
public:
	TYPE_INFO(ConstIterable, CLASS(ConstIterable<T>), INHERITS(Object));
public:
	virtual UPtr<ConstIterator<SPtr<T>>> constIterator() const = 0;
};

template <class T>
class Iterable : virtual public Object {
public:
	TYPE_INFO(Iterable, CLASS(Iterable<T>), INHERITS(Object));
public:
	virtual UPtr<Iterator<SPtr<T>>> iterator() const = 0;
};

} // namespace

#endif // H_SLIB_ITERATOR_H
