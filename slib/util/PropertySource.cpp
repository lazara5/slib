#include "slib/util/PropertySource.h"

namespace slib {

PropertySource::~PropertySource() {};

std::string PropertySource::getProperty(const std::string& name) {
	if (!_initialized)
		init();
	PropMapConstIter i = _properties.find(name);
	if (i == _properties.end())
		throw MissingValueException(_HERE_, name.c_str());
	GetProperty getProp = i->second;
	std::string ret = *((this->*getProp)());
	return ret;
}

std::shared_ptr<std::string> PropertySource::get(std::string const& name) const {
	if (!_initialized)
		const_cast<PropertySource *>(this)->init();
	PropMapConstIter i = _properties.find(name);
	if (i == _properties.end())
		throw MissingValueException(_HERE_, name.c_str());
	GetProperty getProp = i->second;
	return (this->*getProp)();
}

bool PropertySource::containsKey(std::string const& name) const {
	PropMapConstIter i = _properties.find(name);
	return (i != _properties.end());
}

} // namespace slib
