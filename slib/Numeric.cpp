#include "slib/Numeric.h"

namespace slib {

Number::~Number() {}

double Integer::doubleValue() const {
	return (double)_value;
}

double UInt::doubleValue() const {
	return (double)_value;
}

double Long::doubleValue() const {
	return (double)_value;
}

double ULong::doubleValue() const {
	return (double)_value;
}

double Double::doubleValue() const {
	return _value;
}

} // namespace slib
