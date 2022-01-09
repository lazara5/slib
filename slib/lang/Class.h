/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_CLASS_H
#define H_SLIB_CLASS_H

#include "slib/exception/NullPointerException.h"
#include "slib/exception/IllegalStateException.h"
#include "slib/lang/StringView.h"
#include "slib/util/TemplateUtils.h"
#include "slib/util/MacroUtils.h"

#include "slib/third-party/mpml/mpml.h"

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>

#include <string>
#include <atomic>
#include <memory>
#include <cstddef>

namespace slib {

class ClassCastException : public Exception {
public:
	ClassCastException(const char *where, StringView const& c1, StringView const& c2);
};

class InstantiationException : public Exception {
public:
	InstantiationException(const char *where, const char *msg)
	: Exception(where, "ClassCastException", msg) {}
};

template <typename T, typename = void>
struct hasTypeInfo : std::false_type{};

template <typename T>
struct hasTypeInfo<T, decltype((void)T::__className, void())> : std::true_type {};
//struct hasTypeInfo<T, decltype((void)T::__classCast, void())> : std::true_type {};

template<typename ...TS>
using typelist = qcstudio::mpml::typelist<TS...>;

template<typename TYPE, typename TYPELIST>
using get_ancestors_t = qcstudio::mpml::get_ancestors_t<TYPE, TYPELIST>;

template <typename ... T>
struct inherits;

template <typename C>
struct inherits<C> {
	typedef typename cat<typelist<C>, typename C::__classInherits>::type types;
};

template <typename C, typename ... Cs>
struct inherits<C, Cs... > {
	typedef typename cat<typelist<C>, typename C::__classInherits, typename inherits<Cs...>::types>::type types;
};

template <typename ... T>
struct hierarchy;

template <typename C>
struct hierarchy<C> {
	typedef typename cat<typelist<C>, typename C::__classInherits>::type types;
};

template <typename C, typename ... Cs>
struct hierarchy<C, Cs... > {
	typedef typename cat<typelist<C>, typename C::__classInherits, typename hierarchy<Cs...>::types>::type types;
};

// Template class that holds the declaration of the type id
template<typename T>
struct typeIdMarker {
	static const T* const id;
};

typedef const void* TypeId;

// Definition of the type id
template<typename T>
const T* const typeIdMarker<T>::id = nullptr;

template <typename T>
constexpr TypeId _typeId() noexcept {
	return &typeIdMarker<T>::id;
}

typedef struct {
	TypeId typeId;
} TypeData;

// This is the hierarchy iterator general case
template<class C, typename TYPELIST, unsigned LENGTH = TYPELIST::size, unsigned INDEX = 0>
struct _typelistIterator {
	static_assert(qcstudio::mpml::is_typelist<TYPELIST>::value, "Not a typelist");
	static constexpr StaticArray<TypeData, LENGTH - INDEX> exec() {
		using T = qcstudio::mpml::at_t<INDEX, TYPELIST>;
		return join(StaticArray<TypeData, 1> {{{_typeId<T>()}}},
					_typelistIterator<C, TYPELIST, LENGTH, INDEX + 1>::exec());
	}
};

// This is the hierarchy iterator when we are at the end of the type-list
template <class C, typename TYPELIST, unsigned LENGTH>
struct _typelistIterator<C, TYPELIST, LENGTH, LENGTH> {
	static constexpr StaticArray<TypeData, 0> exec() {
		return StaticArray<TypeData, 0> {};
	}
};

template <class C>
constexpr size_t _typeDescSize(std::false_type) {
	return 1;
}

template <class C>
constexpr size_t _typeDescSize(std::true_type) {
	using ch = get_ancestors_t<C, typename hierarchy<C>::types>;
	return ch::size;
}

template <typename C>
constexpr StaticArray<TypeData, 1> _typeDesc(std::false_type) {
	return StaticArray<TypeData, 1> {{{_typeId<C>()}}};
}

template <typename C>
constexpr StaticArray<TypeData, 1> _primitiveTypeDesc() {
	return StaticArray<TypeData, 1> {{{_typeId<C>()}}};
}

template <typename C>
constexpr StaticArray<TypeData, _typeDescSize<C>(hasTypeInfo<C>{})> _typeDesc(std::true_type) {
	using ch = get_ancestors_t<C, typename hierarchy<C>::types>;
	return _typelistIterator<C, get_ancestors_t<C, ch>>::exec();
}

template<int = sizeof(uintptr_t)>
int32_t uintptrTHash(uintptr_t h);

template<>
int32_t uintptrTHash<8>(size_t h);

template<>
int32_t uintptrTHash<4>(size_t h);

class ReflectionInfo;

typedef ReflectionInfo *(*GetReflectionInfo)(void);

class Field;

template <class T, bool p>
class Array;

class Class;

enum class RefType : uint8_t {
	INSTANCE,
	SPTR,
	UPTR
};

typedef void (*ObjRefDeleter)(void *ref, RefType refType);
typedef void *(*GetInstanceRef)(void *ref, RefType refType);

namespace internal {
template <typename T>
void *getInstanceRef(void *ref, RefType refType) {
	if (!ref)
		return nullptr;

	switch (refType) {
		case RefType::INSTANCE:
			return ref;
		case RefType::SPTR:
			return ((SPtr<T> *)ref)->get();
		case RefType::UPTR:
			return ((UPtr<T> *)ref)->get();
			break;
	}

	return nullptr;
}
} // namespace internal

class ObjRef {
public:
	void *_ref;
	RefType _refType;
	Class const& _class;
private:
	ObjRefDeleter _deleter;
public:
	ObjRef(void *ref, RefType refType, Class const& objClass,
		   ObjRefDeleter deleter = nullptr)
	: _ref(ref)
	, _refType(refType)
	, _class(objClass)
	, _deleter(deleter) {}

