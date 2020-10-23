#include "CppUTest/TestHarness.h"

#include "slib/util/Config.h"
#include "slib/util/SystemInfo.h"
#include "slib/util/FileUtils.h"

#include "Global.h"

using namespace slib;

class TestConfig : public Config {
private:
	SPtr<slib::SystemInfo> _systemInfo;
public:
	TestConfig(String const& confFileName, String const& appName)
	:Config(confFileName, appName)
	,_systemInfo(std::make_shared<slib::SystemInfo>()) {
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
}

static UPtr<TestConfig> config;

TEST_GROUP(ConfigTests)
{
	void setup() {
		config = std::make_unique<TestConfig>(*FileUtils::buildPath(*FileUtils::getPath(test_argv[0]), "data/test.conf"), "SlibTest");
		//config = std::make_unique<TestConfig>("test.conf", "SlibTest");
	}

	void teardown() {
		config = nullptr;
	}
};

TEST(ConfigTests, BasicTests)
{
	config->init();
	STRCMP_EQUAL("str123", config->getProperty("testName")->c_str());
	fmt::print("hostname: {}", *config->getProperty("hostname"));
}
