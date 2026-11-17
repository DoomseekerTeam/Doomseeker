//------------------------------------------------------------------------------
// tst_doomseekerpolicycopts.cpp
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
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "doomseekerpolicy.h"

#include "doomseeker_copts.h"
#include <QtTest>

class TestDoomseekerPolicyCopts : public QObject
{
	Q_OBJECT

private slots:
	void fromBuildOptions();
};

void TestDoomseekerPolicyCopts::fromBuildOptions()
{
	// Given: this variant's generated build-options header, shared with the
	// production factory translation unit.
	const bool serviceAccess = DOOMSEEKER_ALLOW_PROJECT_SERVICE_NETWORK_ACCESS;
	const bool applicationUpdates = DOOMSEEKER_WITH_AUTOUPDATES;
	const bool ip2cUpdates = DOOMSEEKER_WITH_IP2C_AUTOUPDATES;

	// When: the production factory interprets those build options.
	const auto policy = DoomseekerPolicy::fromBuildOptions();

	// Then: the blanket permission gates each service, manual IP2C permission
	// ignores the automatic option, and application permission is host-independent.
	QCOMPARE(policy.isAutoUpdateAllowed(), serviceAccess && applicationUpdates);
	QCOMPARE(policy.isIP2CUpdateAllowed(), serviceAccess);
	QCOMPARE(policy.isIP2CAutoUpdateAllowed(), serviceAccess && ip2cUpdates);
}

QTEST_GUILESS_MAIN(TestDoomseekerPolicyCopts)

#include "tst_doomseekerpolicycopts.moc"