	ObjRef(ObjRef&& other)
	: _ref(other._ref)
	, _refType(other._refType)
	, _class(other._class)
	, _deleter(other._deleter) {
		other._ref = nullptr;
	}

	ObjRef& operator =(ObjRef const&) = delete;

	~ObjRef() {
		if (_deleter && _ref) {
			_deleter(_ref, _refType);
		}
	}

	void *getInstanceRef() const;
};

typedef Class const& (*GetClassRef)();
typedef ObjRef (*NewInstance)(RefType refType);

class Class {
friend class ObjRef;
protected:
	const StringView _name;						///< Class name
	const TypeData *_typeStack;					///< Class hierarchy
	const GetReflectionInfo _reflectionInfo;	///< Reflection info
	GetClassRef _getArrayComponentClass;		///< Component class getter for array types
	NewInstance _newInstance;
	const size_t _hDepth : 16;					///< Class hierarchy depth
	bool _isPrimitive : 1;
	bool _isArray : 1;
public:
	bool _hasTypeInfo : 1;
protected:
	SPtr<Field> _getDeclaredField(StringView const& name) const;
protected:
	// ObjRef helpers
	GetInstanceRef _objRefGetInstance;
public:
	constexpr Class(StringView const& name, size_t hDepth,
					const TypeData* typeStack, const GetReflectionInfo reflectionInfo,
					bool isPrimitive, bool isArray,
					GetClassRef getArrayComponentClass, NewInstance newInstance,
					GetInstanceRef objRefGetInstance,
					bool hasTypeInfo)
	: _name(name)
	, _typeStack(typeStack)
	, _reflectionInfo(reflectionInfo)
	, _getArrayComponentClass(getArrayComponentClass)
	, _newInstance(newInstance)
	, _hDepth(hDepth)
	, _isPrimitive(isPrimitive)
	, _isArray(isArray)
	, _hasTypeInfo(hasTypeInfo)
	, _objRefGetInstance(objRefGetInstance) {}

	constexpr Class(Class const& other)
	: _name(other._name)
	, _typeStack(other._typeStack)
	, _reflectionInfo(other._reflectionInfo)
	, _getArrayComponentClass(other._getArrayComponentClass)
	, _newInstance(other._newInstance)
	, _hDepth(other._hDepth)
	, _isPrimitive(other._isPrimitive)
	, _isArray(other._isArray)
	, _hasTypeInfo(other._hasTypeInfo)
	, _objRefGetInstance(other._objRefGetInstance) {}

	Class& operator=(Class const&) = delete;

