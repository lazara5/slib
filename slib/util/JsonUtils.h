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

class JsonException : public slib::Exception {
public:
	JsonException(const char *where, const char *msg)
	:Exception(where, "JsonException", msg) {}
};

class JsonParseException : public slib::Exception {
public:
	JsonParseException(const char *where, const char *msg)
	:Exception(where, "JsonParseException", msg) {}
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

class JsonUtils {
public:
	/** @throws JsonParseException */
	template <unsigned parseFlags = rapidjson::kParseDefaultFlags>
	static void parse(const char *where, rapidjson::Document &doc, const slib::StringBuilder& json) {
		if (json.isEmpty())
			return;
		doc.Parse<parseFlags>(json.c_str());
		if (doc.HasParseError())
			throw JsonParseException(where, fmt::format("JSON parse failed, error={}", rapidjson::GetParseError_En(doc.GetParseError())).c_str());
	}

	/** @throws JsonParseException */
	template <unsigned parseFlags = rapidjson::kParseDefaultFlags>
	static void parse(const char *where, rapidjson::Document &doc, const char *buffer, size_t length) {
		if (length <= 0)
			return;
		rapidjson::MemoryStream ms(buffer, length);\
		doc.ParseStream<parseFlags>(ms);
		if (doc.HasParseError())
			throw JsonParseException(where, fmt::format("JSON parse failed, error={}", rapidjson::GetParseError_En(doc.GetParseError())).c_str());
	}

	/** @throws JsonParseException */
	template <unsigned parseFlags = rapidjson::kParseDefaultFlags>
	static void parseFile(const char *where, rapidjson::Document &doc, const std::string& fileName) {
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

	static void dump(const char *label, rapidjson::Document const& doc) {
		extern slib::Log logger;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		logger.tracef("[{}]>{}<", label, buffer.GetString());
	}

	static const rapidjson::Value *getValue(const char *where, const rapidjson::Pointer& pointer, const rapidjson::Document& doc) {
		return pointer.Get(doc);
	}

	static std::string pathToString(const char *path) {
		return path;
	}

	static std::string pathToString(const rapidjson::Pointer& pointer) {
		std::string path;
		JsonStringAdapter sa(path);
		pointer.Stringify(sa);
		return path;
	}

	/** @throws JSONException */
	static const rapidjson::Value *getValue(const char *where, const char *path, const rapidjson::Document& doc) {
		if (!doc.IsObject())
			throw slib::InvalidValueException(where, path, "Object params expected");
		if (path[0] == '@') {
			if (doc.HasMember(path + 1))
				return &(doc[path + 1]);
			return nullptr;
		} else {
			rapidjson::Pointer pointer(path);
			if (!pointer.IsValid())
				throw JsonException(where, fmt::format("JSON pointer error {:d} (@'{}':{:d})",
													   pointer.GetParseErrorCode(), path, pointer.GetParseErrorOffset()).c_str());
			return getValue(where, pointer, doc);
		}
	}

	/** @throws ValueException */
	template <class P>
	static std::string getString(const char *where, P path, const rapidjson::Document& doc) {
		const rapidjson::Value *value = getValue(where, path, doc);
		if (value == nullptr)
			throw slib::MissingValueException(where, pathToString(path).c_str());
		if (value->IsString())
			return slib::String::trim(value->GetString());
		throw slib::InvalidValueException(where, pathToString(path).c_str(), "String expected");
	}

	template <class P>
	static std::unique_ptr<std::string> getStringIfExists(const char *where, P path, const rapidjson::Document& doc) {
		const rapidjson::Value *value = getValue(where, path, doc);
		if (value == nullptr)
			return std::unique_ptr<std::string>();
		if (value->IsString())
			return std::make_unique<std::string>(slib::String::trim(value->GetString()));
		throw slib::InvalidValueException(where, pathToString(path).c_str(), "String expected");
	}

	/** @throws InvalidValueException */
	template <class P>
	static std::string getString(const char *where, P path, const rapidjson::Document& doc, const std::string& defaultValue) {
		try {
			return getString(where, path, doc);
		} catch (slib::MissingValueException const&) {
			return defaultValue;
		}
	}

	static void checkArrayIndex(const char *where, unsigned int index, const rapidjson::Document& doc) {
		if (!doc.IsArray())
			throw slib::InvalidValueException(where, slib::UInt::toString(index).c_str(), "Array params expected");
		if (index >= doc.Size())
			throw slib::InvalidValueException(where, slib::UInt::toString(index).c_str(), "Param not present in array");
	}

	/** @throws InvalidValueException */
	static std::string getString(const char *where, unsigned int index, const rapidjson::Document& doc) {
		checkArrayIndex(where, index, doc);
		const rapidjson::Value& value = doc[index];
		if (value.IsString())
			return value.GetString();
		throw slib::InvalidValueException(where, slib::UInt::toString(index).c_str(), "String expected");
	}

	/** @throws ValueException */
	template <class P>
	static int64_t getLong(const char *where, P path, const rapidjson::Document& doc) {
		const rapidjson::Value *value = getValue(where, path, doc);
		if (value == nullptr)
			throw slib::MissingValueException(where, pathToString(path).c_str());
		if (value->IsInt64())
			return value->GetInt64();
		throw slib::InvalidValueException(where, pathToString(path).c_str(), "Int64 expected");
	}

	/** @throws InvalidValueException */
	template <class P>
	static int64_t getLong(const char *where, P path, const rapidjson::Document& doc, int64_t defaultValue) {
		try {
			return getLong(where, path, doc);
		} catch (slib::MissingValueException const&) {
			return defaultValue;
		}
	}

	/** @throws ValueException */
	template <class P>
	static bool getBool(const char *where, P path, const rapidjson::Document& doc) {
		const rapidjson::Value *value = getValue(where, path, doc);
		if (value == nullptr)
			throw slib::MissingValueException(where, pathToString(path).c_str());
		if (value->IsBool())
			return value->GetBool();
		throw slib::InvalidValueException(where, pathToString(path).c_str(), "Bool expected");
	}

	/** @throws InvalidValueException */
	template <class P>
	static bool getBool(const char *where, P path, const rapidjson::Document& doc, bool defaultValue) {
		try {
			return getBool(where, path, doc);
		} catch (slib::MissingValueException const&) {
			return defaultValue;
		}
	}

	/** @throws InvalidValueException */
	static bool getBool(const char *where, unsigned int index, const rapidjson::Document& doc) {
		checkArrayIndex(where, index, doc);
		const rapidjson::Value& value = doc[index];
		if (value.IsBool())
			return value.GetBool();
		throw slib::InvalidValueException(where, slib::UInt::toString(index).c_str(), "Bool expected");
	}

};

#endif // H_SLIB_UTIL_JSONUTILS_H
