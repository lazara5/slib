/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "slib/lang/Field.h"

namespace slib {

namespace internal {

static int64_t fieldGetInt64(FieldValue const& val) {
	if ((val._type & FieldValType::NUMBER) != 0) {
		switch (val._type) {
			case FieldValType::P_INT64:
				return nonstd::get<int64_t>(val._val);
			case FieldValType::DOUBLE: {
				double d = nonstd::get<double>(val._val);
				if (Number::isMathematicalInteger(d)) {
					if ((d <= Double::MAX_SAFE_INTEGER) && (d >= Double::MIN_SAFE_INTEGER))
						return (int64_t)d;
				}
			}
			default:
				break;
		}
	}
	THROW(IllegalArgumentException);
}

} // namespace internal

void *Field::autoBox(const void *src, Class const& srcType, internal::FieldValue &dst, Class const& dstType) {
	if (dstType.isAssignableFrom(srcType)) {
		if (srcType._hasTypeInfo) {
			TypedClass *tVal = (TypedClass *)src;
			void *castVal = tVal->__classCast(dstType.getTypeId());
			if (castVal) {
				dst._type = internal::FieldValType::VOID;
				dst._val = castVal;
				return castVal;
			}
		}
		THROW(IllegalArgumentException);
	} else {
		internal::FieldValue fieldValue;

		if (classOf<Number>::_class().isAssignableFrom(srcType)) {
			if (classOf<Long>::_class().isAssignableFrom(srcType)) {
				fieldValue._type = internal::FieldValType::P_INT64;
				fieldValue._val = (static_cast<const Long *>(src))->longValue();
			}
		} else if (classOf<Boolean>::_class().isAssignableFrom(srcType)) {
			fieldValue._type = internal::FieldValType::P_BOOL;
			fieldValue._val = (static_cast<const Boolean *>(src))->booleanValue();
		} else if (srcType.isPrimitive()) {
			if (classOf<int64_t>::_class().isAssignableFrom(srcType)) {
				fieldValue._type = internal::FieldValType::P_INT64;
				fieldValue._val = *(static_cast<const int64_t *>(src));
			}
		}

		if ((fieldValue._type & internal::FieldValType::NUMBER) != 0) {
			if (classOf<Number>::_class().isAssignableFrom(dstType)) {
				if (classOf<Long>::_class().isAssignableFrom(dstType)) {
					dst._type = internal::FieldValType::LONG;
					dst._val = Long(fieldGetInt64(fieldValue));
					return &(dst._val.get<Long>());
				}
			} else if (dstType.isPrimitive()) {
				if (classOf<int64_t>::_class().isAssignableFrom(dstType)) {
					dst._type = internal::FieldValType::P_INT64;
					dst._val = fieldGetInt64(fieldValue);
					return &(dst._val.get<int64_t>());
				}
			}
			THROW(IllegalArgumentException);
		} else if (fieldValue._type == internal::FieldValType::P_BOOL) {
			if (classOf<Boolean>::_class().isAssignableFrom(dstType)) {
				dst._type = internal::FieldValType::BOOLEAN;
				dst._val = Boolean(nonstd::get<bool>(fieldValue._val));
				return &(dst._val.get<Boolean>());
			} else if (classOf<bool>::_class().isAssignableFrom(dstType)) {
				dst._type = internal::FieldValType::P_BOOL;
				dst._val = nonstd::get<bool>(fieldValue._val);
				return &(dst._val.get<bool>());
			}
			THROW(IllegalArgumentException);
		}

		THROW(IllegalArgumentException);
	}
}

} // namespace slib
