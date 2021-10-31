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

enum class ValueDomain : uint8_t {
	GLOBAL,
	DEFAULT,
	LOCAL,
	MAX,
	NONE = MAX
};

const std::array<ValueDomain, static_cast<uint8_t>(ValueDomain::MAX)> allValueDomains = {
	ValueDomain::GLOBAL, ValueDomain::DEFAULT, ValueDomain::LOCAL
};

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
	virtual SPtr<Object> getVar(String const& key, ValueDomain domain) const = 0;

	virtual bool isWritable(ValueDomain domain SLIB_UNUSED) const {
		return false;
	}

	virtual void setVar(SPtr<String> const& key SLIB_UNUSED, SPtr<Object> const& value SLIB_UNUSED, ValueDomain domain SLIB_UNUSED) {
		throw EvaluationException(_HERE_, "Attempted to write to read-only resolver");
	}
};

class MapResolver : public Resolver {
public:
	TYPE_INFO(MapResolver, CLASS(MapResolver), INHERITS(Resolver));
private:
	SPtr<Map<String, Object>> _map;
	ValueDomain _writableDomain;
	bool _domains[static_cast<uint8_t>(ValueDomain::MAX)];
	bool _writable;
public:
	MapResolver(SPtr<Map<String, Object>> map, ValueDomain domain = ValueDomain::DEFAULT, Mode mode = Mode::READ_ONLY)
	: _map(map)
	, _writable(mode == Mode::WRITABLE) {
		memset(_domains, 0, sizeof(_domains));
		switch (domain) {
			case ValueDomain::GLOBAL:
			case ValueDomain::DEFAULT:
				_domains[static_cast<uint8_t>(ValueDomain::GLOBAL)] = true;
				_domains[static_cast<uint8_t>(ValueDomain::DEFAULT)] = true;
				break;
			case ValueDomain::LOCAL:
				_domains[static_cast<uint8_t>(ValueDomain::LOCAL)] = true;
				break;
			default:
				throw EvaluationException(_HERE_, "Invalid value domain");
		}
	}

	virtual ~MapResolver() override;

	virtual SPtr<Object> getVar(const String &key, ValueDomain domain) const override {
		uint8_t iDomain = static_cast<uint8_t>(domain);
		if (!_domains[iDomain])
			return nullptr;
		return _map->get(key);
	}

	virtual bool isWritable(ValueDomain domain) const override {
		uint8_t iDomain = static_cast<uint8_t>(domain);
		return (_domains[iDomain] && _writable);
	}

	virtual void setVar(SPtr<String> const& key, SPtr<Object> const& value, ValueDomain domain) override {
		if (!isWritable(domain))
			return Resolver::setVar(key, value, domain);
		_map->put(key, value);
	}
};

class ChainedResolver : public Resolver {
public:
	TYPE_INFO(ChainedResolver, CLASS(ChainedResolver), INHERITS(Resolver));
private:
	UPtr<List<Resolver>> _resolvers;
	UPtr<Map<String, Resolver>> _namedResolvers;
	SPtr<Resolver> _writableResolver[static_cast<uint8_t>(ValueDomain::MAX)];
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
		for (ValueDomain domain : allValueDomains) {
			uint8_t iDomain = static_cast<uint8_t>(domain);
			if ((!_writableResolver[iDomain]) && (resolver->isWritable(domain)))
				_writableResolver[iDomain] = resolver;
		}
		return *this;
	}

	ChainedResolver& add(SPtr<String> const& name, SPtr<Resolver> const& resolver) {
		_namedResolvers->put(name, resolver);
		return *this;
	}

	ChainedResolver& add(SPtr<Map<String, Object>> map, ValueDomain domain = ValueDomain::DEFAULT,
						 Resolver::Mode mode = Resolver::Mode::WRITABLE) {
		SPtr<Resolver> resolver = newS<MapResolver>(map, domain, mode);
		_resolvers->add(resolver);
		for (ValueDomain domain : allValueDomains) {
			uint8_t iDomain = static_cast<uint8_t>(domain);
			if ((!_writableResolver[iDomain]) && (resolver->isWritable(domain)))
				_writableResolver[iDomain] = resolver;
		}
		return *this;
	}

	ChainedResolver& add(SPtr<String> const& name, SPtr<Map<String, Object>> map) {
		_namedResolvers->put(name, newS<MapResolver>(map));
		return *this;
	}

	ChainedResolver& clear() {
		_resolvers->clear();
		_namedResolvers->clear();
		for (uint8_t i = 0; i < static_cast<uint8_t>(ValueDomain::MAX); i++)
			_writableResolver[i] = nullptr;
		return *this;
	}

	virtual SPtr<Object> getVar(const String &key, ValueDomain domain) const override;

	virtual bool isWritable(ValueDomain domain) const override {
		uint8_t iDomain = static_cast<uint8_t>(domain);
		return (bool)_writableResolver[iDomain];
	}

	virtual void setVar(SPtr<String> const& key, SPtr<Object> const& value, ValueDomain domain) override;
};

class LazyResolver : public Resolver {
protected:
	typedef SPtr<Object> (LazyResolver::*GetVar)() const;
private:
	bool _initialized;
	HashMap<String, GetVar> _vars;
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

	virtual SPtr<Object> getVar(String const& name, ValueDomain domain SLIB_UNUSED) const override final {
		if (!_initialized)
			const_cast<LazyResolver *>(this)->init();
		GetVar *getter = _vars.getPtr(name);
		if (!getter)
			return nullptr;
		return (this->**getter)();
	}

protected:
	void provideVar(String const& name, GetVar var) {
		_vars.put(newS<String>(name), newS<GetVar>(var));
	}
};


} // namespace expr
} // namespace slib

#endif // H_SLIB_UTIL_EXPR_RESOLVER_H
