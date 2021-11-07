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
	/*static void _registerReflection(ReflectionInfo &ri) {
		ri.registerField("i"_SV, &X::i);
	}

	static ClassReflectionInfo *_getReflectionInfo() {
		static ReflectionInfo ri(&_registerReflection);
		return &ri;
	}*/

	/*template<typename T>
	static void _registerReflection(ReflectionInfo &ri) {
		ri.registerField("i"_SV, &T::i);
	}

	static ClassReflectionInfo *_getReflectionInfo() {
		static ReflectionInfo ri(&_registerReflection<X>);
		return &ri;
	}*/

	/*REFLECT(CLASS(X),
		FIELD(i),
		FIELD(str)
	);*/

	REFLECT(X) {
		FIELD(i64);
		FIELD(i);
		FIELD(str);
		FIELD(ustr);
		FIELD(sint);
	}
};

template <typename F>
SPtr<Field> setDeclaredField(StringView const& name, F field) {
	return newS<SpecificField<F>>(name, field);
}

TEST(ReflectionTests, BasicTests)
{
	auto a = hasReflectionInfo<X> {};
	auto b = hasReflectionInfo<Object> {};
	auto c = hasTypeInfo<String> {};
	X x;
	SPtr<Field> f = setDeclaredField("field"_SV, &X::i64);
	f->set(x, (int64_t)10);
	int64_t i = f->get<int64_t>(x);
	//int64_t j = f1->get<int32_t>(x);
	f = setDeclaredField("field"_SV, &X::str);
	f->set(x, "xxx"_SPTR);
	f = setDeclaredField("obj"_SV, &X::obj);
	f->set(x, "xxx"_SPTR);
	Class cls = classOf<X>::_class();
	f = cls.getDeclaredField("ustr");
	f->set(x, String("yyy"));
	f = cls.getDeclaredField("i64");
	f->set(x, (int64_t)42);
	f->set(x, Long(43));
	f = cls.getDeclaredField("i");
	f = cls.getDeclaredField("sint");
	f->set(x, newS<uint64_t>(42));
	UPtr<Array<Field>> fields = cls.getDeclaredFields();
}
