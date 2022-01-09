/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_REFLECTION_H
#define H_SLIB_REFLECTION_H

#include "slib/lang/Class.h"
#include "slib/collections/HashMap.h"
#include "slib/lang/Field.h"
#include "slib/lang/StringView.h"

namespace slib {

class ReflectionInfo {
private:
	UPtr<Map<StringView, Field>> _fields;
public:
	typedef void (*registerReflection)(ReflectionInfo &);

	ReflectionInfo(registerReflection reg)
	: _fields(newU<HashMap<StringView, Field>>()) {
		reg(*this);
	}

	template <typename F>
	void registerField(StringView const& name, F field) {
		_fields->put(newS<StringView>(name), newS<internal::SpecificField<F>>(name, field));
	}

	UPtr<Array<Field>> getFields();

	SPtr<Field> getField(StringView const& name);
};

class NoSuchFieldException : public Exception {
public:
	NoSuchFieldException(const char *where, const char *msg)
	:Exception(where, "NoSuchFieldException", msg) {
	}
};

} // namespace slib

#define STRSV(str) #str ## _SV
#define SV(str) STRSV(str)

#define REFLECT(clazz) \
static void __registerReflection(slib::ReflectionInfo &ri); \
static slib::ReflectionInfo *__getReflectionInfo() { \
	static slib::ReflectionInfo ri(&__registerReflection<clazz>); \
	return &ri; \
} \
template<typename T> \
static void __registerReflection(slib::ReflectionInfo &ri)

#define FIELD(name) { using namespace slib; ri.registerField(SV(name), &T::name); }

namespace slib {

namespace reflect {

struct Array {
	static ObjRef getRef(ObjRef const& array, size_t index);
	static void resize(IArray& array, size_t newSize);
	static void resize(ObjRef const& array, size_t newSize);
};

} // namespace reflect

} // namespace slib

#endif // H_SLIB_REFLECTION_H
