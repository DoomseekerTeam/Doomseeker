//------------------------------------------------------------------------------
// test_datetime.cpp
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
// Copyright (C) 2025 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include <QtTest/QtTest>

#include "datetime.h"

#include <QDate>
#include <QTime>
#include <QTimeZone>

class TestDateTime : public QObject
{
	Q_OBJECT
private slots:
	void iso8601_should_format_utc_epoch();
	void iso8601_should_format_a_named_timezone();
	void iso8601_should_format_an_offset_time();
	void toPathFriendlyUtcIso8601_should_format_utc_epoch();
	void toPathFriendlyUtcIso8601_should_format_a_named_timezone();
	void toPathFriendlyUtcIso8601_should_format_an_offset_time();
	void parsePathFriendlyUtcIso8601_should_parse_correctly();
};

void TestDateTime::iso8601_should_format_utc_epoch()
{
	QCOMPARE(
		DateTime::toISO8601(
			QDateTime::fromMSecsSinceEpoch(0, Qt::UTC)),
		QString("1970-01-01T00:00:00Z")
		);
}

void TestDateTime::iso8601_should_format_a_named_timezone()
{
	// Getting the timezone by its name may fail on its own,
	// but it's not what we're testing here, so try to recover
	// on a fall-back, and skip the test altogether if the
	// timezone cannot be defined.

	// Given -- a named timezone
	QTimeZone warsawZone("Europe/Warsaw");
	if (!warsawZone.isValid())
		warsawZone = QTimeZone("UTC+02:00");
	if (warsawZone.isValid())
	{
		// When
		QDateTime warsaw = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), warsawZone);
		// Then
		QCOMPARE(DateTime::toISO8601(warsaw), QString("2024-07-04T17:00:00+02:00"));
	}
	else
	{
		QSKIP("skipped named timezone because couldn't resolve the timezone");
	}
}

void TestDateTime::iso8601_should_format_an_offset_time()
{
	// Given -- an offset time
	QDateTime offsetM1 = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), QTimeZone(-1 * 60 * 60));
	// When/Then
	QCOMPARE(DateTime::toISO8601(offsetM1), QString("2024-07-04T17:00:00-01:00"));
}

void TestDateTime::toPathFriendlyUtcIso8601_should_format_utc_epoch()
{
	QCOMPARE(
		DateTime::toPathFriendlyUTCISO8601(
			QDateTime::fromMSecsSinceEpoch(0, Qt::UTC)),
		QString("1970-01-01T000000Z")
		);
}

void TestDateTime::toPathFriendlyUtcIso8601_should_format_a_named_timezone()
{
	// Given -- a named timezone
	QTimeZone warsawZone("Europe/Warsaw");
	if (!warsawZone.isValid())
		warsawZone = QTimeZone("UTC+02:00");
	if (warsawZone.isValid())
	{
		// When
		QDateTime warsaw = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), warsawZone);
		// Then
		QCOMPARE(DateTime::toPathFriendlyUTCISO8601(warsaw), QString("2024-07-04T150000Z"));
	}
	else
	{
		QSKIP("skipped named timezone because couldn't resolve the timezone");
	}
}

void TestDateTime::toPathFriendlyUtcIso8601_should_format_an_offset_time()
{
	// Given -- an offset time
	QDateTime offsetM1 = QDateTime(QDate(2024, 07, 04), QTime(12, 37, 21), QTimeZone(-1 * 60 * 60));
	// When/Then
	QCOMPARE(DateTime::toPathFriendlyUTCISO8601(offsetM1), QString("2024-07-04T133721Z"));
}

void TestDateTime::parsePathFriendlyUtcIso8601_should_parse_correctly()
{
	QCOMPARE(
		DateTime::fromPathFriendlyUTCISO8601("1970-01-01T000000Z"),
		QDateTime(QDate(1970, 01, 01), QTime(0, 0, 0), Qt::UTC)
		);

	QCOMPARE(
		DateTime::fromPathFriendlyUTCISO8601("2024-07-04T133721Z"),
		QDateTime(QDate(2024, 07, 04), QTime(13, 37, 21), Qt::UTC)
		);

	QCOMPARE(DateTime::fromPathFriendlyUTCISO8601("doomseeker").isValid(), false);
	QCOMPARE(DateTime::fromPathFriendlyUTCISO8601("20240704T133721Z").isValid(), false);
	QCOMPARE(DateTime::fromPathFriendlyUTCISO8601("2024-07-04T13:37:21Z").isValid(), false);
}

QTEST_MAIN(TestDateTime)
#include "test_datetime.moc"
