#include "slib/lang/String.h"
#include "slib/lang/StringBuilder.h"
#include "slib/collections/HashMap.h"
#include "slib/collections/LinkedList.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

using namespace slib;
using namespace rapidjson;

struct Context {
	uint64_t _classIdx;
	HashMap<String, String> _registry;
	StringBuilder _objName;

	StringBuilder _body;

	bool _need_stdint_h;
	bool _need_slib_string_h;
	bool _need_slib_array_h;

	Context(StringBuilder const& objName)
	: _classIdx(1)
	, _objName(objName) {}
};

struct ClassDesc {
	SPtr<String> _name;
	bool _needsRef;
	SPtr<String> _sig;

	ClassDesc(SPtr<String> const& name, bool needsRef, SPtr<String> const& sig)
	: _name(name)
	, _needsRef(needsRef)
	, _sig(sig) {}
};

UPtr<ClassDesc> getClass(Value const& obj, Context &ctx, bool topLevel) {
	if (obj.IsObject()) {
		if (obj.HasMember("_type")) {
			// leaf node
			Value const& typeDesc = obj["_type"];
			assert(typeDesc.IsString());
			String typeName = typeDesc.GetString();

			if (typeName.equals("int64"_SV)) {
				ctx._need_stdint_h = true;
				return newU<ClassDesc>("int64_t"_UPTR, false, "int64"_UPTR);
			} else if (typeName.equals("bool"_SV)) {
				return newU<ClassDesc>("bool"_UPTR, false, "bool"_UPTR);
			} else if (typeName.equals("string"_SV)) {
				ctx._need_slib_string_h = true;
				return newU<ClassDesc>("slib::String"_UPTR, true, "string"_UPTR);
			}
		} else {
			StringBuilder localClassName;
			if (!topLevel) {
				localClassName = ctx._objName;
				localClassName.add('_').add(ctx._classIdx);
				ctx._classIdx++;
			}

			StringBuilder const& className = topLevel ? ctx._objName : localClassName;
			StringBuilder classBody;
			StringBuilder classSig("{");
			LinkedList<String> members;

			for (Value::ConstMemberIterator i = obj.MemberBegin(); i != obj.MemberEnd(); ++i) {
				SPtr<String> memberName = newS<String>(i->name.GetString());
				Value const& memberValue = i->value;

				UPtr<ClassDesc> memberClassDesc = getClass(memberValue, ctx, false);
				members.add(memberName);
				SPtr<String> const& memberClassName = memberClassDesc->_name;
				classSig.addFmt("<%s:%s>", memberName->c_str(), memberClassDesc->_sig->c_str());
				if (!memberClassDesc->_needsRef)
					classBody.addFmtLine("\t%s %s;", memberClassName->c_str(), memberName->c_str());
				else
					classBody.addFmtLine("\tslib::UPtr<%s> %s;", memberClassName->c_str(), memberName->c_str());
			}

			classSig.add('}');

			SPtr<String> existingClassName = ctx._registry.get(classSig);
			if (existingClassName) {
				// roll back new class
				if (!topLevel)
					ctx._classIdx--;

				return newU<ClassDesc>(existingClassName, false, classSig.toString());
			}

			ctx._registry.put(classSig.toString(), localClassName.toString());
			printf(">'%s'-> '%s'\n", classSig.c_str(), localClassName.c_str());

			ctx._body.addFmtLine("struct %s {", className.c_str());
			// may contain UPtr, let's not cause issues for reflection setter
			ctx._body.addFmtLine("\t%s& operator =(%s const&) = delete;", className.c_str(), className.c_str());
			ctx._body.addLine("");

			ctx._body.add(classBody);

			ctx._body.addNewLine();
			ctx._body.addFmtLine("\tREFLECT(%s) {", className.c_str());

			UPtr<ConstIterator<SPtr<String>>> i = members.constIterator();
			while (i->hasNext()) {
				SPtr<String> memberName = i->next();
				ctx._body.addFmtLine("\t\tFIELD(%s);", memberName->c_str());
			}

			ctx._body.addFmtLine("\t}");
			ctx._body.addFmtLine("};");
			ctx._body.addNewLine();

			return newU<ClassDesc>(className.toString(), false, classSig.toString());
		}
	} else if (obj.IsArray()) {
		if (obj.Size() == 1) {
			ctx._need_slib_array_h = true;

			Value const& typeDef = obj[0];
			UPtr<ClassDesc> classDesc = getClass(typeDef, ctx, false);

			StringBuilder arrayClassName;
			arrayClassName.addFmt("slib::Array<%s>", classDesc->_name->c_str());

			StringBuilder arraySig;
			arraySig.addFmt("[%s]", classDesc->_name->c_str());

			return newU<ClassDesc>(arrayClassName.toString(), false, arraySig.toString());
		}
	}
}

int main(int argc, char **argv) {
	const char *defFile = argv[1];
	const char *classFile = argv[2];

	char buffer[0xffff];

	FILE *fp = fopen(defFile, "r");
	FileReadStream input(fp, buffer, sizeof(buffer));
	Document spec;
	spec.ParseStream(input);
	fclose(fp);

	StringBuilder className = spec["className"].GetString();
	Value const& classDef = spec["def"];

	Context ctx(className);

	getClass(classDef, ctx, true);

	FILE *cfp = fopen(classFile, "w");

	fprintf(cfp, "// Auto-generated from %s\n\n", defFile);

	if (ctx._need_slib_string_h)
		fprintf(cfp, "#include \"slib/lang/String.h\"\n");
	if (ctx._need_slib_array_h)
		fprintf(cfp, "#include \"slib/lang/Array.h\"\n");
	fprintf(cfp, "#include \"slib/lang/Reflection.h\"\n");
	if (ctx._need_stdint_h)
		fprintf(cfp, "#include <stdint.h>\n");

	fprintf(cfp, "\n");

	fwrite(ctx._body.c_str(), ctx._body.length(), 1, cfp);

	fclose(cfp);
}
