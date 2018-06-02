/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_TEMPLATEUTILS_H
#define H_SLIB_UTIL_TEMPLATEUTILS_H

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

} // namespace slib

#endif // H_SLIB_UTIL_TEMPLATEUTILS_H
