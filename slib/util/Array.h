/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_ARRAY_H
#define H_SLIB_UTIL_ARRAY_H

#include <cstddef>

namespace slib {

template <typename T, size_t S>
struct ArrayTraits {
	typedef T _Type[S];

	static constexpr T&
	_S_ref(const _Type& __t, size_t __n) noexcept
	{ return const_cast<T&>(__t[__n]); }

	static constexpr T*
	_S_ptr(const _Type& __t) noexcept
	{ return const_cast<T*>(__t); }
};

template <typename T>
struct ArrayTraits<T, 0> {
	struct _Type { };

	static constexpr T&
	_S_ref(const _Type&, std::size_t) noexcept
	{ return *static_cast<T*>(nullptr); }

	static constexpr T*
	_S_ptr(const _Type&) noexcept
	{ return nullptr; }
};

/** Minimal implementation of constexpr array */
template <typename T, size_t S>
class Array {
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type *const_pointer;
	typedef value_type&                   	      reference;
	typedef const value_type&             	      const_reference;

	typedef ArrayTraits<T, S> _AT_Type;
	typename _AT_Type::_Type _M_elems;
public:
	constexpr size_t size() const noexcept {
		return S;
	}

	// Element access

	constexpr const_reference
	operator[](size_t __n) const noexcept {
		return _AT_Type::_S_ref(_M_elems, __n);
	}

	constexpr const_pointer
	data() const noexcept {
		return _AT_Type::_S_ptr(_M_elems);
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_ARRAY_H
