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
#define FOR_EACH(action,...) \
  GET_MACRO(_0,__VA_ARGS__,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0,)(action,__VA_ARGS__)

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

#endif // H_SLIB_UTIL_MACROUTILS_H
