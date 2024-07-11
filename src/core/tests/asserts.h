//------------------------------------------------------------------------------
// asserts.h
//------------------------------------------------------------------------------
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//
//------------------------------------------------------------------------------
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_TESTS_ASSERTS_H
#define DOOMSEEKER_TESTS_ASSERTS_H

// TODO 3533 -- replace with Qt Test

#define _T_STR2(x) #x
#define _T_STR(x) _T_STR2(x)

#define _TLINE __FILE__ ":" _T_STR(__LINE__) ":"

#define T_ASSERT_DATETIME_EQUAL(expected, actual) { \
	auto _expected = (expected); \
	auto _actual = (actual); \
	if (!(_expected == _actual)) \
	{ \
		testLog << QString(_TLINE "expected: %1, got %2") \
			.arg(_expected.toString(Qt::TextDate)).arg(_actual.toString(Qt::TextDate)); \
		return false; \
	}}

#define T_ASSERT_EQUAL(expected, actual) { \
	auto _expected = (expected); \
	auto _actual = (actual); \
	if (!(_expected == _actual)) \
	{ \
		testLog << QString(_TLINE "expected: %1, got %2").arg(_expected).arg(_actual); \
		return false; \
	}}

#define T_ASSERT_FALSE(actual) { \
	auto _actual = (actual); \
	if (_actual) \
	{ \
		testLog << QString(_TLINE "expected: false, got %2").arg(_actual); \
		return false; \
	}}

#define T_ASSERT_ISEMPTY(actual) { \
	auto _isEmpty = (actual).isEmpty(); \
	if (!_isEmpty) \
	{ \
		testLog << QString(_TLINE "`" #actual "` expected to be empty"); \
		return false; \
	}}

#define T_ASSERT_SIZE(expected, actual) { \
	auto _expected = (expected); \
	auto _actual = (actual).size(); \
	if (!(_expected == _actual)) \
	{ \
		testLog << QString(_TLINE "`" #expected " == " #actual ".size()`" \
			" expected size: %1, got: %2").arg(_expected).arg(_actual); \
		return false; \
	}}

#endif
