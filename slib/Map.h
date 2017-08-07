/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_MAP_H
#define H_SLIB_MAP_H

#include "Iterator.h"

#include <memory>

namespace slib {

/**
 * An object that maps keys to values. A map cannot contain duplicate keys;
 * each key can map to at most one value.
 *
 * <p>Note: Mutable objects <b> should NOT</b> be used as map keys. The behavior 
 * of a map is not specified if the value of an object is changed in a manner that 
 * affects <tt>equals</tt> comparisons while the object is a key in the map.
 *
 * @see HashMap
 * @see LinkedHashMap
 */
template <class K, class V,
		  class Pred = std::equal_to<K> >
class Map {
public:
	class Entry {
	public:
		virtual const K& getKey() const = 0;
		virtual const std::shared_ptr<V> getValue() const = 0;
		virtual ~Entry() {}
	};

public:
	virtual ~Map() {}

	virtual std::shared_ptr<V> put(const K& key, const V& value) = 0;
	virtual std::shared_ptr<V> put(const K& key, const std::shared_ptr<V> value) = 0;
	virtual std::shared_ptr<V> get(const K& key) const = 0;
	virtual const Entry *getEntry(const K& key) const = 0;
	virtual std::shared_ptr<V> remove(const K& key) = 0;

	virtual bool containsKey(const K& key) const = 0;

	virtual int size() const = 0;
	virtual bool isEmpty() const = 0;

	virtual void clear() = 0;
public:
	virtual ConstIterator<Entry> constIterator() const = 0;
};

} // namespace

#endif // H_SLIB_MAP_H
