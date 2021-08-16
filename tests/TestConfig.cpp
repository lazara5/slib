#include "CppUTest/TestHarness.h"

#include "slib/util/Config.h"
#include "slib/util/SystemInfo.h"
#include "slib/util/FilenameUtils.h"
#include "slib/lang/Field.h"
#include "slib/lang/Reflection.h"

#include "Global.h"
#include "ConfigObject.h"

using namespace slib;

/*class TestConfig : public Config {
private:
	SPtr<slib::SystemInfo> _systemInfo;
public:
	TestConfig(String const& confFileName, String const& appName)
	:Config(confFileName, appName)
	,_systemInfo(newS<slib::SystemInfo>()) {
		registerPropertySource("system", _systemInfo);
		registerPropertySink("sink", [](String const& name, SPtr<Object> const& value) {
			STRCMP_EQUAL("name", name.c_str());
			CHECK(instanceof<String>(value));
		});
	}

	virtual void onBeforeSearch(slib::List<String> &pathList) const override;
};

void TestConfig::onBeforeSearch(slib::List<String> &pathList) const {
	pathList.emplace<String>(0, "data");
}*/

//static UPtr<TestConfig> config;
static SPtr<Config> config;

TEST_GROUP(ConfigTests) {
	void setup() {
		SPtr<String> path = FilenameUtils::concat(FilenameUtils::getPath(test_argv[0]), "data/test.conf");
		printf("path: %s\n", path->c_str());
		ConfigLoader configLoader(path, "SlibTest"_SPTR);
		configLoader
		.clearPaths()
		.search("data"_SPTR)
		.withResolver("system"_SPTR, newS<SystemInfo>());
		config = configLoader.load();
		//config = newU<TestConfig>(*FileUtils::buildPath(FileUtils::getPath(test_argv[0]), "data/test.conf"), "SlibTest");
		//config = newU<TestConfig>("test.conf", "SlibTest");
	}

	void teardown() {
		config = nullptr;
	}
};

class X {
public:
	int64_t i;
	SPtr<String> str;
	SPtr<Object> obj;
	UPtr<String> ustr;
public:
	/*static void _registerReflection(ReflectionInfo &ri) {
		ri.registerField("i"_SV, &X::i);
	}

	static ClassReflectionInfo *_getReflectionInfo() {
		static ReflectionInfo ri(&_registerReflection);
		return &ri;
	}*/

	/*template<typename T>
	static void _registerReflection(ReflectionInfo &ri) {
		ri.registerField("i"_SV, &T::i);
	}

	static ClassReflectionInfo *_getReflectionInfo() {
		static ReflectionInfo ri(&_registerReflection<X>);
		return &ri;
	}*/

	/*REFLECT(CLASS(X),
		FIELD(i),
		FIELD(str)
	);*/

	REFLECT(X) {
		FIELD(i);
		FIELD(str);
	}
};

template <typename F>
SPtr<Field> setDeclaredField(StringView const& name, F field) {
	return newS<SpecificField<F>>(name, field);
}

TEST(ConfigTests, BasicTests)
{
	auto a = hasReflectionInfo<X> {};
	auto b = hasReflectionInfo<Object> {};
	auto c = hasTypeInfo<String> {};
	X x;
	SPtr<Field> f = setDeclaredField("field"_SV, &X::i);
	f->set(x, (int64_t)10);
	int64_t i = f->get<int64_t>(x);
	//int64_t j = f1->get<int32_t>(x);
	f = setDeclaredField("field"_SV, &X::str);
	f->set(x, "xxx"_SPTR);
	f = setDeclaredField("obj"_SV, &X::obj);
	f->set(x, "xxx"_SPTR);
	f = setDeclaredField("ustr"_SV, &X::ustr);
	f->set(x, String("yyy"));
	Class cls = classOf<X>::_class();

	fmt::print("config: {}\n", *config->getRoot()->toString());

	STRCMP_EQUAL("str123", config->getString("testName")->c_str());
	CHECK_EQUAL(3, config->getLong("testLong1"));
	CHECK_EQUAL(5, config->getLong("testObj.testLong2"));
	CHECK_EQUAL(4.7, config->getDouble("testObj.testDouble1"));
	CHECK_EQUAL(true, config->getBool("testObj.testBool1"));
	CHECK_EQUAL(6, config->getLong("testObj.testArray.*.b", {0}));
	CHECK_THROWS(MissingValueException, config->getLong("missing"));
	CHECK_THROWS(MissingValueException, config->getLong("missing"));

	fmt::print("hostname: {}", *config->getString("hostname"));
}
