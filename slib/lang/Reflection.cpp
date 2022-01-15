/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/Reflection.h"

namespace slib {

UPtr<Array<Field>> ReflectionInfo::getFields() {
	UPtr<Array<Field>> fields = newU<Array<Field>>(_fields->size());
	size_t idx = 0;
	auto i = _fields->constIterator();
	for (auto &fieldEntry : *i) {
		(*fields)[idx++] = fieldEntry.getValue();
	}
	return fields;
}

SPtr<Field> ReflectionInfo::getField(StringView const& name) {
	return _fields->get(name);
}

namespace reflect {

ObjRef Array::getRef(ObjRef const& array, size_t index) {
	if (!array._ref)
		THROW(NullPointerException);
	if (!array._class.isArray())
		THROW(IllegalArgumentException);
	IArray *arrayInstance = (IArray *)array.getInstanceRef();
	return arrayInstance->getRef(index);
}

void Array::setRef(ObjRef& array, size_t index, ObjRef const& value) {
	if (!array._ref)
		THROW(NullPointerException);
	if (!array._class.isArray())
		THROW(IllegalArgumentException);
	IArray *arrayInstance = (IArray *)array.getInstanceRef();
	arrayInstance->setRef(index, value);
}

void Array::resize(IArray& array, size_t newSize) {
	array.resize(newSize);
}

void Array::resize(ObjRef const& array, size_t newSize) {
	if (!array._class.isArray())
		THROW(IllegalArgumentException);

	IArray *arrayInstance = (IArray *)array.getInstanceRef();
	arrayInstance->resize(newSize);
}

bool Array::hasIndex(ObjRef const& array, size_t index) {
	if (!array._class.isArray())
		THROW(IllegalArgumentException);

	IArray *arrayInstance = (IArray *)array.getInstanceRef();
	return arrayInstance->hasIndex(index);
}

} // namespace reflect

} // namespace slib