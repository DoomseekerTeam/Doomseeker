//------------------------------------------------------------------------------
// testircisupportparser.h
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
#pragma once
#ifndef id2A069239_73F2_4359_82ECDB738CF531BF
#define id2A069239_73F2_4359_82ECDB738CF531BF

#include "tests/testbase.h"

class TestIRCISupportPrefix : public TestUnitBase
{
public:
	TestIRCISupportPrefix()
		: TestUnitBase("RPL_ISUPPORT - PREFIX") {}

	bool executeTest() override;
};

class TestIRCISupportNoPrefix : public TestUnitBase
{
public:
	TestIRCISupportNoPrefix()
		: TestUnitBase("RPL_ISUPPORT - No PREFIX") {}

	bool executeTest() override;
};


#endif // header
