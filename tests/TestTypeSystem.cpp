#include "CppUTest/TestHarness.h"

#include "slib/lang/Numeric.h"

using namespace slib;

TEST_GROUP(TypeSystem) {
};

TEST(TypeSystem, InstanceOf) {
	Integer i1(3);
	CHECK(instanceof<Number>(i1));
	CHECK(instanceof<Integer>(i1));
	CHECK_FALSE(instanceof<String>(i1));
}
