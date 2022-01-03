#include "CppUTest/TestHarness.h"

#include "slib/lang/Numeric.h"
#include "slib/collections/LinkedHashMap.h"
#include "slib/lang/Array.h"

using namespace slib;

TEST_GROUP(TypeSystem) {
};

TEST(TypeSystem, InstanceOf) {
	Integer i1(3);
	String s1("str");
	LinkedHashMap<String, Object> lhm1;
	Array<String> as;
	Array<int> ai;

	CHECK(instanceof<Number>(i1));
	CHECK(instanceof<Integer>(i1));
	CHECK_FALSE(instanceof<String>(i1));
	CHECK(StringView::equals(i1.getClass().getName(), "Integer"));

	CHECK(instanceof<IString>(s1));
	CHECK(instanceof<String>(s1));
	CHECK_FALSE(instanceof<Number>(s1));

	CHECK(instanceof<Object>(lhm1));
	CHECK((instanceof<Map<String, Object>>(lhm1)));
	CHECK((instanceof<HashMap<String, Object>>(lhm1)));
	CHECK((instanceof<LinkedHashMap<String, Object>>(lhm1)));
	CHECK_FALSE((instanceof<LinkedHashMap<String, String>>(lhm1)));
	CHECK_FALSE(instanceof<Number>(lhm1));

	HashMap<std::string, uint64_t> hm1;
	CHECK((instanceof<Map<std::string, uint64_t>>(hm1)));

	CHECK(StringView::equals(classOf<int64_t>::_class().getName(), "long"));

	Class const& asc = as.getClass();
	Class const& ascc = asc.getComponentClass();
	CHECK(asc.isArray());
	Class const& aic = ai.getClass();
	Class const& aicc = aic.getComponentClass();
	CHECK(aic.isArray());
}
