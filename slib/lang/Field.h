/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_FIELD_H
#define H_SLIB_FIELD_H

#include "slib/lang/StringView.h"
#include "slib/lang/String.h"
#include "slib/lang/Numeric.h"
#include "slib/util/TemplateUtils.h"
#include "slib/exception/IllegalArgumentException.h"
#include "slib/third-party/variant-lite/variant.hpp"

namespace slib {

namespace internal {

enum class RefType {
	INSTANCE,
	SPTR,
	UPTR
};

enum class InnerRefType {
	INSTANCE,
	SPTR
};

typedef enum {
	NONE = 0,
	VOID =		1 << 0,
	NUMBER =	1 << 1,
	_INT64 =	1 << 2,
	_LONG =		1 << 3,
	INT64 = NUMBER | _INT64,
	LONG = NUMBER | _LONG,
	_DOUBLE =	1 << 4,
	DOUBLE = NUMBER | _DOUBLE
} FieldValType;

struct FieldValue {
	FieldValType _type;
	nonstd::variant<void *, int64_t, Long, double, Double> _val;
public:
	FieldValue()
	: _type(FieldValType::NONE) {}
};

int64_t _fieldGetLong(FieldValue const& val);

template <class S>
void *_fieldBox(void *src, FieldValue &dst, Class const& dstType) {
	Class const& srcType = classOf<S>::_class();

	if (dstType.isAssignableFrom(srcType)) {
		if (hasTypeInfo<S> {}) {
			TypedClass *tVal = (TypedClass *)src;
			void *castVal = tVal->__classCast(dstType.getTypeId());
			if (castVal) {
				dst._type = FieldValType::VOID;
				dst._val = castVal;
				return castVal;
			}
		}
		THROW(IllegalArgumentException);
	} else {
		FieldValue fieldValue;

		if (classOf<Number>::_class().isAssignableFrom(srcType)) {
			if (classOf<Long>::_class().isAssignableFrom(srcType)) {
				fieldValue._type = FieldValType::INT64;
				fieldValue._val = (static_cast<Long *>(src))->longValue();
			}
		} else if (srcType.isPrimitive()) {
			if (classOf<int64_t>::_class().isAssignableFrom(srcType)) {
				fieldValue._type = FieldValType::INT64;
				fieldValue._val = *(static_cast<int64_t *>(src));
			}
		}

		if ((fieldValue._type & FieldValType::NUMBER) != 0) {
			if (classOf<Number>::_class().isAssignableFrom(dstType)) {
				if (classOf<Long>::_class().isAssignableFrom(dstType)) {
					dst._type = FieldValType::LONG;
					dst._val = Long(_fieldGetLong(fieldValue));
					return &dst._val;
				}
			} else if (dstType.isPrimitive()) {
				if (classOf<int64_t>::_class().isAssignableFrom(dstType)) {
					dst._type = FieldValType::INT64;
					dst._val = _fieldGetLong(fieldValue);
					return &dst._val;
				}
			}
			THROW(IllegalArgumentException);
		}
	}
}

} // namespace internal

class Field {
protected:
	const StringView _name;	///< Field name
	const Class _fieldType;	///< Field type
	const Class _classType;	///< Class type
protected:
	virtual void *getAddr(void *instance) = 0;

	virtual void set(void *instance, void *valueRef, internal::InnerRefType valueRefType) = 0;

	Field(StringView const& name, Class const& fieldType, Class const& classType)
	: _name(name)
	, _fieldType(fieldType)
	, _classType(classType) {}

