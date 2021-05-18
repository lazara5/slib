#include "CppUTest/TestHarness.h"

#include "slib/util/Config.h"
#include "slib/util/SystemInfo.h"
#include "slib/util/FilenameUtils.h"

#include "Global.h"

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
		SPtr<String> path = FilenameUtils::concat(CPtr(FilenameUtils::getPath(test_argv[0])), "data/test.conf");
		printf("path: %s\n", path->c_str());
		ConfigLoader configLoader(path, "SlibTest"_SPTR);
		configLoader
		.clearPaths()
		.search("data"_SPTR)
		.withResolver("system"_SPTR, newS<SystemInfo>());
		config = configLoader.load();
		//config = newU<TestConfig>(*FileUtils::buildPath(CPtr(FileUtils::getPath(test_argv[0])), "data/test.conf"), "SlibTest");
		//config = newU<TestConfig>("test.conf", "SlibTest");
	}

	void teardown() {
		config = nullptr;
	}
};

TEST(ConfigTests, BasicTests)
{
	//config->init();
	STRCMP_EQUAL("str123", config->getString("testName")->c_str());
	fmt::print("hostname: {}", *config->getString("hostname"));
}