	bool operator ==(Class const& other) const {
		if (this == &other)
			return true;
		// TODO: do we need to compare depth?
		return (_hDepth == other._hDepth) && _typeStack[_hDepth - 1].typeId == other._typeStack[_hDepth - 1].typeId;
	}

	bool operator !=(Class const& other) const {
		return !(*this == other);
	}

	int32_t hashCode() const {
		return uintptrTHash((uintptr_t)getTypeId());
	}

	constexpr StringView const& getName() const {
		return _name;
	}

	constexpr TypeId getTypeId() const {
		return _typeStack[_hDepth - 1].typeId;
	}

	constexpr bool isPrimitive() const {
		return _isPrimitive;
	}

	constexpr bool isArray() const {
		return _isArray;
	}

	constexpr Class const& getComponentClass() const {
		return _getArrayComponentClass();
	}

	ObjRef newInstance(RefType refType = RefType::INSTANCE) const {
		return _newInstance(refType);
	}

	bool isAssignableFrom(Class const& cls) const {
		if ((_hDepth == 0) || (cls._hDepth == 0)) {
			return false;
		}
		TypeId myTypeId = _typeStack[_hDepth - 1].typeId;
		for (size_t i = 0; i < cls._hDepth; i++) {
			TypeId typeId = cls._typeStack[i].typeId;
			if (typeId == myTypeId) {
				return true;
			}
		}
		return false;
	}

	template <class D, class S>
	static D* cast(S *from);

	template <class D, class S>
	static D const* constCast(S const* from);

	template <class D, class S>
	static SPtr<D> cast(SPtr<S> const& from);

	template <class D, class S>
	static D* castPtr(SPtr<S> const& from) {
		if (!from)
			return nullptr;
		return cast<D>(from.get());
	}

	template <class D, class S>
	static D* castPtr(UPtr<S> const& from) {
		if (!from)
			return nullptr;
		return cast<D>(from.get());
	}

	UPtr<Array<Field, false>> getDeclaredFields();

	template <class S>
	SPtr<Field> getDeclaredField(S const& name) const {
		StringView fieldName(strData(CPtr(name)), strLen(CPtr(name)));
		return _getDeclaredField(fieldName);
	}
};

template <typename T>
constexpr StringView _className(std::false_type) {
	return "<unknown>"_SV;
}

template <typename T>
constexpr StringView _className(std::true_type) {
	return T::__className;
}

template <typename T, typename = void>
struct hasReflectionInfo : std::false_type{};

template <typename T>
struct hasReflectionInfo<T, decltype(T::__getReflectionInfo(), void())> : std::true_type {};

template <typename T>
constexpr GetReflectionInfo _getReflectionInfo(std::false_type) {
	return nullptr;
}

template <typename T>
constexpr GetReflectionInfo _getReflectionInfo(std::true_type) {
	return T::__getReflectionInfo;
}

template <typename T, typename = void>
struct isDefaultConstructible : std::false_type{};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
template <typename T>
struct isDefaultConstructible<T, decltype(T(), void())> : std::true_type {};
#pragma GCC diagnostic pop

template <typename T>
struct classOf {
	static Class const& getArrayComponentClass() {
		THROW(IllegalStateException, "Not an array");
	}

	static ObjRef noNewInstance(RefType) {
		THROW(InstantiationException, "No default constructor available");
	}

	static void deleter(void *ref, RefType refType) {
		switch (refType) {
			case RefType::INSTANCE:
				delete (T*)ref;
				break;
			case RefType::SPTR:
				delete (SPtr<T>*)ref;
				break;
			case RefType::UPTR:
				delete (UPtr<T>*)ref;
				break;
		}
	}

	static ObjRef newInstance(RefType refType) {
		switch (refType) {
			case RefType::INSTANCE:
				return ObjRef(new T(), refType, classOf<T>::_class(), deleter);
			case RefType::SPTR:
				return ObjRef(new std::shared_ptr<T>(newS<T>()),
							  refType, classOf<T>::_class(), deleter);
			case RefType::UPTR:
				return ObjRef(new std::unique_ptr<T>(new T()), refType, classOf<T>::_class(), deleter);
		}
		return ObjRef(nullptr, refType, classOf<T>::_class());
	}

