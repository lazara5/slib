/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef H_SLIB_UTIL_MACROUTILS_H
#define H_SLIB_UTIL_MACROUTILS_H

// FOREACH macro
// based on https://stackoverflow.com/a/11994395
#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X, 0)
#define FE_2(WHAT, X, ...) WHAT(X, 1)FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X, 2)FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X, 3)FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X, 4)FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X, 5)FE_5(WHAT, __VA_ARGS__)

#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,NAME,...) NAME

#define FE_ARG_0(ARG, WHAT)
#define FE_ARG_1(ARG, WHAT, X) WHAT(ARG, X, 0)
#define FE_ARG_2(ARG, WHAT, X, ...) WHAT(ARG, X, 1)FE_ARG_1(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_3(ARG, WHAT, X, ...) WHAT(ARG, X, 2)FE_ARG_2(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_4(ARG, WHAT, X, ...) WHAT(ARG, X, 3)FE_ARG_3(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_5(ARG, WHAT, X, ...) WHAT(ARG, X, 4)FE_ARG_4(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_6(ARG, WHAT, X, ...) WHAT(ARG, X, 5)FE_ARG_5(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_7(ARG, WHAT, X, ...) WHAT(ARG, X, 6)FE_ARG_6(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_8(ARG, WHAT, X, ...) WHAT(ARG, X, 7)FE_ARG_7(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_9(ARG, WHAT, X, ...) WHAT(ARG, X, 8)FE_ARG_8(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_10(ARG, WHAT, X, ...) WHAT(ARG, X, 9)FE_ARG_9(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_11(ARG, WHAT, X, ...) WHAT(ARG, X, 10)FE_ARG_10(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_12(ARG, WHAT, X, ...) WHAT(ARG, X, 11)FE_ARG_11(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_13(ARG, WHAT, X, ...) WHAT(ARG, X, 12)FE_ARG_12(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_14(ARG, WHAT, X, ...) WHAT(ARG, X, 13)FE_ARG_13(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_15(ARG, WHAT, X, ...) WHAT(ARG, X, 14)FE_ARG_14(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_16(ARG, WHAT, X, ...) WHAT(ARG, X, 15)FE_ARG_15(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_17(ARG, WHAT, X, ...) WHAT(ARG, X, 16)FE_ARG_16(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_18(ARG, WHAT, X, ...) WHAT(ARG, X, 17)FE_ARG_17(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_19(ARG, WHAT, X, ...) WHAT(ARG, X, 18)FE_ARG_18(ARG, WHAT, __VA_ARGS__)
#define FE_ARG_20(ARG, WHAT, X, ...) WHAT(ARG, X, 19)FE_ARG_19(ARG, WHAT, __VA_ARGS__)

#define GET_ARG_MACRO( \
	_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10, \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
	NAME,...) NAME

#define FOR_EACH(action,...) \
	GET_MACRO(_0,__VA_ARGS__,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0,)(action,__VA_ARGS__)

#define FOR_EACH_ARG(arg,action,...) \
	GET_ARG_MACRO(_0,__VA_ARGS__, \
	FE_ARG_20,FE_ARG_19,FE_ARG_18,FE_ARG_17,FE_ARG_16,FE_ARG_15,FE_ARG_14,FE_ARG_13,FE_ARG_12,FE_ARG_11, \
	FE_ARG_10,FE_ARG_9,FE_ARG_8,FE_ARG_7,FE_ARG_6,FE_ARG_5,FE_ARG_4,FE_ARG_3,FE_ARG_2,FE_ARG_1,FE_ARG_0, \
	)(arg,action,__VA_ARGS__)

// Conditional expansion
// based on https://stackoverflow.com/a/41283294

// for MSVC compatibility in the unlikely case it's ever needed...
#define EXPAND(...) __VA_ARGS__

#define JOIN_EXPAND( a , b )	a##b
#define JOIN( a , b )			JOIN_EXPAND( a , b )

#define SECOND_EXPAND( a , b , ... )	b
#define SECOND(...)						EXPAND( SECOND_EXPAND( __VA_ARGS__ ) )

#define HIDDEN0					unused,0
#define CHECK0( value )			SECOND( JOIN( HIDDEN , value ) , 1 , unused )

// Conditional expansion implementation for conditional comma

#define COMMA0
#define COMMA1 ,
#define COMMA_IF( value )   JOIN( COMMA , CHECK0( value ) )

/** Unused symbol annotation */
#define SLIB_UNUSED __attribute__((unused))

/** Forces packed data structures */
#define SLIB_PACKED __attribute__((__packed__))

/**
 * Hint to the compiler that the expression e is likely to be true
 *
 * @param e  Expression
 */
#define SLIB_LIKELY(e) __builtin_expect((e), 1)

/**
 * Hint to the compiler that the expression e is likely to be false
 *
 * @param e  Expression
 */
#define SLIB_UNLIKELY(e) __builtin_expect((e), 0)

#endif // H_SLIB_UTIL_MACROUTILS_H
