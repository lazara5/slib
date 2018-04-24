/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_HASHMAP_H
#define H_SLIB_COLLECTIONS_HASHMAP_H

#include <functional>

#include "slib/collections/Map.h"
#include "slib/Numeric.h"
#include "slib/exception/IllegalStateException.h"

namespace slib {

#define HASH_DEFAULT_LOAD_FACTOR (0.75f)

/** Hash table based object that maps keys to values */
template <class K, class V, class Pred = std::equal_to<K> >
class HashMap : public Map<K, V, Pred> {
public:
	class Entry : public Map<K, V, Pred>::Entry {
	friend class HashMap;
	template <class K1, class V1, class Pred1> friend class LinkedHashMap;
	protected:
		const K _key;
		std::shared_ptr<V> _value;
		Entry *_next;
		const int _hash;
	public:
		Entry(int h, const K& k, const V& v, Entry *n)
		: _key(k)
		, _value(std::make_shared<V>(v))
		, _hash(h) {
			_next = n;
		}

		Entry(int h, const K& k, std::shared_ptr<V> v, Entry *n)
		: _key(k)
		, _value(v)
		, _hash(h) {
			_next = n;
		}

		/** in-place constructor */
		Entry(Entry *n, int h, K& k, std::shared_ptr<V> v)
		:_key(std::move(k))
		,_value(v)
		,_hash(h) {
			_next = n;
		}

		virtual const K& getKey() const {
			return _key;
		}

		virtual const std::shared_ptr<V> getValue() const {
			return _value;
		}

		int32_t hashCode() const {
			return (std::hash<K>()(_key) ^ (_value ? 0 : std::hash<V>(*_value)));
		}

		virtual void onRemove(HashMap *) {}

		virtual ~Entry() {}
	};
public:
	static const int DEFAULT_INITIAL_CAPACITY = 16;
	static const int MAXIMUM_CAPACITY = 1 << 30;
protected:
	Entry **_table;
	int _tableLength;
	ssize_t _size;
	int _threshold;
	float _loadFactor;
protected:
	/**
	 * Applies a supplemental hash function to a given hashCode, which
	 * defends against poor quality hash functions. This is critical
	 * because this HashMap implementation uses power-of-two length 
	 * hash tables that would otherwise encounter collisions for hash codes 
	 * that do not differ in lower bits.
	 */
	static int _smudge(int h) {
		// Ensures that hashCodes that differ only by constant 
		// multiples at each bit position have a bounded number 
		// of collisions (approximately 8 at the default load factor).
		h ^= ((unsigned int)h >> 20) ^ ((unsigned int)h >> 12);
		return h ^ ((unsigned int)h >> 7) ^ ((unsigned int)h >> 4);
	}

	/** Returns the index for hash code h. */
	static int indexFor(int h, int length) {
		return h & (length-1);
	}

	/** Transfers all entries from current table to newTable */
	void transfer(Entry** newTable, int newTableLength) {
		Entry** src = _table;
		int newCapacity = newTableLength;
		for (int j = 0; j < _tableLength; j++) {
			Entry* e = src[j];
				if (e != nullptr) {
				src[j] = nullptr;
				do {
					Entry* next = e->_next;
					int i = indexFor(e->_hash, newCapacity);
					e->_next = newTable[i];
					newTable[i] = e;
					e = next;
				} while (e != nullptr);
			}
		}
	}

