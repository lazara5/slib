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
		vars = newS<HashMap<String, Object>>();

		vars->emplace<String>("var1", "val1");
		vars->emplace<Integer>("var2", 2);
		vars->emplace<String>("var3", "val3");
		SPtr<ArrayList<Object>> list = newS<ArrayList<Object>>();
		vars->put("varr", std::dynamic_pointer_cast<Object>(list));
		list->emplace<Integer>(1);
		list->emplace<Integer>(2);
		list->emplace<Integer>(3);

		auto map = newS<HashMap<Object, Object>>();
		vars->put("oo", map);
		map->emplace<Integer, String>(3, "xxx");

		auto map1 = newS<HashMap<Object, Object>>();
		auto *map2 = dynamic_cast<Map<Object, Object>*>(map1.get());

		resolver = newU<MapResolver>(vars);
	}

	void teardown() {
		resolver = nullptr;
		vars = nullptr;
	}
};

UPtr<String> strEval(const char *expr) {
	return ExpressionEvaluator::strExpressionValue(newS<String>(expr), *resolver);
}

TEST(ExprTests, BasicTests) {
	STRCMP_EQUAL("0", strEval("1 + (-1)")->c_str());
	STRCMP_EQUAL("2.5", strEval("(7 - 2)/2")->c_str());
	STRCMP_EQUAL("5", strEval("math.ceil(2.3) + math.floor(2.5)")->c_str());
}

TEST(ExprTests, FormatTests) {
	STRCMP_EQUAL("xxx:true:yyy:42.00", strEval("format('xxx:%b:%s:%.2f', true, 'yyy', 42)")->c_str());
}

TEST(ExprTests, ExtraTests) {
	UPtr<String> res = strEval("oo[3]");
	printf("\n");

	SPtr<Object> res1 = ExpressionEvaluator::expressionValue(newS<String>("{a = 3, b = 2 * (2 + 1), c = {d = '123', e = 1 + 2}}"), *resolver);
	UPtr<String> res2 = res1->toString();
	CHECK((instanceof<Map<String, Object>>(res1)));
	STRCMP_EQUAL("{a=3, b=6, c={d=123, e=3}}", res2->c_str());

	res1 = ExpressionEvaluator::expressionValue(newS<String>("[1, 2, 3 * 5, {a = 'b', c = [1, 'x'], d = math.abs(-2), e = -1}]"), *resolver);
	res2 = res1->toString();
	CHECK((instanceof<List<Object>>(res1)));
	STRCMP_EQUAL("[1, 2, 15, {a=b, c=[1, x], d=2, e=-1}]", res2->c_str());
}
