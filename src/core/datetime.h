//------------------------------------------------------------------------------
// datetime.h
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
#ifndef DOOMSEEKER_DATETIME_H
#define DOOMSEEKER_DATETIME_H

#include <QDateTime>
#include <QString>

/**
 * Date and time utility class.
 */
class DateTime
{
public:
	/**
	 * Format QDateTime in the ISO 8601 format with the timezone offset.
	 *
	 * Unlike the Qt's built-in QDateTime::toString(Qt::ISODate), this
	 * converter ensures that the timezone offset is in the returned string.
	 */
	static QString toISO8601(const QDateTime &);
};

#endif
