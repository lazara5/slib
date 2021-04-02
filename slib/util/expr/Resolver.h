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
	bool _readOnly;
public:
	MapResolver(SPtr<Map<String, Object>> map, bool readOnly = true)
	: _map(map)
	, _readOnly(readOnly) {}

	virtual ~MapResolver() override;

	virtual SPtr<Object> getVar(const String &key) const override {
		return _map->get(key);
	}

	virtual bool isReadOnly() const override {
		return _readOnly;
	}

	virtual void setVar(String const& key, SPtr<Object> const& value) override {
		if (_readOnly)
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
	ChainedResolver(bool /* dontUse */, SPtr<Resolver> const& resolver)
	: _resolvers(newU<ArrayList<Resolver>>())
	, _namedResolvers(newU<HashMap<String, Resolver>>()){
		_resolvers->add(resolver);
		if (!resolver->isReadOnly())
			_writableResolver = resolver;
	}

	/** do NOT use directly, only public for make_shared */
	ChainedResolver(bool /* dontUse */, SPtr<String> const& name, SPtr<Resolver> const& resolver)
	: _resolvers(newU<ArrayList<Resolver>>())
	, _namedResolvers(newU<HashMap<String, Resolver>>()){
		_namedResolvers->put(name, resolver);
	}
public:
	virtual ~ChainedResolver() override;

	static SPtr<ChainedResolver> over(SPtr<Resolver> const& resolver) {
		return newS<ChainedResolver>(true, resolver);
	}

	static SPtr<ChainedResolver> over(SPtr<String> const& name, SPtr<Resolver> const& resolver) {
		return newS<ChainedResolver>(true, name, resolver);
	}

	ChainedResolver const& with(SPtr<Resolver> const& resolver) {
		_resolvers->add(resolver);
		if ((!_writableResolver) && (!resolver->isReadOnly()))
			_writableResolver = resolver;
		return *this;
	}

	ChainedResolver const& with(SPtr<String> const& name, SPtr<Resolver> const& resolver) {
		_namedResolvers->put(name, resolver);
		return *this;
	}

	ChainedResolver const& with(SPtr<Map<String, Object>> map, bool readOnly = true) {
		SPtr<Resolver> resolver = newS<MapResolver>(map, readOnly);
		_resolvers->add(resolver);
		if ((!_writableResolver) && (!resolver->isReadOnly()))
			_writableResolver = resolver;
		return *this;
	}

	ChainedResolver const& with(SPtr<String> const& name, SPtr<Map<String, Object>> map) {
		_namedResolvers->emplaceValue<MapResolver>(name, map);
		return *this;
	}

	virtual SPtr<Object> getVar(const String &key) const override;

	virtual bool isReadOnly() const override {
		return !((bool)_writableResolver);
	}

	virtual void setVar(String const& key, SPtr<Object> const& value) override;
};

} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_RESOLVER_H
