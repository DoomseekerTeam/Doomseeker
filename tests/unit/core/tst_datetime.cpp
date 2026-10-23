//------------------------------------------------------------------------------
// tst_datetime.cpp
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
#include "datetime.h"

#include <QtTest>
#include <QTimeZone>

class TestDateTime : public QObject
{
	Q_OBJECT

private slots:
	void iso8601_data();
	void iso8601();
	void pathFriendlyUTCISO8601_data();
	void pathFriendlyUTCISO8601();
	void parsePathFriendlyUTCISO8601_data();
	void parsePathFriendlyUTCISO8601();
};

static QTimeZone warsawZone()
{
	QTimeZone zone("Europe/Warsaw");
	return zone.isValid() ? zone : QTimeZone("UTC+02:00");
}

void TestDateTime::iso8601_data()
{
	QTest::addColumn<QDateTime>("time");
	QTest::addColumn<QString>("expected");

	QTest::newRow("epoch") << QDateTime::fromMSecsSinceEpoch(0, QTimeZone(0))
		<< "1970-01-01T00:00:00Z";
	QTest::newRow("warsaw")
		<< QDateTime(QDate(2024, 7, 4), QTime(17, 0), warsawZone())
		<< "2024-07-04T17:00:00+02:00";
	QTest::newRow("negative-offset")
		<< QDateTime(QDate(2024, 7, 4), QTime(17, 0), QTimeZone(-3600))
		<< "2024-07-04T17:00:00-01:00";
}

void TestDateTime::iso8601()
{
	// Given: a datetime with an explicit timezone.
	QFETCH(QDateTime, time);
	QFETCH(QString, expected);
	if (!time.timeZone().isValid())
		QSKIP("Cannot resolve the timezone for this row");

	// When: formatting it as ISO 8601.
	const QString result = DateTime::toISO8601(time);

	// Then: the timezone offset is preserved.
	QCOMPARE(result, expected);
}

void TestDateTime::pathFriendlyUTCISO8601_data()
{
	QTest::addColumn<QDateTime>("time");
	QTest::addColumn<QString>("expected");

	QTest::newRow("epoch") << QDateTime::fromMSecsSinceEpoch(0, QTimeZone(0))
		<< "1970-01-01T000000Z";
	QTest::newRow("warsaw")
		<< QDateTime(QDate(2024, 7, 4), QTime(17, 0), warsawZone())
		<< "2024-07-04T150000Z";
	QTest::newRow("negative-offset")
		<< QDateTime(QDate(2024, 7, 4), QTime(12, 37, 21), QTimeZone(-3600))
		<< "2024-07-04T133721Z";
}

void TestDateTime::pathFriendlyUTCISO8601()
{
	// Given: a datetime with an explicit timezone.
	QFETCH(QDateTime, time);
	QFETCH(QString, expected);
	if (!time.timeZone().isValid())
		QSKIP("Cannot resolve the timezone for this row");

	// When: formatting it for a filename.
	const QString result = DateTime::toPathFriendlyUTCISO8601(time);

	// Then: the result is UTC and contains no time separators.
	QCOMPARE(result, expected);
}

void TestDateTime::parsePathFriendlyUTCISO8601_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QDateTime>("expected");

	QTest::newRow("epoch") << "1970-01-01T000000Z"
		<< QDateTime(QDate(1970, 1, 1), QTime(0, 0), QTimeZone(0));
	QTest::newRow("date") << "2024-07-04T133721Z"
		<< QDateTime(QDate(2024, 7, 4), QTime(13, 37, 21), QTimeZone(0));
	QTest::newRow("not-a-date") << "doomseeker" << QDateTime();
	QTest::newRow("missing-date-separators") << "20240704T133721Z" << QDateTime();
	QTest::newRow("time-separators") << "2024-07-04T13:37:21Z" << QDateTime();
}

void TestDateTime::parsePathFriendlyUTCISO8601()
{
	// Given: a valid or invalid filename-safe datetime string.
	QFETCH(QString, input);
	QFETCH(QDateTime, expected);

	// When: parsing it.
	const QDateTime result = DateTime::fromPathFriendlyUTCISO8601(input);

	// Then: valid input is read as UTC and malformed input is rejected.
	QCOMPARE(result.isValid(), expected.isValid());
	if (expected.isValid())
	{
		QCOMPARE(result, expected);
		QCOMPARE(result.timeSpec(), Qt::UTC);
	}
}

QTEST_GUILESS_MAIN(TestDateTime)

#include "tst_datetime.moc"
