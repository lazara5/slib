/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_COLLECTIONS_LINKEDHASHMAP_H
#define H_SLIB_COLLECTIONS_LINKEDHASHMAP_H

#include "slib/collections/HashMap.h"

namespace slib {

template <class K, class V, class Pred = std::equal_to<K>>
class InternalLinkedHashMap : public InternalHashMap<K, V, Pred> {
template <class K1, class V1, class Pred1> friend class LinkedHashMap;
public:
	static const int32_t DEFAULT_INITIAL_CAPACITY = 16;
	static const int32_t MAXIMUM_CAPACITY = 1 << 30;
private:
	class Entry : public InternalHashMap<K, V, Pred>::Entry {
	template <class K1, class V1, class Pred1> friend class InternalLinkedHashMap;
	protected:
		Entry *_before, *_after;
	private:
		void remove() {
			_before->_after = _after;
			_after->_before = _before;
		}

		/** Inserts this entry before the specified existing entry in the list */
		void addBefore(Entry *existingEntry) {
			_after = existingEntry;
			_before = existingEntry->_before;
			_before->_after = this;
			_after->_before = this;
		}
	public:
		Entry(int h, const K& k, SPtr<V> v, typename InternalHashMap<K, V, Pred>::Entry *n)
		:InternalHashMap<K, V, Pred>::Entry(h, k, v, n) {
			_before = _after = nullptr;
		}

		/** inplace constructor */
		Entry(typename HashMap<K, V, Pred>::Entry *n, int h, K& k, V& v)
		:InternalHashMap<K, V, Pred>::Entry(n, h, k, v) {
			_before = _after = nullptr;
		}

		virtual void onRemove(InternalHashMap<K, V, Pred> *) override {
			remove();
		}
	};

	Entry *_header;
protected:
	virtual void createEntry(int hash, const K& key, SPtr<V> value, int bucketIndex) {
		typename InternalHashMap<K, V, Pred>::Entry *old = this->_table[bucketIndex];
		Entry *e = new Entry(hash, key, value, old);
		this->_table[bucketIndex] = e;
		e->addBefore(_header);
		this->_size++;
	}

	virtual void createInplaceEntry(int hash, K& key, SPtr<V> value, int bucketIndex) {
		typename InternalHashMap<K, V, Pred>::Entry *old = this->_table[bucketIndex];
		Entry *e = new Entry(hash, key, value, old);
		this->_table[bucketIndex] = e;
		e->addBefore(_header);
		this->_size++;
	}

	virtual void addEntry(int hash, const K& key, SPtr<V> value, int bucketIndex) override {
		createEntry(hash, key, value, bucketIndex);

		if (this->_size >= this->_threshold)
			this->resize(2 * this->_tableLength);
	}

	virtual void emplaceEntry(int hash, K& key, SPtr<V> value, int bucketIndex) override {
		createInplaceEntry(hash, key, value, bucketIndex);

		if (this->_size >= this->_threshold)
			this->resize(2 * this->_tableLength);
	}
public:
	InternalLinkedHashMap(int32_t initialCapacity = DEFAULT_INITIAL_CAPACITY, float loadFactor = HASH_DEFAULT_LOAD_FACTOR)
	:InternalHashMap<K, V, Pred>(initialCapacity, loadFactor) {
		_header = new Entry(-1, K(), nullptr, nullptr);
		_header->_before = _header->_after = _header;
	}

	InternalLinkedHashMap(InternalLinkedHashMap const& other)
	:InternalHashMap<K, V, Pred>(other._tableLength, other._loadFactor) {
		_header = new Entry(-1, K(), nullptr, nullptr);
		_header->_before = _header->_after = _header;

		copyFrom(other);
	}

	~InternalLinkedHashMap() {
		if (_header != nullptr)
			delete _header;
		_header = nullptr;
	}

	virtual void clear() override {
		InternalHashMap<K, V, Pred>::clear();
		_header->_before = _header->_after = _header;
	}

	/**
	 * Copies all mappings from <i>other</i> to this map. Does <b>not</b> clear
	 * this map beforehand.
	 */
	virtual void copyFrom(InternalLinkedHashMap const& other) {
		Entry *entry = other._header->_after;
		while (entry != other._header) {
			this->put(entry->_key, entry->_value);
			entry = entry->_after;
		}
	}

	virtual void forEach(bool (*callback)(void*, const K&, const SPtr<V>&), void *data) const override {
		Entry *entry = _header->_after;
		while (entry != _header) {
			bool cont = callback(data, entry->_key, entry->_value);
			if (!cont)
				return;
			entry = entry->_after;
		}
	}

