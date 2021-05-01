#include "CppUTest/TestHarness.h"

#include "slib/collections/LinkedList.h"

using namespace slib;

TEST_GROUP(LinkedListTests) {

};

TEST(LinkedListTests, BasicTests) {
	LinkedList<String> s;

	s.push("a"_SPTR);
	s.push("b"_SPTR);
	s.push("c"_SPTR);

	STRCMP_EQUAL("[c, b, a]", s.toString()->c_str());

	UNSIGNED_LONGLONGS_EQUAL(s.size(), 3);
	CHECK(s.peek()->equals("c"_SV));

	s.pop();

	UNSIGNED_LONGLONGS_EQUAL(s.size(), 2);
	CHECK(s.peek()->equals("b"_SV));

}

