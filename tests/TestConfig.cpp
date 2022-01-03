#include "CppUTest/TestHarness.h"

#include "slib/util/Config.h"
#include "slib/util/SystemInfo.h"
#include "slib/util/FilenameUtils.h"

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
			.withResolver("system"_SPTR, newS<SystemInfo>())
			.option("testName", OptionType::STRING)
			.option("hostname", OptionType::STRING)
			.option("testLong1", OptionType::LONG)
			.option("testObj.testLong2", OptionType::LONG).range(1, 10)
			.option("testObj.testLong3", OptionType::LONG)
			.option("testObj.testLong4", OptionType::LONG).range(6, 8).defaultValue("7")
			.option("testObj.testObj2.testLong4", OptionType::LONG)
			.option("testObj.testArray2.[]", OptionType::LONG)
			.option("testObj.testArray3.[].name", OptionType::STRING)
			.option("testObj.testArray3.[].type", OptionType::LONG);
		config = configLoader.load();
		//config = newU<TestConfig>(*FileUtils::buildPath(FileUtils::getPath(test_argv[0]), "data/test.conf"), "SlibTest");
		//config = newU<TestConfig>("test.conf", "SlibTest");
	}

	void teardown() {
		config = nullptr;
	}
};

TEST(ConfigTests, BasicTests) {
	fmt::print("config: {}\n", *config->getRoot()->toString());

	STRCMP_EQUAL("str123", config->getString("testName")->c_str());
	CHECK_EQUAL(3, config->getLong("testLong1"));
	CHECK_EQUAL(5, config->getLong("testObj.testLong2"));
	CHECK_EQUAL(4.7, config->getDouble("testObj.testDouble1"));
	CHECK_EQUAL(true, config->getBool("testObj.testBool1"));
	CHECK_EQUAL(6, config->getLong("testObj.testArray.[].b", {0}));
	CHECK_EQUAL(7, config->getLong("testObj.testLong4"));
	CHECK_THROWS(MissingValueException, config->getLong("missing"));
	CHECK_THROWS(MissingValueException, config->getLong("missing"));

	fmt::print("hostname: {}", *config->getString("hostname"));
}

TEST(ConfigTests, MapObjTest) {
	auto a = std::is_assignable<ConfigObject_2&, ConfigObject_2>::value;
	SPtr<String> path = FilenameUtils::concat(FilenameUtils::getPath(test_argv[0]), "data/testObj.conf");
	ConfigLoader configLoader(path, "SlibObjTest"_SPTR);
	SPtr<Config> cfg = configLoader.load();
	fmt::print("config: {}\n", *cfg->getRoot()->toString());
	SPtr<Object> cfgRoot = cfg->getRoot();
	SPtr<ConfigObject> cfgObj = configLoader.toObject<ConfigObject>(cfg);
	cfgRoot = cfg->getRoot();
}

struct TestConfig {
	String str;

	REFLECT(TestConfig) {
		FIELD(str);
	}
};

TEST(ConfigTests, MapObjTest1) {
	const char *cfgString = "str = 'xxx'";
	ConfigLoader configLoader(nullptr, "SlibObjTest"_SPTR);
	SPtr<Config> cfg = configLoader.load(cfgString);
	//SPtr<TestConfig> cfgObj = configLoader.toObject<TestConfig>(cfg);

}
