#include "CppUTest/TestHarness.h"

#include "slib/collections/HashMap.h"
#include "slib/collections/ArrayList.h"
#include "slib/util/expr/ExpressionEvaluator.h"

using namespace slib;
using namespace slib::expr;

static SPtr<Map<String, Object>> vars;
static UPtr<Resolver> resolver;

TEST_GROUP(ExprTests) {
	void setup() {
		vars = std::make_shared<HashMap<String, Object>>();

		vars->emplace<String>("var1", "val1");
		vars->emplace<Integer>("var2", 2);
		vars->emplace<String>("var3", "val3");
		SPtr<ArrayList<Object>> list = std::make_shared<ArrayList<Object>>();
		vars->put("varr", std::dynamic_pointer_cast<Object>(list));
		list->emplace<Integer>(1);
		list->emplace<Integer>(2);
		list->emplace<Integer>(3);

		resolver = std::make_unique<MapResolver>(vars);
	}

	void teardown() {
		resolver = nullptr;
		vars = nullptr;
	}
};

std::unique_ptr<String> strEval(const char *expr) {
	return ExpressionEvaluator::strExpressionValue(std::make_shared<String>(expr), *resolver);
}

TEST(ExprTests, BasicTests) {
	STRCMP_EQUAL("0", strEval("1 + (-1)")->c_str());
	STRCMP_EQUAL("5", strEval("math.ceil(2.3) + math.floor(2.5)")->c_str());
}

TEST(ExprTests, FormatTests) {
	STRCMP_EQUAL("xxx:true:yyy:42.00", strEval("format('xxx:%b:%s:%.2f', true, 'yyy', 42)")->c_str());
}
