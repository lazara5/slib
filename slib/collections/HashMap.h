/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_HASHMAP_H
#define H_SLIB_COLLECTIONS_HASHMAP_H



#include "slib/collections/Map.h"
#include "slib/lang/Numeric.h"
#include "slib/exception/IllegalStateException.h"

#include <inttypes.h>

#include <functional>
#include <memory>

namespace slib {

#define HASH_DEFAULT_LOAD_FACTOR (0.75f)

template <class K, class V, class Pred = std::equal_to<K> >
class InternalHashMap {
template <class K1, class V1, class Pred1> friend class HashMap;
public:
	static const int32_t DEFAULT_INITIAL_CAPACITY = 16;
	static const int32_t MAXIMUM_CAPACITY = 1 << 30;
public:
	class Entry : public Map<K, V, Pred>::Entry {
	template <class K1, class V1, class Pred1> friend class InternalHashMap;
	template <class K1, class V1, class Pred1> friend class InternalLinkedHashMap;
	protected:
		SPtr<K> _key;
		SPtr<V> _value;
		Entry *_next;
		const int32_t _keyHash;
	public:
		Entry(int32_t hash, const K& k, const V& v, Entry *n)
		: _key(newS<K>(k))
		, _value(newS<V>(v))
		, _keyHash(hash) {
			_next = n;
		}

		Entry(int32_t hash, SPtr<K> const& k, SPtr<V> const& v, Entry *n)
		: _key(k)
		, _value(v)
		, _keyHash(hash) {
			_next = n;
		}

		/** in-place constructor */
		Entry(Entry *n, int32_t hash, SPtr<K> const& k, SPtr<V> const& v)
		:_key(std::move(k))
		,_value(v)
		,_keyHash(hash) {
			_next = n;
		}

		virtual const SPtr<K> getKey() const {
			return _key;
		}

		virtual K *getKeyPtr() const {
			return _key.get();
		}

		virtual const SPtr<V> getValue() const {
			return _value;
		}

		virtual V *getValuePtr() const {
			return _value.get();
		}

		int32_t hashCode() const {
			return (sizeTHash(std::hash<K>()(*_key)) ^ (_value ? 0 : sizeTHash(std::hash<V>(*_value))));
		}

		virtual void onRemove(InternalHashMap *) {}

		virtual ~Entry() {}
	};
protected:
	Entry **_table;
	int32_t _tableLength;
	size_t _size;
	size_t _threshold;
	float _loadFactor;
protected:
	/**
	 * Applies a supplemental hash function to a given hashCode, which
	 * defends against poor quality hash functions. This is critical
	 * because this HashMap implementation uses power-of-two length
	 * hash tables that would otherwise encounter collisions for hash codes
	 * that do not differ in lower bits.
	 */
	static int32_t _smudge(int32_t h) {
		// Ensures that hashCodes that differ only by constant
		// multiples at each bit position have a bounded number
		// of collisions (approximately 8 at the default load factor).
		uint32_t uh = *(uint32_t*)&h;
		h ^= (uh >> 20) ^ (uh >> 12);
		uint32_t sh = uh ^ (uh >> 7) ^ (uh >> 4);
		return *(int32_t*)&sh;
	}

	/** Returns the index for hash code h. */
	static int32_t indexFor(int32_t h, int32_t length) {
		// note: only works when len is power of two !
		return h & (length - 1);
	}

	/** Transfers all entries from current table to newTable */
	void transfer(Entry** newTable, int32_t newTableLength) {
		Entry** src = _table;
		int32_t newCapacity = newTableLength;
		for (int32_t j = 0; j < _tableLength; j++) {
			Entry* e = src[j];
				if (e != nullptr) {
				src[j] = nullptr;
				do {
					Entry* next = e->_next;
					int i = indexFor(e->_keyHash, newCapacity);
					e->_next = newTable[i];
					newTable[i] = e;
					e = next;
				} while (e != nullptr);
			}
		}
	}

