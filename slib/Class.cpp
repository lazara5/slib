/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/Class.h"
#include "slib/compat/cppbits/make_unique.h"

#include "fmt/format.h"

#define CLASSDEF(instName, className) const Class *instName ## CLASS() { static Class inst(#className, instName ## ID); return &inst; }

namespace slib {

ClassCastException::ClassCastException(const char *where, const char *c1, const char *c2)
:Exception(where, "ClassCastException", fmt::format("Cannot cast from {} to {}", c1, c2).c_str()) {}

CLASSDEF(OBJECT, Object)
CLASSDEF(NUMBER, Number)
CLASSDEF(INTEGER, Integer)
CLASSDEF(UINT, UInt)
CLASSDEF(LONG, Long)
CLASSDEF(ULONG, ULong)
CLASSDEF(DOUBLE, Double)
CLASSDEF(PRIORITYQUEUE, PriorityQueue)
CLASSDEF(ARRAYLIST, ArrayList)
CLASSDEF(MAP, Map)
CLASSDEF(HASHMAP, HashMap)
CLASSDEF(LINKEDHASHMAP, LinkedHashMap)
CLASSDEF(PROPERTIES, Properties)
CLASSDEF(BASICSTRING, BasicString)
CLASSDEF(STRING, String)
CLASSDEF(STRINGBUILDER, StringBuilder)
CLASSDEF(BOOLEAN, Boolean)
// Expression evaluator
CLASSDEF(RESOLVER, Resolver)




} // namespace slib
