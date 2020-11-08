#include "CppUTest/TestHarness.h"

#include "slib/lang/Numeric.h"

using namespace slib;

TEST_GROUP(NumericTests) {

};

TEST(NumericTests, BasicTests) {
	Integer i1 = Integer::parseInt("123");
	LONGS_EQUAL(i1, 123);
	UInt ui1 = UInt::parseUInt("123");
	LONGS_EQUAL(ui1, 123);
	CHECK_THROWS(NumberFormatException, ui1 = UInt::parseUInt("-123"));
};
