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

	bool _need_stdint_h;
	bool _need_slib_string_h;

	Context(StringBuilder const& objName)
	: _classIdx(1)
	, _objName(objName) {}
};

UPtr<String> genClass(StringBuilder &body, Context &ctx, Value const& obj, StringBuilder const& objName) {
	StringBuilder classSig("{");
	StringBuilder classDef;
	LinkedList<String> members;

	classDef.addFmtLine("struct %s {", objName.c_str());
	// may contain UPtr, let's not cause issues for reflection setter
	classDef.addFmtLine("\t%s& operator =(%s const&) = delete;", objName.c_str(), objName.c_str());
	classDef.addLine("");

	for (Value::ConstMemberIterator i = obj.MemberBegin(); i != obj.MemberEnd(); ++i) {
		SPtr<String> name = newS<String>(i->name.GetString());
		Value const& value = i->value;

		if (value.IsObject()) {
			if (value.HasMember("_type")) {
				// leaf node
				Value const& typeDesc = value["_type"];
				assert(typeDesc.IsString());
				String typeName = typeDesc.GetString();

				UPtr<String> type;

				if (typeName.equals("int64"_SV)) {
					type = "int64_t"_UPTR;
					ctx._need_stdint_h = true;
				} else if (typeName.equals("bool"_SV)) {
					type = "bool"_UPTR;
				} else if (typeName.equals("string"_SV)) {
					type = "slib::UPtr<slib::String>"_UPTR;
					ctx._need_slib_string_h = true;
				}

				if (type) {
					members.add(name);
					classSig.addFmt("<%s:%s>", name->c_str(), typeName.c_str());
					classDef.addFmtLine("\t%s %s;", type->c_str(), name->c_str());
				}
			} else {
				StringBuilder className(ctx._objName);
				className.add('_').add(ctx._classIdx);
				ctx._classIdx++;

				StringBuilder newClass;
				SPtr<String> localClassSig = genClass(newClass, ctx, value, className);
				SPtr<String> existingClassName = ctx._registry.get(*localClassSig);
				if (existingClassName)
					className = *existingClassName;
				else {
					body.add(newClass);
					ctx._registry.put(localClassSig, className.toString());
				}

				members.add(name);
				classSig.addFmt("<%s:%s>", name->c_str(), className.c_str());
				classDef.addFmtLine("\t%s %s;", className.c_str(), name->c_str());
			}
		}
	}

	classDef.addNewLine();
	classDef.addFmtLine("\tREFLECT(%s) {", objName.c_str());

	UPtr<ConstIterator<SPtr<String>>> i = members.constIterator();
	while (i->hasNext()) {
		SPtr<String> memberName = i->next();
		classDef.addFmtLine("\t\tFIELD(%s);", memberName->c_str());
	}

	classDef.addFmtLine("\t}");

	classDef.addFmtLine("};");
	classDef.addNewLine();
	classSig.add('}');

	body.add(classDef);

	return classSig.toString();
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

	StringBuilder classBody;

	StringBuilder className = spec["className"].GetString();
	Value const& classDef = spec["def"];

	Context ctx(className);

	genClass(classBody, ctx, classDef, className);

	FILE *cfp = fopen(classFile, "w");

	fprintf(cfp, "// Auto-generated from %s\n\n", defFile);

	if (ctx._need_slib_string_h)
		fprintf(cfp, "#include \"slib/lang/String.h\"\n");
	fprintf(cfp, "#include \"slib/lang/Reflection.h\"\n");
	if (ctx._need_stdint_h)
		fprintf(cfp, "#include <stdint.h>\n");

	fprintf(cfp, "\n");

	fwrite(classBody.c_str(), classBody.length(), 1, cfp);

	fclose(cfp);
}
