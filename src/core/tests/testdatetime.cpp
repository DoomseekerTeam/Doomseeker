//------------------------------------------------------------------------------
// testdatetime.cpp
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
#include "testdatetime.h"

#include "datetime.h"
#include "tests/asserts.h"

#include <QDate>
#include <QTime>
#include <QTimeZone>

bool TestDateTimeISO8601::executeTest()
{
	T_ASSERT_EQUAL("1970-01-01T00:00:00Z", DateTime::toISO8601(
		QDateTime::fromMSecsSinceEpoch(0, Qt::UTC)));

	// Getting the timezone by its name may fail on its own,
	// but it's not what we're testing here, so try to recover
	// on a fall-back, and skip the test if altogether if the
	// timezone cannot be defined.
	QTimeZone warsawZone("Europe/Warsaw");
	if (!warsawZone.isValid())
		warsawZone = QTimeZone("UTC+02:00");
	if (warsawZone.isValid())
	{
		QDateTime warsaw = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), warsawZone);
		T_ASSERT_EQUAL("2024-07-04T17:00:00+02:00", DateTime::toISO8601(warsaw));
	}
	else
	{
		testLog << "skipped Europe/Warsaw test because couldn't resolve the timezone";
	}

	QDateTime offsetM1 = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), QTimeZone(-1 * 60 * 60));
	T_ASSERT_EQUAL("2024-07-04T17:00:00-01:00", DateTime::toISO8601(offsetM1));

	return true;
}

bool TestDateTimeToPathFriendlyUTCISO8601::executeTest()
{
	T_ASSERT_EQUAL("1970-01-01T000000Z", DateTime::toPathFriendlyUTCISO8601(
		QDateTime::fromMSecsSinceEpoch(0, Qt::UTC)));

	QTimeZone warsawZone("Europe/Warsaw");
	if (!warsawZone.isValid())
		warsawZone = QTimeZone("UTC+02:00");
	if (warsawZone.isValid())
	{
		QDateTime warsaw = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), warsawZone);
		T_ASSERT_EQUAL("2024-07-04T150000Z", DateTime::toPathFriendlyUTCISO8601(warsaw));
	}
	else
	{
		testLog << "skipped Europe/Warsaw test because couldn't resolve the timezone";
	}

	QDateTime offsetM1 = QDateTime(QDate(2024, 07, 04), QTime(12, 37, 21), QTimeZone(-1 * 60 * 60));
	T_ASSERT_EQUAL("2024-07-04T133721Z", DateTime::toPathFriendlyUTCISO8601(offsetM1));

	return true;
}

bool TestDateTimeParsePathFriendlyUTCISO8601::executeTest()
{
	T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1970, 01, 01), QTime(0, 0, 0), Qt::UTC),
		DateTime::fromPathFriendlyUTCISO8601("1970-01-01T000000Z"));

	T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(2024, 07, 04), QTime(13, 37, 21), Qt::UTC),
		DateTime::fromPathFriendlyUTCISO8601("2024-07-04T133721Z"));

	T_ASSERT_FALSE(DateTime::fromPathFriendlyUTCISO8601("doomseeker").isValid());
	T_ASSERT_FALSE(DateTime::fromPathFriendlyUTCISO8601("20240704T133721Z").isValid());
	T_ASSERT_FALSE(DateTime::fromPathFriendlyUTCISO8601("2024-07-04T13:37:21Z").isValid());

	return true;
}
