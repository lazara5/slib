/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_TEMPLATEUTILS_H
#define H_SLIB_UTIL_TEMPLATEUTILS_H

#include <slib/util/Array.h>
#include <slib/third-party/cppbits/make_unique.h>

#include <memory>


namespace slib {

template <class T>
using SPtr = std::shared_ptr<T>;

template <class T>
using UPtr = std::unique_ptr<T>;

template <typename T>
T const* CPtr(T const& obj) {
	return &obj;
}

template <typename T>
T const* CPtr(T const* obj) {
	return obj;
}

template <typename T>
T const* CPtr(T * obj) {
	return obj;
}

template <typename T>
T const* CPtr(std::shared_ptr<T> const& obj) {
	return obj.get();
}

template <typename T>
T const* CPtr(std::unique_ptr<T> const& obj) {
	return obj.get();
}

template<int = sizeof(size_t)>
int32_t sizeTHash(size_t h);

template<>
int32_t sizeTHash<8>(size_t h);

template<>
int32_t sizeTHash<4>(size_t h);

// for array join

template<std::size_t ... Size>
struct num_tuple {};

template<std::size_t Prepend, typename T>
struct appender {};

template<std::size_t Prepend, std::size_t ... Sizes>
struct appender<Prepend, num_tuple<Sizes...>>
{
	using type = num_tuple<Prepend, Sizes...>;
};

template<std::size_t Size, std::size_t Counter = 0>
struct counter_tuple
{
	using type = typename appender<Counter, typename counter_tuple<Size, Counter + 1>::type>::type;
};

template<std::size_t Size>
struct counter_tuple<Size, Size>
{
	using type = num_tuple<>;
};

template<typename T, size_t LL, size_t RL, size_t ... LLs, size_t ... RLs>
constexpr Array<T, LL+RL> join(const Array<T, LL> rhs, const Array<T, RL> lhs, num_tuple<LLs...>, num_tuple<RLs...>)
{
	return {rhs[LLs]..., lhs[RLs]... };
};


template<typename T, size_t LL, size_t RL>
constexpr Array<T, LL+RL> join(Array<T, LL> rhs, Array<T, RL> lhs)
{
	return join(rhs, lhs, typename counter_tuple<LL>::type(), typename counter_tuple<RL>::type());
}

// Concatenate variadic template parameter packs

template <typename T, typename ...>
struct cat {
	using type = T;
};

template <template <typename ...> class C,
		  typename ... Ts1, typename ... Ts2, typename ... Ts3>
struct cat<C<Ts1...>, C<Ts2...>, Ts3...>
: public cat<C<Ts1..., Ts2...>, Ts3...> {
};

// ---------------

} // namespace slib

#endif // H_SLIB_UTIL_TEMPLATEUTILS_H
