/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_OBJECT_H
#define H_SLIB_OBJECT_H

#include "slib/lang/Class.h"
#include "slib/util/TemplateUtils.h"

#include <stdint.h>

#include <memory>

namespace slib {

class String;

class Object {
public:
	BASE_TYPE_INFO(Object, (Object));
public:
	virtual ~Object() {}

	virtual Class const& getClass() const {
		return classOf<Object>::_class();
	}

	virtual int32_t hashCode() const {
		uint64_t value = (uint64_t)this;
		return (int32_t)(value ^ (value >> 32));
	}

	virtual UPtr<String> toString() const;

	virtual bool equals(Object const& other) const {
		return this == &other;
	}

	bool operator ==(Object const& other) const {
		return equals(other);
	}
};

template <class T>
bool instanceof(Object const* obj) {
	if (!obj)
		return false;
	return (classOf<T>::_class().isAssignableFrom(obj->getClass()));
}

template <class T>
bool instanceof(SPtr<Object> const& obj) {
	return instanceof<T>(obj.get());
}

template <class T>
bool instanceof(UPtr<Object> const& obj) {
	return instanceof<T>(obj.get());
}

template <class T>
bool instanceof(Object const& obj) {
	return instanceof<T>(&obj);
}

} // namespace slib

namespace std {
	// for using Object as key in unordered_map
	template<> struct hash<slib::Object> {
		std::size_t operator()(const slib::Object& obj) const {
			return (size_t)obj.hashCode();
		}
	};
}

#endif // H_SLIB_OBJECT_H
