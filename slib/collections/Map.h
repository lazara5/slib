/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_MAP_H
#define H_SLIB_COLLECTIONS_MAP_H

#include "slib/lang/Object.h"
#include "slib/util/Iterator.h"
#include "slib/lang/StringBuilder.h"

#include <memory>

namespace slib {

template <class K, class V>
class ValueProvider {
public:
	BASE_TYPE_INFO(ValueProvider, CLASS(ValueProvider<K, V>));
public:
	virtual ~ValueProvider() {}

	virtual SPtr<V> get(const K& key) const = 0;
	virtual bool containsKey(const K& key) const = 0;
};

/**
 * An object that maps keys to values. A map cannot contain duplicate keys;
 * each key can map to at most one value.
 *
 * <p>Note: Mutable objects <b> should NOT </b> be used as map keys. The behavior
 * of a map is not specified if the value of an object is changed in a manner that 
 * affects \c equals comparisons while the object is a key in the map.
 *
 * @see HashMap
 * @see LinkedHashMap
 */
template <class K, class V,
		  class Pred = std::equal_to<K>>
class Map : virtual public Object, public ValueProvider<K, V> {
public:
	TYPE_INFO(Map, CLASS(Map<K, V, Pred>), CLASS(Object), CLASS(ValueProvider<K, V>));
public:
	class Entry {
	public:
		virtual const SPtr<K> getKey() const = 0;
		virtual K *getKeyPtr() const = 0;
		virtual const SPtr<V> getValue() const = 0;
		virtual V *getValuePtr() const = 0;
		virtual ~Entry() {}
	};

public:
	virtual ~Map() {}

	virtual SPtr<V> put(SPtr<K> const& key, SPtr<V> const& value) = 0;

	/*virtual SPtr<V> put(K const& key, SPtr<V> const& value) {
		return put(newS<K>(key), value);
	}

	template <class AVT, typename... A>
	SPtr<V> emplace(const K& key, A&&... args) {
		return put(newS<K>(key), newS<AVT>(std::forward<A>(args)...));
	}

	template <class AVT, typename... A>
	SPtr<V> emplaceValue(SPtr<K> const& key, A&&... args) {
		return put(key, newS<AVT>(std::forward<A>(args)...));
	}*/

	template <class AKT, class AVT, typename KAT, typename VAT>
	SPtr<V> emplace(KAT&& key_arg, VAT&& value_arg) {
		static_assert(std::is_base_of<K, AKT>::value, "");
		static_assert(std::is_base_of<V, AVT>::value, "");
		return put(newS<AKT>(std::forward<KAT>(key_arg)), newS<AVT>(std::forward<VAT>(value_arg)));
	}

	template <class AKT, typename KAT, typename AVT>
	SPtr<V> emplaceKey(KAT&& key_arg, SPtr<AVT> const& value) {
		static_assert(std::is_base_of<K, AKT>::value, "");
		static_assert(std::is_base_of<V, AVT>::value, "");
		return put(newS<AKT>(std::forward<KAT>(key_arg)), value);
	}

	virtual SPtr<V> get(K const& key) const override = 0;
	virtual V *getPtr(K const& key) const = 0;
	virtual Entry const *getEntry(const K& key) const = 0;
	virtual SPtr<V> remove(const K& key) = 0;

	virtual bool containsKey(const K& key) const override = 0;

	virtual size_t size() const = 0;
	virtual bool isEmpty() const = 0;

	virtual void clear() = 0;

	virtual UPtr<String> toString() const override {
		UPtr<ConstIterator<Entry>> i = constIterator();
		if (!i->hasNext())
			return "{}"_UPTR;

		StringBuilder sb;
		sb.add('{');
		do {
			Entry const& e = i->next();

			K const* key = e.getKeyPtr();
			if (instanceof<Map>(key) && Class::constCast<Map>(key) == this)
				sb.add("(this Map)");
			else
				sb.add(slib::toString(key));

			sb.add('=');

			const V *value = e.getValuePtr();
			if (instanceof<Map>(value) && Class::constCast<Map>(value) == this)
				sb.add("(this Map)");
			else
				sb.add(slib::toString(value));

			if (!i->hasNext())
				return sb.add('}').toString();
			sb.add(", ");
		} while (true);
	}

public:
	virtual UPtr<ConstIterator<Entry>> constIterator() const = 0;
};

} // namespace

#endif // H_SLIB_COLLECTIONS_MAP_H
