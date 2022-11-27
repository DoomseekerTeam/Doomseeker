//------------------------------------------------------------------------------
// testruns.cpp
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
#include "testruns.h"
#include "tests/testdatapaths.h"
#include "tests/testini.h"
#include "tests/testircisupportparser.h"
#include "tests/testmodreader.h"
#include "tests/testplayer.h"
#include "tests/teststrings.h"
#include "tests/testtemplatedpathresolver.h"
#include "tests/testutf8splitter.h"

TestCore *TestRuns::pTestCore = nullptr;

void TestRuns::callTests()
{
	// DataPaths
	pTestCore->executeTest(new TestDataPathsAppDataDirectoryAccess(false));
	pTestCore->executeTest(new TestDataPathsAppDataDirectoryAccess(true));

	pTestCore->executeTest(new TestDataPathsAppDataDirectoryWrite(false));
	pTestCore->executeTest(new TestDataPathsAppDataDirectoryWrite(true));

	// INI
	// Note: All of these tests may fail if TestReadINI fails.
	pTestCore->executeTest(new TestReadINI());
	pTestCore->executeTest(new TestReadINIVariable());
	pTestCore->executeTest(new TestDeleteINISection());
	pTestCore->executeTest(new TestDeleteINIVariable());
	pTestCore->executeTest(new TestStringsWrapUrls());

	pTestCore->executeTest(new TestIRCISupportPrefix());
	pTestCore->executeTest(new TestIRCISupportNoPrefix());

	// Templated Path Resolver
	pTestCore->executeTest(new TestTemplatedPathResolver());

	// WadReader.
	pTestCore->executeTest(new TestModReader());

	// Misc.
	pTestCore->executeTest(new TestUtf8Splitter());
	pTestCore->executeTest(new TestPlayerNameColorStrip());
}