	void resize(int newCapacity) {
		int oldCapacity = _tableLength;
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

	virtual void addEntry(int hash, const K& key, std::shared_ptr<V> value, int bucketIndex) {
		Entry* e = _table[bucketIndex];
		_table[bucketIndex] = new Entry(hash, key, value, e);
		if (_size++ >= _threshold)
			resize(2 * _tableLength);
	}

	virtual void emplaceEntry(int hash, K& key, std::shared_ptr<V> value, int bucketIndex) {
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
	std::shared_ptr<V> removeEntryForKey(const K& key) {
		int hash = _smudge(std::hash<K>()(key));
		int i = indexFor(hash, _tableLength);
		Entry* prev = _table[i];
		Entry* e = prev;
		Pred eq;

		while (e != nullptr) {
			Entry* next = e->_next;
			if ((e->_hash == hash) && (eq(key, e->_key))) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->_next = next;
				e->onRemove(this);
				std::shared_ptr<V> oldVal = e->_value;
				delete e;
				return oldVal;
			}
			prev = e;
			e = next;
		}

		return std::shared_ptr<V>();
	}

	/**
	 * Removes the entry associated with the specified key
	 * in the HashMap.
	 */
	void eraseEntryForKey(const K& key) {
		int hash = _smudge(std::hash<K>()(key));
		int i = indexFor(hash, _tableLength);
		Entry* prev = _table[i];
		Entry* e = prev;
		Pred eq;

		while (e != nullptr) {
			Entry* next = e->_next;
			if ((e->_hash == hash) && (eq(key, e->_key))) {
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
	HashMap(int initialCapacity = DEFAULT_INITIAL_CAPACITY, float loadFactor = HASH_DEFAULT_LOAD_FACTOR) {
		if (initialCapacity > MAXIMUM_CAPACITY)
			initialCapacity = MAXIMUM_CAPACITY;

		// Find a power of 2 >= initialCapacity
		int capacity = 1;
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

	HashMap(const HashMap& other) {
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

	/** Removes all mappings from this map. */
	virtual void clear() {
		for (int i = 0; i <_tableLength; i++) {
			Entry *e = _table[i];
			while (e != NULL) {
				Entry *next = e->_next;
				delete e;
				e = next;
			}
		}
		memset(_table, 0, _tableLength * sizeof(Entry*));
		_size = 0;
	}

	virtual ~HashMap() {
		if (_table != NULL) {
			clear();
			free(_table);
		}
		_table = NULL;
	}

	virtual Class const& getClass() const override {
		return hashMapClass;
	}

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of mappings in this map
	 */
	ssize_t size() const override {
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
	std::shared_ptr<V> get(const K& key) const {
		int hash = _smudge(std::hash<K>()(key));
		Pred eq;
		for (Entry *e = _table[indexFor(hash, _tableLength)]; e != nullptr; e = e->_next) {
			if ((e->_hash == hash) && (eq(e->_key, key)))
				return e->_value;
		}
		return std::shared_ptr<V>();
	}

	virtual const typename Map<K, V>::Entry *getEntry(const K& key) const {
		int hash = _smudge(std::hash<K>()(key));
		Pred eq;
		for (const Entry *e = _table[indexFor(hash, _tableLength)]; e != nullptr; e = e->_next) {
			if ((e->_hash == hash) && (eq(e->_key, key)))
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
		int hash = _smudge(std::hash<K>()(key));
		Pred eq;
		for (Entry *e = _table[indexFor(hash, _tableLength)]; e != nullptr; e = e->_next) {
			if ((e->_hash == hash) && (eq(e->_key, key)))
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
	std::shared_ptr<V> put(const K& key, const V& value) {
		return put(key, std::make_shared<V>(value));
	}

	std::shared_ptr<V> put(const K& key, std::shared_ptr<V> const& value) {
		int hash = _smudge(std::hash<K>()(key));
		int i = indexFor(hash, _tableLength);
		Pred eq;
		for (Entry *e = _table[i]; e != nullptr; e = e->_next) {
			if ((e->_hash == hash) && (eq(e->_key, key))) {
				std::shared_ptr<V> oldValue = e->_value;
				e->_value = value;
				return oldValue;
			}
		}
		addEntry(hash, key, value, i);
		return std::shared_ptr<V>();
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
		int hash = _smudge(std::hash<K>()(key));
		int i = indexFor(hash, _tableLength);
		Pred eq;
		for (Entry *e = _table[i]; e != nullptr; e = e->_next) {
			if ((e->_hash == hash) && (eq(e->_key, key))) {
				e->_value = value;
				return;
			}
		}
		addEntry(hash, key, value, i);
	}

	template<class K1, class V1>
	void emplace(K1&& k, V1&& v) {
		V value(std::forward<V1>(v));
		emplace(k, std::make_shared<V1>(std::move(value)));
	}

	template<class K1, class V1>
	void emplace(K1&& k, std::shared_ptr<V1> v) {
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
	}

	void put(std::initializer_list<std::pair<const K, V> > args) {
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
	std::shared_ptr<V> remove(const K& key) {
		return removeEntryForKey(key);
	}

	void erase(const K& key) {
		eraseEntryForKey(key);
	}

	/** 
	 * Copies all mappings from <i>other</i> to this map. Does <b>not</b> clear
	 * this map beforehand.
	 */
	virtual void copyFrom(const HashMap& other) {
		for (int i = 0; i < other._tableLength; i++) {
			Entry *e = other._table[i];
			while (e != NULL) {
				put(e->_key, e->_value);
				e = e->_next;
			}
		}
	}

	void forEach(bool (*callback)(void*, const K&, const V&), void *data) const {
		for (int i = 0; i < _tableLength; i++) {
			Entry *e = _table[i];
			while (e != NULL) {
				bool cont = callback(data, e->_key, e->_value);
				if (!cont)
					return;
				e = e->_next;
			}
		}
	}

	void forEach(std::function<bool(const K&, const V&)> callback) const {
		for (int i = 0; i < _tableLength; i++) {
			Entry *e = _table[i];
			while (e != NULL) {
				bool cont = callback(e->_key, e->_value);
				if (!cont)
					return;
				e = e->_next;
			}
		}
	}


private:
	// iterator
	class ConstEntryIterator : public ConstIterator<typename Map<K, V>::Entry>::ConstIteratorImpl {
	private:
		const HashMap *_map;
	protected:
		int _index;
		Entry *_current, *_next;
	protected:
		ConstEntryIterator(ConstEntryIterator* other) {
			_map = other->_map;
			_index = other->_index;
			_current = other->_current;
			_next = other->_next;
		}
	public:
		ConstEntryIterator(const HashMap *map) {
			_map = map;
			_next = nullptr;
			_index = 0;
			_current = nullptr;
			if (_map->_size > 0) {
				// go to first entry
				while (_index < _map->_tableLength && (_next = _map->_table[_index++]) == nullptr);
			}
		}

		virtual bool hasNext() {
			return (_next != nullptr);
		}

		virtual const typename Map<K, V>::Entry& next() {
			Entry *e = _next;
			if (e == nullptr)
				throw NoSuchElementException(_HERE_);
			if ((_next = e->_next) == nullptr) {
				while (_index < _map->_tableLength && (_next = _map->_table[_index++]) == nullptr);
			}
			_current = e;
			return *e;
		}

		virtual typename ConstIterator<typename Map<K, V>::Entry>::ConstIteratorImpl *clone() {
			return new ConstEntryIterator(this);
		}
	};

	class EntryIterator : public Iterator<typename Map<K, V>::Entry>::IteratorImpl, public ConstEntryIterator {
	private:
		HashMap *_ncMap;
	protected:
		EntryIterator(EntryIterator* other) 
		: ConstEntryIterator(other) {
			_ncMap = other->_ncMap;
		}
	public:
		EntryIterator(HashMap *map) 
		: ConstEntryIterator(map) {
			_ncMap = map;
		}

		virtual bool hasNext() {
			return ConstEntryIterator::hasNext();
		}

		virtual const typename Map<K, V>::Entry& next() {
			return ConstEntryIterator::next();
		}

		virtual void remove() {
			if (this->_current == NULL)
				throw IllegalStateException(_HERE_);
			K k = this->_current->_key;
			_ncMap->removeEntryForKey(k);
		}

		virtual typename Iterator<typename Map<K, V>::Entry>::IteratorImpl *clone() {
			return new EntryIterator(this);
		}
	};

public:
	virtual ConstIterator<typename Map<K, V>::Entry> constIterator() const {
		return ConstIterator<typename Map<K, V>::Entry>(new ConstEntryIterator(this));
	}

	virtual Iterator<typename Map<K, V>::Entry> iterator() {
		return Iterator<typename Map<K, V>::Entry>(new EntryIterator(this));
	}
};

} // namespace

#endif // H_SLIB_COLLECTIONS_HASHMAP_H
