/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_CLASS_H
#define H_SLIB_CLASS_H

#include "slib/exception/NullPointerException.h"
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

namespace slib {

class ClassCastException : public Exception {
public:
	ClassCastException(const char *where, const char *c1, const char *c2);
};

template<typename ...TS>
using typelist = qcstudio::mpml::typelist<TS...>;

template<typename TYPE, typename TYPELIST>
using get_ancestors_t = qcstudio::mpml::get_ancestors_t<TYPE, TYPELIST>;

template <typename ... T>
struct inherits;

template <typename C>
struct inherits<C> {
	typedef typename cat<typelist<C>, typename C::_classInherits>::type types;
};

template <typename C, typename ... Cs>
struct inherits<C, Cs... > {
	typedef typename cat<typelist<C>, typename C::_classInherits, typename inherits<Cs...>::types>::type types;
};

template <typename ... T>
struct hierarchy;

template <typename C>
struct hierarchy<C> {
	typedef typename cat<typelist<C>, typename C::_classInherits>::type types;
};

template <typename C, typename ... Cs>
struct hierarchy<C, Cs... > {
	typedef typename cat<typelist<C>, typename C::_classInherits, typename hierarchy<Cs...>::types>::type types;
};

//Template class that holds the declaration of the id.
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
	static constexpr Array<TypeData, LENGTH - INDEX> exec() {
		using T = qcstudio::mpml::at_t<INDEX, TYPELIST>;
		return join(Array<TypeData, 1> {{{_typeId<T>()}}},
					_typelistIterator<C, TYPELIST, LENGTH, INDEX + 1>::exec());
	}
};

// This is the hierarchy iterator when we are at the end of the type-list
template <class C, typename TYPELIST, unsigned LENGTH>
struct _typelistIterator<C, TYPELIST, LENGTH, LENGTH> {
	static constexpr Array<TypeData, 0> exec() {
		return Array<TypeData, 0> {};
	}
};

struct _unknown {
};

template <class C>
constexpr size_t _typeDescSize() {
	using ch = get_ancestors_t<C, typename hierarchy<C>::types>;
	return ch::size;
}

template <>
constexpr size_t _typeDescSize<_unknown>() {
	return 0;
}

template <class C>
constexpr Array<TypeData, _typeDescSize<C>()> _typeDesc() {
	using ch = get_ancestors_t<C, typename hierarchy<C>::types>;
	return _typelistIterator<C, get_ancestors_t<C, ch>>::exec();
}

class Class {
protected:
	const StringView _name;
	const size_t _hDepth;
	const TypeData *_typeStack;
public:
	constexpr Class(StringView const& name, size_t hDepth, const TypeData* typeStack)
	: _name(name)
	, _hDepth(hDepth)
	, _typeStack(typeStack) {
	}

	constexpr Class(Class const& clazz)
	: _name(clazz._name)
	, _hDepth(clazz._hDepth)
	, _typeStack(clazz._typeStack) {}

	Class& operator=(Class const&) = delete;

	bool operator ==(Class const& other) const {
		// TODO: do we need to compare depth?
		return (_hDepth == other._hDepth) && _typeStack[_hDepth - 1].typeId == other._typeStack[_hDepth - 1].typeId;
	}

	constexpr StringView const& getName() const {
		return _name;
	}

	constexpr TypeId getTypeId() const {
		return _typeStack[_hDepth - 1].typeId;
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
};

template <typename T>
struct classOf {
	static Class const& _class() {
		static constexpr auto _typeData {_typeDesc<T>()};
		static constexpr Class _class{T::_className, _typeDescSize<T>(), _typeData.data()};
		return _class;
	}
};

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
		void const* res = const_cast<void const*>(const_cast<S *>(from)->_classCast(_typeId<D>()));
		if (res)
			return (D const*) res;

		throw ClassCastException(_HERE_, classOf<S>::_class().getName().c_str(), classOf<D>::_class().getName().c_str());
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
	return constCastImpl<D, S>(from, std::is_base_of<Object, S>{});
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

		throw ClassCastException(_HERE_, classOf<S>::_class().getName().c_str(), classOf<D>::_class().getName().c_str());
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

		throw ClassCastException(_HERE_, classOf<V>::_class().getName().c_str(), classOf<T>::_class().getName().c_str());
	}

	return nullptr;
}*/

template <class D, class S>
D* Class::cast(S *from) {
	if (from != nullptr) {
		void *res = from->_classCast(_typeId<D>());
		if (res)
			return (D*) res;

		throw ClassCastException(_HERE_, classOf<S>::_class().getName().c_str(), classOf<D>::_class().getName().c_str());
	}

	return nullptr;
}

#define CLASS(...) (__VA_ARGS__)
#define INHERITS(...) (__VA_ARGS__)
#define UNPAREN(...) __VA_ARGS__

#define DO_CHECK(X, I) if ((ptr = UNPAREN X::_classCast(typeId))) return ptr;
#define DO_LIST(X, I) UNPAREN X COMMA_IF(I)

#define BASE_TYPE_INFO(NAME, TYPE) \
	static constexpr StringView _className { #NAME ## _SV}; \
	typedef typelist<> _classInherits; \
	virtual Class const& getClass() const { \
		return classOf<UNPAREN TYPE>::_class(); \
	} \
	virtual void *_classCast(TypeId typeId) { \
		if (typeId == _typeId<UNPAREN TYPE>()) \
			return this; \
		return nullptr; \
	}

#define TYPE_INFO(NAME, TYPE, ...) \
	static constexpr StringView _className { #NAME ## _SV }; \
	typedef typename inherits<FOR_EACH(DO_LIST, __VA_ARGS__)>::types _classInherits; \
	virtual Class const& getClass() const override { \
		return classOf<UNPAREN TYPE>::_class(); \
	} \
	virtual void *_classCast(TypeId typeId) override { \
		if (typeId == _typeId<UNPAREN TYPE>()) return this; \
		void *ptr; \
		FOR_EACH(DO_CHECK, __VA_ARGS__) \
		return nullptr; \
	}

class Void {
public:
	BASE_TYPE_INFO(Void, CLASS(Void));
};

} // namespace slib

#endif // H_SLIB_CLASS_H
