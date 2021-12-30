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
#include "slib/exception/IllegalAccessException.h"
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
	_P_INT64 =	1 << 2,
	_LONG =		1 << 3,
	P_INT64 =	NUMBER | _P_INT64,
	LONG =		NUMBER | _LONG,
	_P_DOUBLE =	1 << 4,
	_DOUBLE =	1 << 5,
	P_DOUBLE =	NUMBER | _P_DOUBLE,
	DOUBLE =	NUMBER | _DOUBLE,
	P_BOOL =	1 << 6,
	BOOLEAN =	1 << 7
} FieldValType;

struct FieldValue {
	FieldValType _type;
	nonstd::variant<void *, int64_t, Long, double, Double, bool, Boolean> _val;
public:
	FieldValue()
	: _type(FieldValType::NONE) {}
};

} // namespace internal

class Field {
protected:
	const StringView _name;	///< Field name
	const Class _fieldType;	///< Field type
	const Class _classType;	///< Class type
protected:
	static void *autoBox(void *src, Class const& srcType, internal::FieldValue &dst, Class const& dstType);

	template <class T>
	static Class const& valueClass(void *valueRef, internal::RefType valueRefType) {
		Class const& clazz = classOf<T>::_class();
		if (clazz.isAssignableFrom(classOf<Object>::_class())) {
			switch (valueRefType) {
				case internal::RefType::SPTR: {
					SPtr<Object> *pSVal = (SPtr<Object> *)valueRef;
					return (*pSVal)->getClass();
				}
				case internal::RefType::UPTR: {
					UPtr<Object> *pUVal = (UPtr<Object> *)valueRef;
					return (*pUVal)->getClass();
				}
				case internal::RefType::INSTANCE: {
					Object *pVal = (Object *)valueRef;
					return pVal->getClass();
				}
			}
		} else
			return clazz;

		return classOf<Void>::_class();
	}
protected:
	virtual void *getAddr(void *instance) = 0;

	virtual void set(void *instance, void *valueRef, internal::InnerRefType valueRefType) = 0;

	Field(StringView const& name, Class const& fieldType, Class const& classType)
	: _name(name)
	, _fieldType(fieldType)
	, _classType(classType) {}

	template <typename T>
	void set(ObjRef const& instance, void *valueRef, internal::RefType valueRefType) {
		if (instance._class != _classType)
			throw IllegalArgumentException(_HERE_);

		Class const& valueClass = Field::valueClass<T>(valueRef, valueRefType);
		bool sameType = (valueClass == _fieldType);

		if (valueRefType == internal::RefType::SPTR) {
			// we want to keep a SPtr for possible copy
			if (sameType)
				set(instance._ref, valueRef, internal::InnerRefType::SPTR);
			else {
				SPtr<T> *pSVal = (SPtr<T> *)valueRef;
				T *pVal = pSVal->get();
				internal::FieldValue tmpVal;
				void *castVal = autoBox(pVal, valueClass, tmpVal, _fieldType);
				SPtr<void> castSPtr(*pSVal, castVal);
				set(instance._ref, &castSPtr, internal::InnerRefType::SPTR);
			}
		} else {
			if (valueRefType == internal::RefType::UPTR) {
				UPtr<T> *pUVal = (UPtr<T> *)valueRef;
				valueRef = pUVal->get();
			}
			internal::FieldValue tmpVal;
			if (!sameType) {
				void *castVal = autoBox(valueRef, valueClass, tmpVal, _fieldType);
				valueRef = castVal;
			}
			set(instance._ref, valueRef, internal::InnerRefType::INSTANCE);
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
		void *fieldAddr = getAddr(&instance);

		if (classOf<T>::_class() == _fieldType)
			return *(static_cast<T*>(fieldAddr));

		/*if (hasTypeInfo<S> {}) {
			TypedClass *tVal = (TypedClass *)src;
			void *castVal = tVal->__classCast(dstType.getTypeId());
		}*/

		THROW(IllegalArgumentException);
	}

	template <typename C>
	ObjRef getRef(C& instance) {
		return ObjRef(getAddr(&instance), _fieldType);
	}

	ObjRef getRef(ObjRef instance) {
		return ObjRef(getAddr(instance._ref), _fieldType);
	}

	template<typename T, typename C> void set(C& instance, T const& value) {
		set<T>(ObjRef(&instance, classOf<C>::_class()), (void*)&value, internal::RefType::INSTANCE);
	}

	template<typename T, typename C> void set(C& instance, SPtr<T> const& value) {
		set<T>(ObjRef(&instance, classOf<C>::_class()), (void *)&value, internal::RefType::SPTR);
	}

	template<typename T, typename C> void set(C& instance, UPtr<T> const& value) {
		set<T>(ObjRef(&instance, classOf<C>()), (void *)&value, internal::RefType::UPTR);
	}

	template<typename T> void set(ObjRef const& instance, T const& value) {
		set<T>(instance, (void*)&value, internal::RefType::INSTANCE);
	}

	template<typename T> void set(ObjRef const& instance, SPtr<T> const& value) {
		set<T>(instance, (void *)&value, internal::RefType::SPTR);
	}

	template<typename T> void set(ObjRef const& instance, UPtr<T> const& value) {
		set<T>(instance, (void *)&value, internal::RefType::UPTR);
	}
};

namespace internal {

template<typename T, typename C>
void setFieldInstance(void *fieldAddr, void *valueRef, std::true_type) {
	*(static_cast<T *>(fieldAddr)) = *((T *)valueRef);
}

template<typename T, typename C>
void setFieldInstance(void *, void *, std::false_type) {
	THROW(IllegalAccessException);
}

template<typename T, typename C>
void setFieldUPtr(void *fieldAddr, void *valueRef, std::true_type) {
	UPtr<T> *pup = static_cast<UPtr<T> *>(fieldAddr);
	*(*pup) = *((T *)valueRef);
}

template<typename T, typename C>
void setFieldUPtr(void *fieldAddr, void *valueRef, std::false_type) {
	UPtr<T> *pup = static_cast<UPtr<T> *>(fieldAddr);
	*pup = newU<T>(*((T *)valueRef));
}

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

	virtual void set(void *instance, void *valueRef, internal::InnerRefType valueRefType) override {
		void *fieldAddr =  &(static_cast<C *>(instance)->*_field);
		//*(static_cast<T *>(fieldAddr)) = *((T *)valueRef);
		switch (valueRefType) {
			case internal::InnerRefType::INSTANCE:
				internal::setFieldInstance<T, C>(fieldAddr, valueRef, std::is_assignable<T&, T>{});
				break;
			case internal::InnerRefType::SPTR: {
				SPtr<T> *ref = (SPtr<T> *)valueRef;
				internal::setFieldInstance<T, C>(fieldAddr, ref->get(), std::is_assignable<T&, T>{});
				break;
			}
		}
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

		switch (valueRefType) {
			case internal::InnerRefType::INSTANCE:
				internal::setFieldUPtr<T, C>(fieldAddr, valueRef, std::is_assignable<T&, T>{});
				break;
			case internal::InnerRefType::SPTR: {
				SPtr<T> *ref = (SPtr<T> *)valueRef;
				internal::setFieldUPtr<T, C>(fieldAddr, ref->get(), std::is_assignable<T&, T>{});
				break;
			}
		}
	}
public:
	SpecificField(StringView const& name, UPtr<T> (C::* field))
	: Field(name, classOf<T>::_class(), classOf<C>::_class())
	, _field(field) {}
};

} // namespace internal

} // namespace slib

#endif // H_SLIB_FIELD_H
