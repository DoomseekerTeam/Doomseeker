//------------------------------------------------------------------------------
// testbase.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __TESTBASE_H__
#define __TESTBASE_H__

#include "log.h"

/**
 *	@brief Base class for all Test classes.
 *
 *	Provides a simple way to access the log.
 */
class TestBase
{
protected:
	static Log testLog;
};

/**
 *	@brief Base class for Test Units.
 */
class TestUnitBase : public TestBase
{
public:
	TestUnitBase(const QString &testName);
	virtual ~TestUnitBase() {}

	virtual bool executeTest() = 0;

	const QString &testName() const { return _testName; }

protected:
	QString _testName;
};

#endif