	void resize(int32_t newCapacity) {
		int32_t oldCapacity = _tableLength;
		if (oldCapacity == MAXIMUM_CAPACITY) {
			_threshold = Integer::MAX_VALUE;
			return;
		}

		Entry** newTable = (Entry**)malloc(newCapacity * sizeof(Entry*));
		if (!newTable)
			throw OutOfMemoryError(_HERE_);
		memset(newTable, 0, newCapacity * sizeof(Entry*));
		transfer(newTable, newCapacity);
		free(_table);
		_table = newTable;
		_tableLength = newCapacity;
		_threshold = (int)(newCapacity * _loadFactor);
	}

	virtual void addEntry(int32_t hash, SPtr<K> const& key, SPtr<V> value, int bucketIndex) {
		Entry* e = _table[bucketIndex];
		_table[bucketIndex] = new Entry(hash, key, value, e);
		if (_size++ >= _threshold)
			resize(2 * _tableLength);
	}

	virtual void emplaceEntry(int32_t hash, SPtr<K> const& key, SPtr<V> value, int bucketIndex) {
		Entry* e = _table[bucketIndex];
		//printf("EE\n");
		_table[bucketIndex] = new Entry(e, hash, key, value);
		if (_size++ >= _threshold)
			resize(2 * _tableLength);
	}

	/**
	 * Removes and returns the entry associated with the specified key
	 * in the HashMap. Returns DefunctEntry(null) if there is no mapping
	 * for this key.
	 */
	SPtr<V> removeEntryForKey(const K& key) {
		int32_t hash = _smudge(std::hash<K>()(key));
		int32_t i = indexFor(hash, _tableLength);
		Entry* prev = _table[i];
		Entry* e = prev;
		Pred eq;

		while (e != nullptr) {
			Entry* next = e->_next;
			if ((e->_keyHash == hash) && (eq(key, *(e->_key)))) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->_next = next;
				e->onRemove(this);
				SPtr<V> oldVal = std::move(e->_value);
				delete e;
				return oldVal;
			}
			prev = e;
			e = next;
		}

		return nullptr;
	}

	/**
	 * Removes the entry associated with the specified key
	 * in the HashMap.
	 */
	void eraseEntryForKey(const K& key) {
		int32_t hash = _smudge(std::hash<K>()(key));
		int32_t i = indexFor(hash, _tableLength);
		Entry* prev = _table[i];
		Entry* e = prev;
		Pred eq;

		while (e != nullptr) {
			Entry* next = e->_next;
			if ((e->_keyHash == hash) && (eq(key, e->_key))) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->_next = next;
				e->onRemove(this);
				delete e;
				return;
			}
			prev = e;
			e = next;
		}
	}