	static constexpr NewInstance _getNewInstance(std::false_type) {
		return noNewInstance;
	}

	static constexpr NewInstance _getNewInstance(std::true_type) {
		return newInstance;
	}

	static Class const& _class() {
		static constexpr auto _typeData {_typeDesc<T>(hasTypeInfo<T>{})};
		static constexpr GetReflectionInfo _reflectionInfo = _getReflectionInfo<T>(hasReflectionInfo<T>{});
		static constexpr NewInstance _newInstance = _getNewInstance(isDefaultConstructible<T>{});
		static constexpr Class _class{_className<T>(hasTypeInfo<T>{}),
									  _typeDescSize<T>(hasTypeInfo<T>{}), _typeData.data(), _reflectionInfo,
									  false, false, &getArrayComponentClass, _newInstance,
									  &internal::getInstanceRef<T>,
									  toBool(hasTypeInfo<T>{})};

		return _class;
	}
};

template <typename E, bool p>
struct classOf<Array<E, p>> {
	static Class const& getArrayComponentClass() {
		return classOf<E>::_class();
	}

	static Class const& _class() {
		static constexpr auto _typeData {_typeDesc<Array<E, p>>(hasTypeInfo<Array<E, p>>{})};
		static constexpr GetReflectionInfo _reflectionInfo = _getReflectionInfo<Array<E, p>>(hasReflectionInfo<Array<E, p>>{});
		static constexpr Class _class{_className<Array<E, p>>(hasTypeInfo<Array<E, p>>{}),
									  _typeDescSize<Array<E, p>>(hasTypeInfo<Array<E, p>>{}), _typeData.data(), _reflectionInfo,
									  false, true, &getArrayComponentClass, nullptr,
									  &internal::getInstanceRef<Array<E, p>>,
									  toBool(hasTypeInfo<Array<E, p>>{})};

		return _class;
	}
};

#define PRIMITIVECLASSOF(TYPE, NAME) \
template <> \
struct classOf<TYPE> { \
	static Class const& getArrayComponentClass() { \
		THROW(IllegalStateException, "Not an array"); \
	} \
	static ObjRef newInstance(RefType) { \
		THROW(InstantiationException, "");\
	} \
	static Class const& _class() { \
		static constexpr auto _typeData {_primitiveTypeDesc<TYPE>()}; \
		static constexpr Class _class{NAME ## _SV, 1, _typeData.data(), nullptr, true, false, \
			&getArrayComponentClass, &newInstance, &internal::getInstanceRef<TYPE>, false}; \
		return _class; \
	} \
}

PRIMITIVECLASSOF(int64_t, "long");
PRIMITIVECLASSOF(uint64_t, "ulong");
PRIMITIVECLASSOF(int32_t, "int");
PRIMITIVECLASSOF(uint32_t, "uint");
PRIMITIVECLASSOF(int16_t, "short");
PRIMITIVECLASSOF(uint16_t, "ushort");
PRIMITIVECLASSOF(float, "float");
PRIMITIVECLASSOF(double, "double");
PRIMITIVECLASSOF(bool, "boolean");

/*template <class D, class V>
D const* Class::constCast(V const* from) {
	if (from != nullptr) {
		// Check if it is convertible to the base class or to itself
		if (std::is_convertible<typename std::remove_pointer<V*>::type, typename std::remove_pointer<D*>::type>::value == true)
			return dynamic_cast<D const*>(from);

		bool canCast = classOf<D>::_class().isAssignableFrom(from->getClass());
		if (canCast) {
			D const* ret =  dynamic_cast<D const*>(from);
			if (ret)
				return ret;
		}

		throw ClassCastException(_HERE_, classOf<V>::_class().getName().c_str(), classOf<D>::_class().getName().c_str());
	}

	return nullptr;
}*/

template <class D, class S>
D const* constCastImpl(S const* from, std::true_type) {
	if (from != nullptr) {
		void const* res = const_cast<void const*>(const_cast<S *>(from)->__classCast(_typeId<D>()));
		if (res)
			return (D const*) res;

		throw ClassCastException(_HERE_, classOf<S>::_class().getName(), classOf<D>::_class().getName());
	}

	return nullptr;
}

template <class D, class S>
D const* constCastImpl(S const* from, std::false_type) {
	if (from != nullptr) {
		throw ClassCastException(_HERE_, "?", "?");
	}

	return nullptr;
}

class Object;

template <class D, class S>
D const* Class::constCast(S const* from) {
	//return constCastImpl<D, S>(from, std::is_base_of<Object, S>{});
	return constCastImpl<D, S>(from, hasTypeInfo<S>{});
}

template <class D, class S>
SPtr<D> Class::cast(SPtr<S> const& from) {
	if (from) {
		// Check if it is convertible to the base class or to itself
		if (std::is_convertible<typename std::remove_pointer<S*>::type, typename std::remove_pointer<D*>::type>::value == true)
			return std::dynamic_pointer_cast<D>(from);

		bool canCast = classOf<D>::_class().isAssignableFrom(from->getClass());
		if (canCast) {
			D* ret = dynamic_cast<D*>(from.get());
			if (ret)
				return SPtr<D>(from, ret);
		}

		throw ClassCastException(_HERE_, classOf<S>::_class().getName(), classOf<D>::_class().getName());
	}

	return nullptr;
}

/*template <class T, class V>
T* Class::cast(V *from) {
	if (from != nullptr) {
		// Check if it is convertible to the base class or to itself
		if (std::is_convertible<typename std::remove_pointer<V*>::type, typename std::remove_pointer<T*>::type>::value == true)
			return dynamic_cast<T*>(from);

		bool canCast = classOf<T>::_class().isAssignableFrom(from->getClass());
		if (canCast) {
			T* ret = dynamic_cast<T*>(from);
			if (ret)
				return ret;
		}

		throw ClassCastException(_HERE_, classOf<V>::_class().getName(), classOf<T>::_class().getName());
	}

	return nullptr;
}*/

template <class D, class S>
D* Class::cast(S *from) {
	if (from != nullptr) {
		void *res = from->__classCast(_typeId<D>());
		if (res)
			return (D*) res;

		THROW(ClassCastException, classOf<S>::_class().getName(), classOf<D>::_class().getName());
	}

	return nullptr;
}

class TypedClass {
public:
	virtual ~TypedClass() {}

