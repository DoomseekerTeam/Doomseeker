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
#include "datetime.h"
#include "testdatetime.h"

#include <QDate>
#include <QTime>
#include <QTimeZone>

#define T_ASSERT_EQUAL(expected, result) { \
	auto _expected = (expected); \
	auto _result = (result); \
	if (!(_expected == _result)) \
	{ \
		testLog << QString("expected: %1, got %2").arg(_expected).arg(_result); \
		return false; \
	}}

bool TestDateTimeISO8601::executeTest()
{
	T_ASSERT_EQUAL("1970-01-01T00:00:00Z", DateTime::toISO8601(
		QDateTime::fromMSecsSinceEpoch(0, Qt::UTC)));

	QDateTime warsaw = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), QTimeZone("Europe/Warsaw"));
	T_ASSERT_EQUAL("2024-07-04T17:00:00+02:00", DateTime::toISO8601(warsaw));

	QDateTime offsetM1 = QDateTime(QDate(2024, 07, 04), QTime(17, 00, 00), QTimeZone(-1 * 60 * 60));
	T_ASSERT_EQUAL("2024-07-04T17:00:00-01:00", DateTime::toISO8601(offsetM1));

	return true;
}
