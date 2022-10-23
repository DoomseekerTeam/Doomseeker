//------------------------------------------------------------------------------
// testcore.h
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
#ifndef __TESTCORE_H__
#define __TESTCORE_H__

#include "tests/testbase.h"
#include <QString>

/**
 * @brief Core for developer tests.
 *
 * <b>IMPORTANT NOTICE:</b>
 * The whole "tests" package is designed for developer use only. It is @b NOT
 * performed in enclosed environment and may cause damage to the application's
 * setup if used inappropriately.
 *
 * Tests package imitates the Java's JUnit behavior.
 * The design is much simplier and the only supported outputs are the Log class
 * (which prints to stdout) and stdout itself. The test results are simple
 * boolean values: false if test failed at some point or true if succeeded.
 *
 * TestBase root class provides a static instance of Log class: testLog.
 * This instance should be used to display output.
 *
 * All tests should be executed through executeTest() method. This will print
 * standard information before and after each test.
 */
class TestCore : public TestBase
{
public:
	TestCore();

	/**
	 * @brief Clears success/fail counters.
	 */
	void clearCounters();

	/**
	 * Provides a standard encapsulation for test execution.
	 *
	 * @param test - An instance of TestUnitBase derivative class.
	 *     @b Important: this object will be deleted by this
	 *     method
	 */
	void executeTest(TestUnitBase *pTest);

	int numTests() const { return testsFailed + testsSucceeded; }
	int numTestsFailed() const { return testsFailed; }
	int numTestsSucceeded() const { return testsSucceeded; }


protected:
	int testsFailed;
	int testsSucceeded;
};

#endif
