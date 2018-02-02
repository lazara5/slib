#include "slib/util/JsonUtils.h"

using namespace rapidjson;

extern slib::Log logger;

namespace slib {

void JsonUtils::dump(const char *label, Document const& doc) {

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	doc.Accept(writer);
	logger.tracef("[{}]>{}<", label, buffer.GetString());
}

const Value *JsonUtils::getValue(const char *where, const Pointer& pointer, const Document& doc) {
	return pointer.Get(doc);
}

std::string JsonUtils::pathToString(const char *path) {
	return path;
}

std::string JsonUtils::pathToString(const Pointer& pointer) {
	std::string path;
	JsonStringAdapter sa(path);
	pointer.Stringify(sa);
	return path;
}

/**
 * @throws InvalidValueException
 * @throws JsonException
 */
const Value *JsonUtils::getValue(const char *where, const char *path, const Document& doc) {
	if (!doc.IsObject())
		throw InvalidValueException(where, path, "Object params expected");
	if (path[0] == '@') {
		if (doc.HasMember(path + 1))
			return &(doc[path + 1]);
		return nullptr;
	} else {
		Pointer pointer(path);
		if (!pointer.IsValid())
			throw JsonException(where, fmt::format("JSON pointer error {:d} (@'{}':{:d})",
												   pointer.GetParseErrorCode(), path, pointer.GetParseErrorOffset()).c_str());
		return getValue(where, pointer, doc);
	}
}

void JsonUtils::checkArrayIndex(const char *where, unsigned int index, const Document& doc) {
	if (!doc.IsArray())
		throw InvalidValueException(where, UInt::toString(index).c_str(), "Array params expected");
	if (index >= doc.Size())
		throw InvalidValueException(where, UInt::toString(index).c_str(), "Param not present in array");
}

/** @throws InvalidValueException */
std::string JsonUtils::getString(const char *where, unsigned int index, const Document& doc) {
	checkArrayIndex(where, index, doc);
	const Value& value = doc[index];
	if (value.IsString())
		return value.GetString();
	throw slib::InvalidValueException(where, UInt::toString(index).c_str(), "String expected");
}

/** @throws InvalidValueException */
bool JsonUtils::getBool(const char *where, unsigned int index, const Document& doc) {
	checkArrayIndex(where, index, doc);
	const Value& value = doc[index];
	if (value.IsBool())
		return value.GetBool();
	throw InvalidValueException(where, UInt::toString(index).c_str(), "Bool expected");
}

} // namespace slib