	virtual void forEach(std::function<bool(const K&, const SPtr<V>&)> callback) const override {
		Entry *entry = _header->_after;
		while (entry != _header) {
			bool cont = callback(entry->_key, entry->_value);
			if (!cont)
				return;
			entry = entry->_after;
		}
	}
protected:
	class ConstEntryIterator : public ConstIterator<typename Map<K, V, Pred>::Entry>::ConstIteratorImpl {
	private:
		SPtr<InternalLinkedHashMap<K, V, Pred>> _map;
	protected:
		Entry *_nextEntry;
		Entry *_lastReturned;
	protected:
		ConstEntryIterator(ConstEntryIterator* other) {
			_map = other->_map;
			_nextEntry = other->_nextEntry;
			_lastReturned = other->_lastReturned;
		}
	public:
		ConstEntryIterator(SPtr<InternalLinkedHashMap> const& map) {
			_map = map;
			_nextEntry = _map->_header->_after;
			_lastReturned = nullptr;
		}

		virtual bool hasNext() {
			return _nextEntry != _map->_header;
		}

		virtual const typename Map<K, V, Pred>::Entry& next() {
			if (_nextEntry == _map->_header)
				throw NoSuchElementException(_HERE_);
			Entry *e = _lastReturned = _nextEntry;
			_nextEntry = e->_after;
			return *e;
		}

		virtual typename ConstIterator<typename Map<K, V, Pred>::Entry>::ConstIteratorImpl *clone() {
			return new ConstEntryIterator(this);
		}
	};

	class EntryIterator : public Iterator<typename Map<K, V, Pred>::Entry>::IteratorImpl, public ConstEntryIterator {
	private:
		SPtr<InternalLinkedHashMap<K, V, Pred>> _ncMap;
	protected:
		EntryIterator(EntryIterator* other)
		: ConstEntryIterator(other) {
			_ncMap = other->_ncMap;
		}
	public:
		EntryIterator(SPtr<InternalLinkedHashMap> const& map)
		: ConstEntryIterator(map) {
			_ncMap = map;
		}

		virtual bool hasNext() {
			return ConstEntryIterator::hasNext();
		}

		virtual const typename Map<K, V, Pred>::Entry& next() {
			return ConstEntryIterator::next();
		}

		virtual void remove() {
			if (this->_lastReturned == nullptr)
				throw IllegalStateException(_HERE_);
			_ncMap->remove(this->_lastReturned->_key);
			this->_lastReturned = nullptr;
		}

		virtual typename Iterator<typename Map<K, V, Pred>::Entry>::IteratorImpl *clone() {
			return new EntryIterator(this);
		}
	};
};

/**
 * Hash table and linked list implementation of the Map interface, with predictable iteration order.
 * This implementation differs from HashMap in that it maintains a doubly-linked list running through the entries.
 */
template <class K, class V, class Pred = std::equal_to<K>>
class LinkedHashMap : public HashMap<K, V, Pred> {
public:
	static const int32_t DEFAULT_INITIAL_CAPACITY = InternalLinkedHashMap<K, V, Pred>::DEFAULT_INITIAL_CAPACITY;
	static const int32_t MAXIMUM_CAPACITY = InternalLinkedHashMap<K, V, Pred>::MAXIMUM_CAPACITY;
protected:
	SPtr<InternalLinkedHashMap<K, V, Pred>> _internalMap;
public:
	LinkedHashMap(int32_t initialCapacity = DEFAULT_INITIAL_CAPACITY, float loadFactor = HASH_DEFAULT_LOAD_FACTOR)
	:_internalMap(std::make_shared<InternalLinkedHashMap<K, V, Pred>>(initialCapacity, loadFactor)) {}

	LinkedHashMap(const LinkedHashMap& other)
	:_internalMap(std::make_shared<InternalLinkedHashMap<K, V, Pred>>(*other._internalMap)) {}

	virtual Class const* getClass() const override {
		return LINKEDHASHMAPCLASS();
	}

	virtual void clear() override {
		_internalMap->clear();
	}

	/**
	 * Copies all mappings from <i>other</i> to this map. Does <b>not</b> clear
	 * this map beforehand.
	 */
	virtual void copyFrom(const LinkedHashMap& other) {
		_internalMap->copyFrom(*other._internalMap);
	}

	void forEach(bool (*callback)(void*, const K&, const SPtr<V>&), void *data) const {
		_internalMap->forEach(callback, data);
	}

	void forEach(std::function<bool(const K&, const SPtr<V>&)> callback) const {
		_internalMap->forEach(callback);
	}

public:
	ConstIterator<typename Map<K, V, Pred>::Entry> constIterator() const {
		return ConstIterator<typename Map<K, V, Pred>::Entry>(new typename InternalLinkedHashMap<K, V, Pred>::ConstEntryIterator(_internalMap));
	}

	Iterator<typename Map<K, V, Pred>::Entry> iterator() {
		return Iterator<typename Map<K, V, Pred>::Entry>(new typename InternalLinkedHashMap<K, V, Pred>::EntryIterator(_internalMap));
	}
};

} // namespace

#endif // H_SLIB_COLLECTIONS_LINKEDHASHMAP_H
