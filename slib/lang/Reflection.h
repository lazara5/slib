/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_REFLECTION_H
#define H_SLIB_REFLECTION_H

#include "slib/collections/HashMap.h"
#include "slib/lang/Field.h"
#include "slib/lang/StringView.h"

namespace slib {

class ReflectionInfo : public ClassReflectionInfo {
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
		_fields->put(newS<StringView>(name), newS<SpecificField<F>>(name, field));
	}
};

} // namespace slib

#define STRSV(str) #str ## _SV
#define SV(str) STRSV(str)

#define REFLECT(clazz) \
static void _registerReflection(slib::ReflectionInfo &ri); \
static slib::ClassReflectionInfo *_getReflectionInfo() { \
	static slib::ReflectionInfo ri(&_registerReflection<clazz>); \
	return &ri; \
} \
template<typename T> \
static void _registerReflection(slib::ReflectionInfo &ri)

#define FIELD(name) { using namespace slib; ri.registerField(SV(name), &T::name); }

#endif // H_SLIB_REFLECTION_H