public:
	InternalHashMap(size_t initialCapacity = DEFAULT_INITIAL_CAPACITY, float loadFactor = HASH_DEFAULT_LOAD_FACTOR) {
		if (initialCapacity > MAXIMUM_CAPACITY)
			initialCapacity = MAXIMUM_CAPACITY;

		// Find a power of 2 >= initialCapacity
		size_t capacity = 1;
		while (capacity < initialCapacity)
			capacity <<= 1;

		_loadFactor = loadFactor;
		_threshold = (int)(capacity * loadFactor);
		_table = (Entry**)malloc(capacity * sizeof(Entry*));
		if (!_table)
			throw OutOfMemoryError(_HERE_);
		_tableLength = capacity;
		memset(_table, 0, capacity * sizeof(Entry*));
		_size = 0;
	}

	InternalHashMap(InternalHashMap const& other) {
		_loadFactor = other._loadFactor;
		_threshold = other._threshold;
		_tableLength = other._tableLength;
		_table = (Entry**)malloc(_tableLength * sizeof(Entry*));
		if (!_table)
			throw OutOfMemoryError(_HERE_);
		memset(_table, 0, _tableLength * sizeof(Entry*));
		_size = 0;

		copyFrom(other);
	}

	virtual ~InternalHashMap() {
		if (_table != nullptr) {
			clear();
			free(_table);
		}
		_table = nullptr;
	}

	/** Removes all mappings from this map. */
	virtual void clear() {
		for (int32_t i = 0; i <_tableLength; i++) {
			Entry *e = _table[i];
			while (e != nullptr) {
				Entry *next = e->_next;
				delete e;
				e = next;
			}
		}
		memset(_table, 0, _tableLength * sizeof(Entry*));
		_size = 0;
	}

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of mappings in this map
	 */
	size_t size() const {
		return _size;
	}

	/**
	 * Returns <i>true</i> if this map contains no key-value mappings.
	 *
	 * @return <i>true</i> if this map contains no mappings
	 */
	bool isEmpty() const {
		return (_size == 0);
	}

	/**
	 * Returns the value to which the specified key is mapped
	 * or a <i>'NULL'</i> reference if this map contains no mapping for the key.
	 * More formally, if this map contains a mapping from a key
	 * <i>k</i> to a value <i>v</i> such that key.isNull() ? k.isNull() : key == k,
	 * then this method returns <i>v</i>, otherwise it returns  a <i>'NULL'</i> reference
	 *  (There can be at most one such mapping.)
	 * A <i>'NULL'</i> reference as a return value does not necessarily indicate that the
	 * map contains no mapping for the key; it is also possible that the map explicitly maps
	 * the key to a <i>'NULL'</i> reference.
	 * HashMap::containsKey may be used to distinguish between these two cases.
	 */
	virtual SPtr<V> get(K const& key) const {
		const typename Map<K, V>::Entry *e = getEntry(key);
		if (e)
			return e->getValue();
		return nullptr;
	}

	virtual V *getPtr(K const& key) const {
		const typename Map<K, V>::Entry *e = getEntry(key);
		if (e)
			return e->getValuePtr();
		return nullptr;
	}

	const typename Map<K, V>::Entry *getEntry(const K& key) const {
		int32_t hash = _smudge(sizeTHash(std::hash<K>()(key)));
		Pred eq;
		for (const Entry *e = _table[indexFor(hash, _tableLength)]; e != nullptr; e = e->_next) {
			if ((e->_keyHash == hash) && (eq(*(e->_key), key)))
				return e;
		}
		return nullptr;
	}

	/**
	 * Returns <i>true</i> if this map contains a mapping for the specified key.
	 * @param key The key whose presence in this map is to be tested
	 * @return <i>true</i> if this map contains a mapping for the specified key.
	 */
	bool containsKey(const K& key) const {
		int32_t hash = _smudge(sizeTHash(std::hash<K>()(key)));
		Pred eq;
		for (Entry *e = _table[indexFor(hash, _tableLength)]; e != nullptr; e = e->_next) {
			if ((e->_keyHash == hash) && (eq(*(e->_key), key)))
				return true;
		}
		return false;
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old value is replaced.
	 * @param key  key with which the value is to be associated
	 * @param value  value to be associated with the key
	 * @return the previous value associated with <i>key</i> or
	 *		a <i>'NULL'</i> reference if there was no mapping for <i>key</i>.
	 *		(A <i>'NULL'</i> reference as a return value can also indicate that the map
	 *		previously associated a <i>'NULL'</i> reference with <i>key</i>.)
	 */
	virtual SPtr<V> put(SPtr<K> const& key, SPtr<V> const& value) {
		int32_t hash = _smudge(sizeTHash(std::hash<K>()(*key)));
		int32_t i = indexFor(hash, _tableLength);
		//fmt::print("->h: {}->{}, if: {}\n", sizeTHash(std::hash<K>()(key)), hash, i);
		Pred eq;
		for (Entry *e = _table[i]; e != nullptr; e = e->_next) {
			if ((e->_keyHash == hash) && (eq(*(e->_key), *key))) {
				SPtr<V> oldValue = std::move(e->_value);
				e->_value = value;
				return oldValue;
			}
		}
		addEntry(hash, key, value, i);
		return nullptr;
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old value is replaced.
	 * @param key  key with which the value is to be associated
	 * @param value  value to be associated with the key
	 * @return the previous value associated with <i>key</i> or
	 *		a <i>'NULL'</i> reference if there was no mapping for <i>key</i>.
	 *		(A <i>'NULL'</i> reference as a return value can also indicate that the map
	 *		previously associated a <i>'NULL'</i> reference with <i>key</i>.)
	 */
	void insert(const K& key, const V& value) {
		int32_t hash = _smudge(sizeTHash(std::hash<K>()(key)));
		int32_t i = indexFor(hash, _tableLength);
		Pred eq;
		for (Entry *e = _table[i]; e != nullptr; e = e->_next) {
			if ((e->_keyHash == hash) && (eq(e->_key, key))) {
				e->_value = value;
				return;
			}
		}
		addEntry(hash, key, value, i);
	}

	SPtr<V> remove(const K& key) {
		return removeEntryForKey(key);
	}

	/**
	 * Copies all mappings from <i>other</i> to this map. Does <b>not</b> clear
	 * this map beforehand.
	 */
	void copyFrom(const InternalHashMap& other) {
		for (int32_t i = 0; i < other._tableLength; i++) {
			Entry *e = other._table[i];
			while (e != nullptr) {
				put(e->_key, e->_value);
				e = e->_next;
			}
		}
	}

	virtual void forEach(bool (*callback)(void*, SPtr<K> const&, const SPtr<V>&), void *data) const {
		for (int32_t i = 0; i < _tableLength; i++) {
			Entry *e = _table[i];
			while (e != nullptr) {
				bool cont = callback(data, e->_key, e->_value);
				if (!cont)
					return;
				e = e->_next;
			}
		}
	}

	virtual void forEach(std::function<bool(SPtr<K> const&, const SPtr<V>&)> callback) const {
		for (int32_t i = 0; i < _tableLength; i++) {
			Entry *e = _table[i];
			while (e != nullptr) {
				bool cont = callback(e->_key, e->_value);
				if (!cont)
					return;
				e = e->_next;
			}
		}
	}
protected:
	// iterator
	class ConstEntryIterator : public ConstIteratorImpl<typename Map<K, V, Pred>::Entry> {
	private:
		SPtr<InternalHashMap<K, V, Pred>> _map;
	protected:
		int32_t _index;
		typename InternalHashMap<K, V, Pred>::Entry *_current, *_next;
	protected:
		ConstEntryIterator(ConstEntryIterator* other) {
			_map = other->_map;
			_index = other->_index;
			_current = other->_current;
			_next = other->_next;
		}
	public:
		ConstEntryIterator(SPtr<InternalHashMap> const& map) {
			_map = map;
			_next = nullptr;
			_index = 0;
			_current = nullptr;
			if (_map->_size > 0) {
				// go to first entry
				while (_index < _map->_tableLength && (_next = _map->_table[_index++]) == nullptr);
			}
		}

		virtual bool hasNext() override {
			return (_next != nullptr);
		}

		virtual const typename Map<K, V>::Entry& next() override {
			typename InternalHashMap<K, V, Pred>::Entry *e = _next;
			if (e == nullptr)
				throw NoSuchElementException(_HERE_);
			if ((_next = e->_next) == nullptr) {
				while (_index < _map->_tableLength && (_next = _map->_table[_index++]) == nullptr);
			}
			_current = e;
			return *e;
		}

		virtual ConstIteratorImpl<typename Map<K, V, Pred>::Entry> *clone() override {
			return new ConstEntryIterator(this);
		}
	};

	class EntryIterator : public IteratorImpl<typename Map<K, V, Pred>::Entry>, public ConstEntryIterator {
	private:
		SPtr<InternalHashMap<K, V, Pred>> _ncMap;
	protected:
		EntryIterator(EntryIterator* other)
		: ConstEntryIterator(other) {
			_ncMap = other->_ncMap;
		}
	public:
		EntryIterator(SPtr<InternalHashMap> const& map)
		: ConstEntryIterator(map) {
			_ncMap = map;
		}

		virtual bool hasNext() override {
			return ConstEntryIterator::hasNext();
		}

		virtual const typename Map<K, V>::Entry& next() override {
			return ConstEntryIterator::next();
		}

		virtual void remove() {
			if (this->_current == NULL)
				throw IllegalStateException(_HERE_);
			SPtr<K> const& k = this->_current->_key;
			_ncMap->removeEntryForKey(*k);
		}

		virtual IteratorImpl<typename Map<K, V, Pred>::Entry> *clone() override {
			return new EntryIterator(this);
		}
	};
};

/** Hash table based object that maps keys to values */
template <class K, class V, class Pred = std::equal_to<K>>
class HashMap : public Map<K, V, Pred> {
public:
	TYPE_INFO(HashMap, CLASS(HashMap<K, V, Pred>), CLASS(Map<K, V, Pred>));
public:
	static const int32_t DEFAULT_INITIAL_CAPACITY = InternalHashMap<K, V, Pred>::DEFAULT_INITIAL_CAPACITY;
	static const int32_t MAXIMUM_CAPACITY = InternalHashMap<K, V, Pred>::MAXIMUM_CAPACITY;
protected:
	SPtr<InternalHashMap<K, V, Pred>> _internalMap;

	virtual InternalHashMap<K, V, Pred> *internalMap() {
		return _internalMap.get();
	}

	virtual InternalHashMap<K, V, Pred> const* constInternalMap() const {
		return _internalMap.get();
	}
	HashMap(SPtr<InternalHashMap<K, V, Pred>> internalMap)
	: _internalMap(internalMap) {}
public:
	HashMap(size_t initialCapacity = DEFAULT_INITIAL_CAPACITY, float loadFactor = HASH_DEFAULT_LOAD_FACTOR)
	: HashMap(newS<InternalHashMap<K, V, Pred>>(initialCapacity, loadFactor)) {}

	HashMap(const HashMap& other)
	: HashMap(newS<InternalHashMap<K, V, Pred>>(other._internalMap)) {}

	HashMap(std::initializer_list<std::pair<K, SPtr<V>>> args) {
		put(args);
	}

	/** Removes all mappings from this map. */
	virtual void clear() override {
		internalMap()->clear();
	}

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of mappings in this map
	 */
	size_t size() const override {
		return constInternalMap()->size();
	}

	/**
	 * Returns <i>true</i> if this map contains no key-value mappings.
	 *
	 * @return <i>true</i> if this map contains no mappings
	 */
	virtual bool isEmpty() const {
		return constInternalMap()->isEmpty();
	}

	/**
	 * Returns the value to which the specified key is mapped
	 * or a <i>'NULL'</i> reference if this map contains no mapping for the key.
	 * More formally, if this map contains a mapping from a key
	 * <i>k</i> to a value <i>v</i> such that key.isNull() ? k.isNull() : key == k, 
	 * then this method returns <i>v</i>, otherwise it returns  a <i>'NULL'</i> reference
	 *  (There can be at most one such mapping.)
	 * A <i>'NULL'</i> reference as a return value does not necessarily indicate that the 
	 * map contains no mapping for the key; it is also possible that the map explicitly maps 
	 * the key to a <i>'NULL'</i> reference.
	 * HashMap::containsKey may be used to distinguish between these two cases.
	 */
	virtual SPtr<V> get(K const& key) const override {
		return constInternalMap()->get(key);
	}

	virtual V *getPtr(K const& key) const override {
		return constInternalMap()->getPtr(key);
	}

	virtual const typename Map<K, V>::Entry *getEntry(const K& key) const override {
		return constInternalMap()->getEntry(key);
	}

	/**
	 * Returns <i>true</i> if this map contains a mapping for the specified key.
	 * @param key The key whose presence in this map is to be tested
	 * @return <i>true</i> if this map contains a mapping for the specified key.
	 */
	virtual bool containsKey(const K& key) const override {
		return constInternalMap()->containsKey(key);
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old value is replaced.
	 * @param key  key with which the value is to be associated
	 * @param value  value to be associated with the key
	 * @return the previous value associated with <i>key</i> or
	 *		a <i>'NULL'</i> reference if there was no mapping for <i>key</i>.
	 *		(A <i>'NULL'</i> reference as a return value can also indicate that the map
	 *		previously associated a <i>'NULL'</i> reference with <i>key</i>.)
	 */
	virtual SPtr<V> put(SPtr<K> const& key, SPtr<V> const& value) override {
		return internalMap()->put(key, value);
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old value is replaced.
	 * @param key  key with which the value is to be associated
	 * @param value  value to be associated with the key
	 * @return the previous value associated with <i>key</i> or
	 *		a <i>'NULL'</i> reference if there was no mapping for <i>key</i>.
	 *		(A <i>'NULL'</i> reference as a return value can also indicate that the map
	 *		previously associated a <i>'NULL'</i> reference with <i>key</i>.)
	 */
	void insert(const K& key, const V& value) {
		internalMap()->insert(key, value);
	}

	/*template<class K1, class V1>
	void emplace(K1&& k, V1&& v) {
		V value(std::forward<V1>(v));
		emplace(k, newS<V1>(std::move(value)));
	}

	template<class K1, class V1>
	void emplace(K1&& k, SPtr<V1> v) {
		K key(std::forward<K1>(k));
		int hash = _smudge(std::hash<K>()(key));
		int i = indexFor(hash, _tableLength);
		Pred eq;
		for (Entry *e = _table[i]; e != nullptr; e = e->_next) {
			if ((e->_hash == hash) && (eq(e->_key, key))) {
				e->_value = v;
				return;
			}
		}
		emplaceEntry(hash, key, v, i);
	}*/

	void put(std::initializer_list<std::pair<const K, V>> args) {
		for (auto i = args.begin(); i != args.end(); ++i)
			put(i->first, newS<V>(i->second));
	}

	void put(std::initializer_list<std::pair<const K, SPtr<V>>> args) {
		for (auto i = args.begin(); i != args.end(); ++i)
			put(i->first, i->second);
	}

	/**
	 * Removes the mapping for the specified key from this map if present.
	 * @param key  key to be removed from the map
	 * @return the previous value associated with <i>key</i> or
	 *		a <i>'NULL'</i> reference if there was no mapping for <i>key</i>.
	 *		(A <i>'NULL'</i> reference as a return can also indicate that the map
	 *		previously associated a <i>'NULL'</i> reference with <i>key</i>.)
	 */
	virtual SPtr<V> remove(const K& key) override {
		return internalMap()->remove(key);
	}

	void erase(const K& key) {
		internalMap()->eraseEntryForKey(key);
	}

	/** 
	 * Copies all mappings from <i>other</i> to this map. Does <b>not</b> clear
	 * this map beforehand.
	 */
	virtual void copyFrom(const HashMap& other) {
		_internalMap->copyFrom(*other._internalMap);
	}

	void forEach(bool (*callback)(void*, const K&, const V&), void *data) const {
		_internalMap->forEach(callback, data);
	}

	void forEach(std::function<bool(const K&, SPtr<V> const&)> callback) const {
		_internalMap->forEach(callback);
	}

public:
	virtual UPtr<ConstIterator<typename Map<K, V, Pred>::Entry>> constIterator() const {
		return newU<ConstIteratorWrapper<typename Map<K, V, Pred>::Entry>>(new typename InternalHashMap<K, V, Pred>::ConstEntryIterator(_internalMap));
	}

	virtual UPtr<Iterator<typename Map<K, V, Pred>::Entry>> iterator() {
		return newU<IteratorWrapper<typename Map<K, V, Pred>::Entry>>(new typename InternalHashMap<K, V, Pred>::EntryIterator(_internalMap));
	}
};

} // namespace slib

#endif // H_SLIB_COLLECTIONS_HASHMAP_H
