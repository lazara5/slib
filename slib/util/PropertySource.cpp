#include "slib/util/PropertySource.h"

namespace slib {

PropertySource::~PropertySource() {};

SPtr<Object> PropertySource::getVar(String const& name) const {
	if (!_initialized)
		const_cast<PropertySource *>(this)->init();
	PropMapConstIter i = _properties.find(name);
	if (i == _properties.end())
		throw MissingValueException(_HERE_, name.c_str());
	GetProperty getProp = i->second;
	return (this->*getProp)();
}

} // namespace slib
