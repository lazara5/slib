/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/Class.h"

namespace slib {

Class objectClass("Object", OBJECTID);
Class integerClass("Integer", INTEGERID);
Class uIntClass("UInt", UINTID);
Class longClass("Long", LONGID);
Class uLongClass("ULong", ULONGID);
Class doubleClass("Double", DOUBLEID);
Class priorityQueueClass("PriorityQueue", PRIORITYQUEUEID);
Class arrayListClass("ArayList", ARRAYLISTID);
Class hashMapClass("HashMap", HASHMAPID);
Class linkedHashMapClass("LinkedHashMap", LINKEDHASHMAPID);
Class propertiesClass("Properties", PROPERTIESID);
Class stringClass("String", STRINGID);
Class stringBuilderClass("StringBuilder", STRINGBUILDERID);

} // namespace slib
