/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_ITERATOR_H
#define H_SLIB_ITERATOR_H

#include "slib/lang/Object.h"
#include "slib/exception/UnsupportedOperationException.h"

namespace slib {

/** Base class for iterators that do not modify the container */
template <class T>
class ConstIterator {
public:
	class ConstIteratorImpl {
	public:
		virtual bool hasNext() = 0;
		virtual const T& next() = 0;
		
		virtual ConstIteratorImpl *clone() = 0;

		virtual ~ConstIteratorImpl() {}
	};

	class NullConstIteratorImpl : public ConstIteratorImpl {
	public:
		virtual bool hasNext() {
			return false;
		}

		virtual const T& next() {
			throw NoSuchElementException(_HERE_);
		}

		virtual typename ConstIterator<T>::ConstIteratorImpl *clone() {
			return new NullConstIteratorImpl();
		}
	};

	class StdConstIterator {
	protected:
		ConstIteratorImpl *_instance;
		const T* _crt;
	public:
		StdConstIterator(const ConstIterator& iter) {
			//printf("I %p\n", this);
			_instance = iter._instance->clone();
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

protected:
	ConstIteratorImpl *_instance;

public:
	ConstIterator(ConstIteratorImpl *instance) {
		_instance = instance;
	}
	
	static ConstIterator<T> getNull() {
		return ConstIterator<T>(new NullConstIteratorImpl());
	}

	virtual ~ConstIterator() {
		if (_instance != nullptr)
			delete _instance;
	}

	ConstIterator(const ConstIterator& other) {
		if (other._instance)
			_instance = other._instance->clone();
		else
			_instance = nullptr;
	}

	ConstIterator& operator=(const ConstIterator& other) {
		if (this == &other)
			return *this;
		if (_instance != nullptr)
			delete _instance;
		if (other._instance)
			_instance = other._instance->clone();
		else
			_instance = nullptr;
		return *this;
	}

	/**
	 * Returns <i>true</i> if the iteration has more elements. (In other
	 * words, returns <i>true</i> if <i>next()</i> would return an element
	 * rather than throwing an exception.)
	 *
	 * @return <i>true</i> if the iterator has more elements.
	 */
	virtual bool hasNext() {
		return _instance->hasNext();
	}

	/**
	 * Returns the next element in the iteration.
	 *
	 * @return the next element in the iteration.
	 * @exception NoSuchElementException iteration has no more elements.
	 */
	virtual const T& next() {
		return _instance->next();
	}

	//--- std iterator compatibility -----------------

	StdConstIterator begin() {
		return StdConstIterator(*this);
	}

	StdConstIterator end() {
		return StdConstIterator();
	}
};

/** Base class for iterators that may modify the container */
template <class T>
class Iterator {
public:
	class IteratorImpl : public ConstIterator<T>::ConstIteratorImpl {
	public:
		virtual void remove() {
			throw UnsupportedOperationException(_HERE_, "Iterator::remove()");
		}
	};

	class NullIteratorImpl : public IteratorImpl {
	public:
		virtual typename Iterator<T>::IteratorImpl *clone() {
			return new NullIteratorImpl();
		}
	};

protected:
	IteratorImpl *_instance;
public:
	Iterator(IteratorImpl *instance) {
		_instance = instance;
	}

	static Iterator<T> getNull() {
		return Iterator<T>(new NullIteratorImpl());
	}

	/**
	 * Returns <i>true</i> if the iteration has more elements. (In other
	 * words, returns <i>true</i> if <i>next()</i> would return an element
	 * rather than throwing an exception.)
	 *
	 * @return <i>true</i> if the iterator has more elements.
	 */
	virtual bool hasNext() {
		return _instance->hasNext();
	}

	/**
	 * Returns the next element in the iteration.
	 *
	 * @return the next element in the iteration.
	 * @exception NoSuchElementException iteration has no more elements.
	 */
	virtual const T& next() {
		return _instance->next();
	}

	/**
	 * 
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
	virtual void remove() {
		return _instance->remove();
	}
};

template <class T>
class ConstIterable : virtual public Object {
public:
	static Class const* CLASS() {
		return CONSTITERABLECLASS();
	}

	virtual ConstIterator<std::shared_ptr<T>> constIterator() const = 0;
};

} // namespace

#endif // H_SLIB_ITERATOR_H
