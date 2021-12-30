#include "CppUTest/TestHarness.h"

#include "slib/lang/Reflection.h"

using namespace slib;

TEST_GROUP(ReflectionTests) {

};

class X {
public:
	int64_t i64;
	int i;
	SPtr<String> str;
	SPtr<Object> obj;
	UPtr<String> ustr;
	SPtr<int> sint;
public:
	REFLECT(X) {
		FIELD(i64);
		FIELD(i);
		FIELD(str);
		FIELD(obj);
		FIELD(ustr);
		FIELD(sint);
	}
};

struct Z {
	int64_t m1;
	REFLECT(Z) {
		FIELD(m1);
	}
};

struct Y {
	int64_t m2;
	Z z;
	REFLECT(Y) {
		FIELD(m2);
		FIELD(z);
	}
};

TEST(ReflectionTests, BasicTests)
{
	auto a = hasReflectionInfo<X> {};
	auto b = hasReflectionInfo<Object> {};
	auto c = hasTypeInfo<String> {};
	auto d = hasTypeInfo<uint64_t> {};
	auto e = std::is_assignable<String, String>::value;
	X x;
	Class cls = classOf<X>::_class();
	SPtr<Field> f = cls.getDeclaredField("i64"_SV);
	f->set(x, (int64_t)10);
	int64_t i = f->get<int64_t>(x);
	//int64_t j = f1->get<int32_t>(x);
	f = cls.getDeclaredField("str");
	f->set(x, "xxx"_SPTR);
	f = cls.getDeclaredField("obj");
	f->set(x, "xxx"_SPTR);
	f = cls.getDeclaredField("ustr");
	f->set(x, String("yyy"));
	f = cls.getDeclaredField("i64");
	f->set(x, (int64_t)42);
	f->set(x, Long(43));
	f = cls.getDeclaredField("i");
	f = cls.getDeclaredField("sint");
//	f->set(x, newS<int64_t>(42));
	UPtr<Array<Field>> fields = cls.getDeclaredFields();

	SPtr<Y> y = newS<Y>();
	ObjRef const& yref = ObjRef(y.get(), classOf<Y>::_class());
	Class ycls = classOf<Y>::_class();
	f = ycls.getDeclaredField("z");
	ObjRef const& ref = f->getRef(yref);
	Class zcls = classOf<Z>::_class();
	SPtr<Field> f1 = zcls.getDeclaredField("m1");
	f1->set(ref, (int64_t)42);
	f = ycls.getDeclaredField("z");
}
