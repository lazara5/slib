/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_FIELD_H
#define H_SLIB_FIELD_H

#include "slib/lang/StringView.h"
#include "slib/lang/String.h"
#include "slib/util/TemplateUtils.h"
#include "slib/exception/IllegalArgumentException.h"

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
			if (!_fieldType.isAssignableFrom(valueClass))
				throw IllegalArgumentException(_HERE_);
			sameType = false;
		}

		if (valueRefType == internal::RefType::SPTR) {
			// we want to keep a SPtr for possible copy
			if (sameType)
				set(&instance, valueRef, internal::InnerRefType::SPTR);
			else {
				SPtr<T> *pSVal = (SPtr<T> *)valueRef;
				T *pVal = pSVal->get();
				if (hasTypeInfo<T> {}) {
					TypedClass *tVal = (TypedClass *)pVal;
					void *castVal = tVal->_classCast(_fieldType.getTypeId());
					if (castVal) {
						SPtr<void> castSPtr(*pSVal, castVal);
						set(&instance, &castSPtr, internal::InnerRefType::SPTR);
					} else
						throw IllegalArgumentException(_HERE_);
				} else
					throw IllegalArgumentException(_HERE_);
			}
		} else {
			if (valueRefType == internal::RefType::UPTR) {
				UPtr<T> *pUVal = (UPtr<T> *)valueRef;
				valueRef = pUVal->get();
			}
			if (!sameType) {
				if (hasTypeInfo<T> {}) {
					TypedClass *tVal = (TypedClass *)valueRef;
					void *castVal = tVal->_classCast(_fieldType.getTypeId());
					if (castVal)
						valueRef = castVal;
					else
						throw IllegalArgumentException(_HERE_);
				} else
					throw IllegalArgumentException(_HERE_);
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

