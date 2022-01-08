/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_EXCEPTION_EXCEPTION_H
#define H_SLIB_EXCEPTION_EXCEPTION_H

#include <string>

#define _STRINGIFY_(x) #x
#define _TOSTRING_(x) _STRINGIFY_(x)
#define _HERE_ __FILE__ ":" _TOSTRING_(__LINE__)

#define THROW(EXCEPTION, ...) throw EXCEPTION(_HERE_, ##__VA_ARGS__)

namespace slib {

/** Base class for all exceptions thrown by 'slib::' classes */
class Exception {
private:
	std::string _where;
	std::string _errorMessage;
	std::string _className;
protected:
	Exception(const char *where, const char *className, const char *msg)
	: _where(where)
	, _errorMessage(msg)
	, _className(className) {
	}

	Exception(const char *where, const char *className, Exception const& e);

	Exception(const char *where, const char *className, const char *msg, Exception const& e);
public:
	/**
	 * Constructs an Exception object with a given location and error message
	 * @param where exception location (should be passed using the <i><b>_HERE_</b></i> macro)
	 * @param msg error message
	 */
	Exception(const char *where, const char *msg)
	: Exception(where, "Exception", msg) {
	}

	Exception(const char *where, const Exception& e)
	: Exception(where, "Exception", e) {
	}

	/**
	 * Returns the class name for this Exception object
	 * @return the exception class name
	 */
	const char *getName() const {
		return _className.c_str();
	}

	/**
	 * Returns the error message for this Exception object
	 * @return the error message
	 */
	const char *getMessage() const {
		return _errorMessage.c_str();
	}

	/**
	 * Returns the location for this Exception object
	 * @return the source code location
	 */
	const char *where() const {
		return _where.c_str();
	}

	/**
	 * Returns a string representation of this Exception object
	 * @return a string representation of this Exception object
	 */
	const std::string toString() const {
		return _className + ":" + _errorMessage;
	}
};

/** Thrown when we cannot allocate an object because we are out of memory. */
class OutOfMemoryError : public Exception {
public:
	OutOfMemoryError(const char *where)
	:Exception(where, "OutOfMemoryError", "Out of memory") {}
};

/**
 * This exception may be thrown by methods that have detected concurrent
 * modification of an object when such modification is not permissible.
 * <p>
 * For example, it is not generally permissible for one thread to modify a Collection
 * while another thread is iterating over it. In general, the results of the
 * iteration are undefined under these circumstances. Some Iterator
 * implementations may choose to throw this exception if this behavior is
 * detected.  Iterators that do this are known as <i>fail-fast</i> iterators,
 * as they fail quickly and cleanly, rather that risking arbitrary,
 * non-deterministic behavior at an undetermined time in the future.
 * <p>
 * Note that this exception does not always indicate that an object has
 * been concurrently modified by a <i>different</i> thread. If a single
 * thread issues a sequence of method invocations that violates the
 * contract of an object, the object may throw this exception. For
 * example, if a thread modifies a collection directly while it is
 * iterating over the collection with a fail-fast iterator, the iterator
 * will throw this exception.
 *
 * <p>Note that fail-fast behavior cannot be guaranteed as it is, generally
 * speaking, impossible to make any hard guarantees in the presence of
 * unsynchronized concurrent modification. Fail-fast operations
 * throw ConcurrentModificationException on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness: <i>ConcurrentModificationException
 * should be used only to detect bugs.</i>
 */
class ConcurrentModificationException : public Exception {
public:
	ConcurrentModificationException(const char *where, const char *msg)
	:Exception(where, "ConcurrentModificationException", msg) {}

	ConcurrentModificationException(const char *where)
	:Exception(where, "ConcurrentModificationException", "") {}
};

class InitException : public Exception {
public:
	InitException(const char *where, const char *msg)
	:Exception(where, "InitException", msg) {}

	InitException(const char *where, const Exception& e)
	:Exception(where, "InitException", e) {}

	InitException(const char *where)
	:Exception(where, "InitException", "") {}
};

class NoSuchElementException : public Exception {
public:
	NoSuchElementException(const char *where, const char *msg)
	:Exception(where, "NoSuchElementException", msg) {}

	NoSuchElementException(const char *where)
	:Exception(where, "NoSuchElementException", "") {}
};

/**
 * Thrown to indicate that an index of some sort (such as to an array, to a string or to a vector)
 * is out of range. Applications can subclass this class to indicate similar exceptions.
 */
class IndexOutOfBoundsException : public Exception {
public:
	IndexOutOfBoundsException(const char *where, const char *msg)
	:Exception(where, "IndexOutOfBoundsException", msg) {}

	IndexOutOfBoundsException(const char *where)
	:Exception(where, "IndexOutOfBoundsException", "") {}
};

class ArrayIndexOutOfBoundsException : public IndexOutOfBoundsException {
public:
	ArrayIndexOutOfBoundsException(const char *where, size_t i);
};

} // namespace

#endif // H_SLIB_EXCEPTION_EXCEPTION_H
