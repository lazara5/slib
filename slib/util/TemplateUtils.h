/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_TEMPLATEUTILS_H
#define H_SLIB_UTIL_TEMPLATEUTILS_H

#include <memory>

namespace slib {

template <typename T>
T const* Ptr(T const& obj) {
	return &obj;
}

template <typename T>
T const* Ptr(T const* obj) {
	return obj;
}

template <typename T>
T const* Ptr(T * obj) {
	return obj;
}

template <typename T>
T const* Ptr(std::shared_ptr<T> const& obj) {
	return obj.get();
}

template <typename T>
T const* Ptr(std::unique_ptr<T> const& obj) {
	return obj.get();
}

} // namespace slib

#endif // H_SLIB_UTIL_TEMPLATEUTILS_H
