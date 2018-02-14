#include "slib/util/JsonUtils.h"

using namespace rapidjson;

extern slib::Log logger;

namespace slib {
namespace json {

void dump(const char *label, const Value &v) {
	if (logger.enabled(Log::Level::Trace)) {
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		v.Accept(writer);
		logger.tracef("[{}]>{}<", label, buffer.GetString());
	}
}

const Value *getValue(const char *where, const Pointer& pointer, const Value &v) {
	return pointer.Get(v);
}

std::string pathToString(const char *path) {
	return path;
}

std::string pathToString(const Pointer& pointer) {
	std::string path;
	JsonStringAdapter sa(path);
	pointer.Stringify(sa);
	return path;
}

/**
 * @throws InvalidValueException
 * @throws JsonException
 */
const Value *getValue(const char *where, const char *path, const Value &v) {
	if (!v.IsObject())
		throw InvalidValueException(where, path, "Object params expected");
	if (path[0] == '@') {
		if (v.HasMember(path + 1))
			return &(v[path + 1]);
		return nullptr;
	} else {
		Pointer pointer(path);
		if (!pointer.IsValid())
			throw JsonException(where, fmt::format("JSON pointer error {:d} (@'{}':{:d})",
												   pointer.GetParseErrorCode(), path, pointer.GetParseErrorOffset()).c_str());
		return getValue(where, pointer, v);
	}
}

void checkArrayIndex(const char *where, unsigned int index, const Value &v) {
	if (!v.IsArray())
		throw InvalidValueException(where, UInt::toString(index).c_str(), "Array params expected");
	if (index >= v.Size())
		throw InvalidValueException(where, UInt::toString(index).c_str(), "Param not present in array");
}

/** @throws InvalidValueException */
std::string getString(const char *where, unsigned int index, const Value &v) {
	checkArrayIndex(where, index, v);
	const Value& value = v[index];
	if (value.IsString())
		return value.GetString();
	throw InvalidValueException(where, UInt::toString(index).c_str(), "String expected");
}

/** @throws InvalidValueException */
const rapidjson::Value *getStringValue(const char *where, unsigned int index, const Value &v) {
	checkArrayIndex(where, index, v);
	const Value& value = v[index];
	if (value.IsString())
		return &value;
	throw InvalidValueException(where, UInt::toString(index).c_str(), "String expected");
}

/** @throws InvalidValueException */
bool getBool(const char *where, unsigned int index, const Value &v) {
	checkArrayIndex(where, index, v);
	const Value& value = v[index];
	if (value.IsBool())
		return value.GetBool();
	throw InvalidValueException(where, UInt::toString(index).c_str(), "Bool expected");
}

} // namespace json
} // namespace slib
