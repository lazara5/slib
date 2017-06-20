/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __LINKEDHASHMAP_H__
#define __LINKEDHASHMAP_H__

#include "slib/HashMap.h"

namespace slib {

/**
 * Hash table and linked list implementation of the Map interface, with predictable iteration order.
 * This implementation differs from HashMap in that it maintains a doubly-linked list running through the entries.
 */
template <class K, class V>
class LinkedHashMap : public HashMap<K, V> {
private:
	class Entry : public HashMap<K, V>::Entry {
	friend class LinkedHashMap;
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
		Entry(int h, const K& k, std::shared_ptr<V> v, typename HashMap<K,V>::Entry *n)
		: HashMap<K, V>::Entry(h, k, v, n) {
			_before = _after = nullptr;
		}

		/** inplace constructor */
		Entry(typename HashMap<K,V>::Entry *n, int h, K& k, V& v)
		: HashMap<K, V>::Entry(n, h, k, v) {
			_before = _after = nullptr;
		}

		virtual void onRemove(HashMap<K, V> *) {
			remove();
		}
	};

	Entry *_header;
protected:
	virtual void createEntry(int hash, const K& key, std::shared_ptr<V> value, int bucketIndex) {
		typename HashMap<K, V>::Entry *old = this->_table[bucketIndex];
		Entry *e = new Entry(hash, key, value, old);
		this->_table[bucketIndex] = e;
		e->addBefore(_header);
		this->_size++;
	}

	virtual void createInplaceEntry(int hash, K& key, std::shared_ptr<V> value, int bucketIndex) {
		typename HashMap<K, V>::Entry *old = this->_table[bucketIndex];
		Entry *e = new Entry(hash, key, value, old);
		this->_table[bucketIndex] = e;
		e->addBefore(_header);
		this->_size++;
	}

	virtual void addEntry(int hash, const K& key, std::shared_ptr<V> value, int bucketIndex) override {
		createEntry(hash, key, value, bucketIndex);

		if (this->_size >= this->_threshold)
			this->resize(2 * this->_tableLength);
	}

	virtual void emplaceEntry(int hash, K& key, std::shared_ptr<V> value, int bucketIndex) override {
		createInplaceEntry(hash, key, value, bucketIndex);

		if (this->_size >= this->_threshold)
			this->resize(2 * this->_tableLength);
	}
public:
	static const int DEFAULT_INITIAL_CAPACITY = 16;
	static const int MAXIMUM_CAPACITY = 1 << 30;
public:
	LinkedHashMap(int initialCapacity = DEFAULT_INITIAL_CAPACITY, float loadFactor = HASH_DEFAULT_LOAD_FACTOR)
	:HashMap<K, V>(initialCapacity, loadFactor) {
		_header = new Entry(-1, K(), std::shared_ptr<V>(), nullptr);
		_header->_before = _header->_after = _header;
	}

	LinkedHashMap(const LinkedHashMap& other)
	:HashMap<K, V>(other._tableLength, other._loadFactor) {
		_header = new Entry(-1, K(), std::shared_ptr<V>(), nullptr);
		_header->_before = _header->_after = _header;

		copyFrom(other);
	}

	virtual void clear() {
		HashMap<K, V>::clear();
		_header->_before = _header->_after = _header;
	}

	~LinkedHashMap() {
		if (_header != nullptr)
			delete _header;
		_header = nullptr;
	}

	/**
	 * Copies all mappings from <i>other</i> to this map. Does <b>not</b> clear
	 * this map beforehand.
	 */
	virtual void copyFrom(const LinkedHashMap& other) {
		Entry *entry = other._header->_after;
		while (entry != other._header) {
			this->put(entry->_key, entry->_value);
			entry = entry->_after;
		}
	}

	void forEach(bool (*callback)(void*, const K&, const std::shared_ptr<V>&), void *data) const {
		Entry *entry = _header->_after;
		while (entry != _header) {
			bool cont = callback(data, entry->_key, entry->_value);
			if (!cont)
				return;
			entry = entry->_after;
		}
	}

	void forEach(std::function<bool(const K&, const std::shared_ptr<V>&)> callback) const {
		Entry *entry = _header->_after;
		while (entry != _header) {
			bool cont = callback(entry->_key, entry->_value);
			if (!cont)
				return;
			entry = entry->_after;
		}
	}

private:
	class ConstEntryIterator : public ConstIterator<typename Map<K, V>::Entry>::ConstIteratorImpl {
	private:
		const LinkedHashMap *_map;
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
		ConstEntryIterator(const LinkedHashMap *map) {
			_map = map;
			_nextEntry = _map->_header->_after;
			_lastReturned = nullptr;
		}

		virtual bool hasNext() {
			return _nextEntry != _map->_header;
		}

		virtual const typename Map<K, V>::Entry& next() {
			if (_nextEntry == _map->_header)
				throw NoSuchElementException(_HERE_);
			Entry *e = _lastReturned = _nextEntry;
			_nextEntry = e->_after;
			return *e;
		}

		virtual typename ConstIterator<typename Map<K, V>::Entry>::ConstIteratorImpl *clone() {
			return new ConstEntryIterator(this);
		}
	};

	class EntryIterator : public Iterator<typename Map<K, V>::Entry>::IteratorImpl, public ConstEntryIterator {
	private:
		LinkedHashMap *_ncMap;
	protected:
		EntryIterator(EntryIterator* other)
		: ConstEntryIterator(other) {
			_ncMap = other->_ncMap;
		}
	public:
		EntryIterator(LinkedHashMap *map)
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
			if (this->_lastReturned == nullptr)
				throw IllegalStateException(_HERE_);
			_ncMap->remove(this->_lastReturned->_key);
			this->_lastReturned = nullptr;
		}

		virtual typename Iterator<typename Map<K, V>::Entry>::IteratorImpl *clone() {
			return new EntryIterator(this);
		}
	};

public:
	ConstIterator<typename Map<K, V>::Entry> constIterator() const {
		return ConstIterator<typename Map<K, V>::Entry>(new ConstEntryIterator(this));
	}

	Iterator<typename Map<K, V>::Entry> iterator() {
		return Iterator<typename Map<K, V>::Entry>(new EntryIterator(this));
	}
};

} // namespace

#endif
