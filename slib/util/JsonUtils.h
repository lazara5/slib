/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_JSONUTILS_H
#define H_SLIB_UTIL_JSONUTILS_H

#include "slib/exception/Exception.h"
#include "slib/exception/ValueException.h"
#include "slib/StringBuilder.h"
#include "slib/String.h"
#include "slib/Numeric.h"
#include "slib/util/StringUtils.h"
#include "slib/util/Log.h"

#include "fmt/format.h"

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include "rapidjson/filereadstream.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/pointer.h>
#include <rapidjson/error/en.h>

#include <memory>

namespace slib {
namespace json {

class JsonException : public Exception {
public:
	JsonException(const char *where, const char *msg)
	:Exception(where, "JsonException", msg) {}
protected:
	JsonException(const char *where, const char *className, const char *msg)
	:Exception(where, className, msg) {}
};

class JsonParseException : public JsonException {
public:
	JsonParseException(const char *where, const char *msg)
	:JsonException(where, "JsonParseException", msg) {}
};

class JsonStringAdapter {
private:
	std::string& _s;
public:
	typedef char Ch;

	JsonStringAdapter(std::string& s)
	:_s(s) {}

	void Put(char c) {
		_s.push_back(c);
	}

	void Clear() {
		_s.clear();
	}

	void Flush() {}

	size_t Size() const {
		return _s.length();
	}
};


/** @throws JsonParseException */
template <unsigned parseFlags = rapidjson::kParseDefaultFlags>
void parse(const char *where, rapidjson::Document &doc, const slib::StringBuilder& json) {
	if (json.isEmpty())
		return;
	doc.Parse<parseFlags>(json.c_str());
	if (doc.HasParseError())
		throw JsonParseException(where, fmt::format("JSON parse failed, error={}", rapidjson::GetParseError_En(doc.GetParseError())).c_str());
}

/** @throws JsonParseException */
template <unsigned parseFlags = rapidjson::kParseDefaultFlags>
void parse(const char *where, rapidjson::Document &doc, const char *buffer, size_t length) {
	if (length <= 0)
		return;
	rapidjson::MemoryStream ms(buffer, length);\
	doc.ParseStream<parseFlags>(ms);
	if (doc.HasParseError())
		throw JsonParseException(where, fmt::format("JSON parse failed, error={}", rapidjson::GetParseError_En(doc.GetParseError())).c_str());
}

/** @throws JsonParseException */
template <unsigned parseFlags = rapidjson::kParseDefaultFlags>
void parseFile(const char *where, rapidjson::Document &doc, const std::string& fileName) {
	FILE* f = fopen(fileName.c_str(), "r");
	if (!f)
		throw JsonParseException(where, fmt::format("JSON parse failed, file error: {}", slib::StringUtils::formatErrno()).c_str());
	char buffer[16384];
	rapidjson::FileReadStream is(f, buffer, sizeof(buffer));
	doc.ParseStream<parseFlags>(is);
	fclose(f);
	if (doc.HasParseError())
		throw JsonParseException(where, fmt::format("JSON parse failed, error={}", rapidjson::GetParseError_En(doc.GetParseError())).c_str());
}

void dump(const char *label, rapidjson::Value const& v);

const rapidjson::Value *getValue(const char *where, const rapidjson::Pointer& pointer, const rapidjson::Value& v);

std::string pathToString(const char *path);

std::string pathToString(const rapidjson::Pointer& pointer);

/**
 * @throws InvalidValueException
 * @throws JsonException
 */
const rapidjson::Value *getValue(const char *where, const char *path, const rapidjson::Value& v);

/** @throws ValueException */
template <class P>
std::string getString(const char *where, P path, const rapidjson::Value& v) {
	const rapidjson::Value *value = getValue(where, path, v);
	if (value == nullptr)
		throw MissingValueException(where, pathToString(path).c_str());
	if (value->IsString())
		return String::trim(value->GetString());
	throw InvalidValueException(where, pathToString(path).c_str(), "String expected");
}

template <class P>
const rapidjson::Value *getStringValue(const char *where, P path, const rapidjson::Value& v) {
	const rapidjson::Value *value = getValue(where, path, v);
	if (value == nullptr)
		throw MissingValueException(where, pathToString(path).c_str());
	if (value->IsString())
		return value;
	throw InvalidValueException(where, pathToString(path).c_str(), "String expected");
}

template <class P>
std::unique_ptr<std::string> getStringIfExists(const char *where, P path, const rapidjson::Value& v) {
	const rapidjson::Value *value = getValue(where, path, v);
	if (value == nullptr)
		return std::unique_ptr<std::string>();
	if (value->IsString())
		return std::make_unique<std::string>(String::trim(value->GetString()));
	throw InvalidValueException(where, pathToString(path).c_str(), "String expected");
}

/** @throws InvalidValueException */
template <class P>
std::string getString(const char *where, P path, const rapidjson::Value& v, const std::string& defaultValue) {
	try {
		return getString(where, path, v);
	} catch (MissingValueException const&) {
		return defaultValue;
	}
}

void checkArrayIndex(const char *where, unsigned int index, const rapidjson::Value& v);

/** @throws InvalidValueException */
std::string getString(const char *where, unsigned int index, const rapidjson::Value& v);

/** @throws InvalidValueException */
const rapidjson::Value *getStringValue(const char *where, unsigned int index, const rapidjson::Value& v);

/** @throws ValueException */
template <class P>
int64_t getLong(const char *where, P path, const rapidjson::Value& v) {
	const rapidjson::Value *value = getValue(where, path, v);
	if (value == nullptr)
		throw MissingValueException(where, pathToString(path).c_str());
	if (value->IsInt64())
		return value->GetInt64();
	throw InvalidValueException(where, pathToString(path).c_str(), "Int64 expected");
}

/** @throws InvalidValueException */
template <class P>
int64_t getLong(const char *where, P path, const rapidjson::Value& v, int64_t defaultValue) {
	try {
		return getLong(where, path, v);
	} catch (slib::MissingValueException const&) {
		return defaultValue;
	}
}

/** @throws ValueException */
template <class P>
bool getBool(const char *where, P path, const rapidjson::Value& v) {
	const rapidjson::Value *value = getValue(where, path, v);
	if (value == nullptr)
		throw MissingValueException(where, pathToString(path).c_str());
	if (value->IsBool())
		return value->GetBool();
	throw InvalidValueException(where, pathToString(path).c_str(), "Bool expected");
}

/** @throws InvalidValueException */
template <class P>
bool getBool(const char *where, P path, const rapidjson::Value& v, bool defaultValue) {
	try {
		return getBool(where, path, v);
	} catch (MissingValueException const&) {
		return defaultValue;
	}
}

/** @throws InvalidValueException */
bool getBool(const char *where, unsigned int index, const rapidjson::Value& v);

} // namespace json
} // namespace slib

#endif // H_SLIB_UTIL_JSONUTILS_H