	template <typename T, typename C>
	void set(C& instance, void *valueRef, internal::RefType valueRefType) {
		if (classOf<C>::_class() != _classType)
			throw IllegalArgumentException(_HERE_);

		bool sameType = true;
		Class const& valueClass = classOf<T>::_class();
		if (valueClass != _fieldType) {
			/*if (!_fieldType.isAssignableFrom(valueClass))
				throw IllegalArgumentException(_HERE_);*/
			sameType = false;
		}

		if (valueRefType == internal::RefType::SPTR) {
			// we want to keep a SPtr for possible copy
			if (sameType)
				set(&instance, valueRef, internal::InnerRefType::SPTR);
			else {
				SPtr<T> *pSVal = (SPtr<T> *)valueRef;
				T *pVal = pSVal->get();
				/*if (hasTypeInfo<T> {}) {
					TypedClass *tVal = (TypedClass *)pVal;
					void *castVal = tVal->__classCast(_fieldType.getTypeId());
					if (castVal) {
						SPtr<void> castSPtr(*pSVal, castVal);
						set(&instance, &castSPtr, internal::InnerRefType::SPTR);
					} else
						THROW(IllegalArgumentException);
				} else
					throw IllegalArgumentException(_HERE_);*/
				internal::FieldValue tmpVal;
				void *castVal = internal::_fieldBox<T>(pVal, tmpVal, _fieldType);
				SPtr<void> castSPtr(*pSVal, castVal);
				set(&instance, &castSPtr, internal::InnerRefType::SPTR);
			}
		} else {
			if (valueRefType == internal::RefType::UPTR) {
				UPtr<T> *pUVal = (UPtr<T> *)valueRef;
				valueRef = pUVal->get();
			}
			internal::FieldValue tmpVal;
			if (!sameType) {
				/*if (hasTypeInfo<T> {}) {
					TypedClass *tVal = (TypedClass *)valueRef;
					void *castVal = tVal->__classCast(_fieldType.getTypeId());
					if (castVal)
						valueRef = castVal;
					else
						throw IllegalArgumentException(_HERE_);
				} else
					throw IllegalArgumentException(_HERE_);*/
				void *castVal = internal::_fieldBox<T>(valueRef, tmpVal, _fieldType);
				valueRef = castVal;
			}
			set(&instance, valueRef, internal::InnerRefType::INSTANCE);
		}
	}
public:
	virtual ~Field() {}

	StringView const& getName() const {
		return _name;
	}

	Class const& getGenericType() const {
		return _fieldType;
	}

	template<typename T, typename C>
	T& get(C& instance) {
		if (classOf<T>::_class() != _fieldType)
			throw IllegalArgumentException(_HERE_);
		return *(static_cast<T*>(getAddr(&instance)));
	}

	template<typename T, typename C> void set(C& instance, T const& value) {
		set<T>(instance, (void*)&value, internal::RefType::INSTANCE);
	}

	template<typename T, typename C> void set(C& instance, SPtr<T> const& value) {
		set<T>(instance, (void *)&value, internal::RefType::SPTR);
	}

	template<typename T, typename C> void set(C& instance, UPtr<T> const& value) {
		set<T>(instance, (void *)&value, internal::RefType::UPTR);
	}
};

template<typename F>
class SpecificField;

template<typename T, typename C>
class SpecificField<T (C::*)> : public Field {
private:
	T C::* _field;
protected:;
	virtual void *getAddr(void *instance) override {
		return &(static_cast<C*>(instance)->*_field);
	}

	virtual void set(void *instance, void *valueRef, internal::InnerRefType valueRefType SLIB_UNUSED) override {
		void *fieldAddr =  &(static_cast<C *>(instance)->*_field);
		*(static_cast<T *>(fieldAddr)) = *((T *)valueRef);
	}
public:
	SpecificField(StringView const& name, T (C::* field))
	: Field(name, classOf<T>::_class(), classOf<C>::_class())
	, _field(field) {}
};

template<typename T, typename C>
class SpecificField<SPtr<T> (C::*)> : public Field {
private:
	SPtr<T> C::* _field;
protected:;
	virtual void *getAddr(void *instance) override {
		return &(static_cast<C*>(instance)->*_field);
	}

	virtual void set(void *instance, void *valueRef, internal::InnerRefType valueRefType) override {
		void *fieldAddr =  &(static_cast<C *>(instance)->*_field);
		switch (valueRefType) {
			case internal::InnerRefType::INSTANCE: {
				SPtr<T> *psp = (static_cast<SPtr<T> *>(fieldAddr));
				*psp = newS<T>(*(T *)valueRef);
				break;
			}
			case internal::InnerRefType::SPTR:
				*(static_cast<SPtr<T> *>(fieldAddr)) = *((SPtr<T> *)valueRef);
				break;
		}
	}
public:
	SpecificField(StringView const& name, SPtr<T> (C::* field))
	: Field(name, classOf<T>::_class(), classOf<C>::_class())
	, _field(field) {}
};

template<typename T, typename C>
class SpecificField<UPtr<T> (C::*)> : public Field {
private:
	UPtr<T> C::* _field;
protected:;
	virtual void *getAddr(void *instance) override {
		return &(static_cast<C*>(instance)->*_field);
	}

	virtual void set(void *instance, void *valueRef, internal::InnerRefType valueRefType SLIB_UNUSED) override {
		void *fieldAddr =  &(static_cast<C *>(instance)->*_field);
		UPtr<T> *pup = (static_cast<UPtr<T> *>(fieldAddr));
		*pup = newU<T>(*(T *)valueRef);
	}
public:
	SpecificField(StringView const& name, UPtr<T> (C::* field))
	: Field(name, classOf<T>::_class(), classOf<C>::_class())
	, _field(field) {}
};

} // namespace slib

#endif // H_SLIB_FIELD_H

