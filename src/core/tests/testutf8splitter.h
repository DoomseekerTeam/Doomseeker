//------------------------------------------------------------------------------
// testutf8splitter.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id8014C7A9_BBB7_40EE_A1DA3CEA56CAD14B
#define id8014C7A9_BBB7_40EE_A1DA3CEA56CAD14B

#include "tests/testbase.h"

class TestUtf8Splitter : public TestUnitBase
{
public:
	TestUtf8Splitter()
		: TestUnitBase("UTF-8 Splitter") {}

	bool executeTest() override;
};


#endif // header
