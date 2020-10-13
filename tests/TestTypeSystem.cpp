#include "CppUTest/TestHarness.h"

#include "slib/lang/Numeric.h"
#include "slib/collections/LinkedHashMap.h"

using namespace slib;

TEST_GROUP(TypeSystem) {
};

TEST(TypeSystem, InstanceOf) {
	Integer i1(3);
	String s1("str");
	LinkedHashMap<String, Object> lhm1;

	CHECK(instanceof<Number>(i1));
	CHECK(instanceof<Integer>(i1));
	CHECK_FALSE(instanceof<String>(i1));

	CHECK(instanceof<BasicString>(s1));
	CHECK(instanceof<String>(s1));
	CHECK_FALSE(instanceof<Number>(s1));

	CHECK(instanceof<Object>(lhm1));
	CHECK((instanceof<Map<String, Object>>(lhm1)));
	CHECK((instanceof<HashMap<String, Object>>(lhm1)));
	CHECK((instanceof<LinkedHashMap<String, Object>>(lhm1)));
	CHECK_FALSE(instanceof<Number>(lhm1));
}
