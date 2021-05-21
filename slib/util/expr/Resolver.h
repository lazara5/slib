/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_EXPR_RESOLVER_H
#define H_SLIB_UTIL_EXPR_RESOLVER_H

#include "slib/lang/Object.h"
#include "slib/lang/String.h"
#include "slib/collections/ArrayList.h"
#include "slib/collections/HashMap.h"
#include "slib/collections/Map.h"
#include "slib/util/expr/Exceptions.h"

namespace slib {

namespace expr {

class Resolver : virtual public Object {
public:
	TYPE_INFO(Resolver, CLASS(Resolver), INHERITS(Object));
public:
	enum struct Mode : uint8_t {
		READ_ONLY,
		WRITABLE
	};
public:
	virtual ~Resolver() override;

	/**
	 * Resolves a variable
	 *
	 * @param key  variable name
	 * @return variable value or nullptr if not defined
	 */
	virtual SPtr<Object> getVar(String const& key) const = 0;

	virtual bool isReadOnly() const {
		return true;
	}

	virtual void setVar(String const& key SLIB_UNUSED, SPtr<Object> const& value SLIB_UNUSED) {
		throw EvaluationException(_HERE_, "Attempted to write to read-only resolver");
	}
};

class MapResolver : public Resolver {
public:
	TYPE_INFO(MapResolver, CLASS(MapResolver), INHERITS(Resolver));
private:
	SPtr<Map<String, Object>> _map;
	Mode _mode;
public:
	MapResolver(SPtr<Map<String, Object>> map, Mode mode = Mode::READ_ONLY)
	: _map(map)
	, _mode(mode) {}

	virtual ~MapResolver() override;

	virtual SPtr<Object> getVar(const String &key) const override {
		return _map->get(key);
	}

	virtual bool isReadOnly() const override {
		return _mode == Mode::READ_ONLY;
	}

	virtual void setVar(String const& key, SPtr<Object> const& value) override {
		if (isReadOnly())
			return Resolver::setVar(key, value);
		_map->emplaceKey<String>(key, value);
	}
};

class ChainedResolver : public Resolver {
public:
	TYPE_INFO(ChainedResolver, CLASS(ChainedResolver), INHERITS(Resolver));
private:
	UPtr<List<Resolver>> _resolvers;
	UPtr<Map<String, Resolver>> _namedResolvers;
	SPtr<Resolver> _writableResolver;
public:
	/** do NOT use directly, only public for make_shared */
	ChainedResolver(bool /* dontUse */)
	: _resolvers(newU<ArrayList<Resolver>>())
	, _namedResolvers(newU<HashMap<String, Resolver>>()) {}
public:
	virtual ~ChainedResolver() override;

	static SPtr<ChainedResolver> newInstance() {
		return newS<ChainedResolver>(true);
	}

	ChainedResolver& add(SPtr<Resolver> const& resolver) {
		_resolvers->add(resolver);
		if ((!_writableResolver) && (!resolver->isReadOnly()))
			_writableResolver = resolver;
		return *this;
	}

	ChainedResolver& add(SPtr<String> const& name, SPtr<Resolver> const& resolver) {
		_namedResolvers->put(name, resolver);
		return *this;
	}

	ChainedResolver& add(SPtr<Map<String, Object>> map, Mode mode = Mode::READ_ONLY) {
		SPtr<Resolver> resolver = newS<MapResolver>(map, mode);
		_resolvers->add(resolver);
		if ((!_writableResolver) && (!resolver->isReadOnly()))
			_writableResolver = resolver;
		return *this;
	}

	ChainedResolver& add(SPtr<String> const& name, SPtr<Map<String, Object>> map) {
		_namedResolvers->put(name, newS<MapResolver>(map));
		return *this;
	}

	ChainedResolver& clear() {
		_resolvers->clear();
		_namedResolvers->clear();
		_writableResolver = nullptr;
		return *this;
	}

	virtual SPtr<Object> getVar(const String &key) const override;

	virtual bool isReadOnly() const override {
		return !((bool)_writableResolver);
	}

	virtual void setVar(String const& key, SPtr<Object> const& value) override;
};

class LazyResolver : public Resolver {
private:
	bool _initialized;
public:
	LazyResolver()
	: _initialized(false) {}

	virtual ~LazyResolver() {}

	virtual void initialize() = 0;

	void init() {
		if (!_initialized) {
			initialize();
			_initialized = true;
		}
	}

	virtual SPtr<Object> provideVar(String const& name) const = 0;

	virtual SPtr<Object> getVar(String const& name) const override final {
		if (!_initialized)
			const_cast<LazyResolver *>(this)->init();
		return provideVar(name);
	}
};


} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_RESOLVER_H
