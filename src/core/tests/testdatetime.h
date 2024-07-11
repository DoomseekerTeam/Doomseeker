//------------------------------------------------------------------------------
// testdatetime.h
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
#ifndef DOOMSEEKER_TESTS_TESTDATETIME_H
#define DOOMSEEKER_TESTS_TESTDATETIME_H

#include "tests/testbase.h"

class TestDateTimeISO8601 : public TestUnitBase
{
public:
	TestDateTimeISO8601()
		: TestUnitBase("Test ISO 8601 formatter") {}

	bool executeTest() override;
};

class TestDateTimeToPathFriendlyUTCISO8601 : public TestUnitBase
{
public:
	TestDateTimeToPathFriendlyUTCISO8601()
		: TestUnitBase("Test path-friendly ISO 8601 UTC formatter") {}

	bool executeTest() override;
};

class TestDateTimeParsePathFriendlyUTCISO8601 : public TestUnitBase
{
public:
	TestDateTimeParsePathFriendlyUTCISO8601()
		: TestUnitBase("Test parse path-friendly ISO 8601 in UTC") {}

	bool executeTest() override;
};

#endif