	virtual Class const& getClass() const = 0;
	virtual void *__classCast(TypeId typeId) = 0;
};

#define CLASS(...) (__VA_ARGS__)
#define INHERITS(...) (__VA_ARGS__)
#define UNPAREN(...) __VA_ARGS__

#define DO_CHECK(X, I) if ((ptr = UNPAREN X::__classCast(typeId))) return ptr;
#define DO_LIST(X, I) UNPAREN X COMMA_IF(I)

#define BASE_TYPE_INFO(NAME, TYPE) \
	static constexpr StringView __className { #NAME ## _SV}; \
	typedef typelist<> __classInherits; \
	virtual Class const& getClass() const override { \
		return classOf<UNPAREN TYPE>::_class(); \
	} \
	virtual void *__classCast(TypeId typeId) override { \
		if (typeId == _typeId<UNPAREN TYPE>()) \
			return this; \
		return nullptr; \
	}

#define TYPE_INFO(NAME, TYPE, ...) \
	static constexpr StringView __className { #NAME ## _SV }; \
	typedef typename inherits<FOR_EACH(DO_LIST, __VA_ARGS__)>::types __classInherits; \
	virtual Class const& getClass() const override { \
		return classOf<UNPAREN TYPE>::_class(); \
	} \
	virtual void *__classCast(TypeId typeId) override { \
		if (typeId == _typeId<UNPAREN TYPE>()) return this; \
		void *ptr; \
		FOR_EACH(DO_CHECK, __VA_ARGS__) \
		return nullptr; \
	}

class Void : virtual public TypedClass {
public:
	BASE_TYPE_INFO(Void, CLASS(Void));
};

} // namespace slib

namespace std {
	// for using Class as key in unordered_map
	template<> struct hash<slib::Class> {
		std::size_t operator()(const slib::Class& obj) const {
			return (size_t)obj.hashCode();
		}
	};
}

#endif // H_SLIB_CLASS_H
