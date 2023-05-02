//------------------------------------------------------------------------------
// datetablewidgetitem.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "datetablewidgetitem.h"

DateTableWidgetItem::DateTableWidgetItem(const QDateTime &date,
	const QString &displayFormat)
{
	this->date = date;
	this->displayFormat = displayFormat;
	updateInternalData();
}

QString DateTableWidgetItem::displayedText() const
{
	return date.toString(displayFormat);
}

bool DateTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
	return data(Qt::EditRole).toDateTime() < other.data(Qt::EditRole).toDateTime();
}

void DateTableWidgetItem::setDisplayFormat(const QString &format)
{
	this->displayFormat = format;
	updateInternalData();
}

void DateTableWidgetItem::setDateTime(const QDateTime &date)
{
	this->date = date;
	updateInternalData();
}

void DateTableWidgetItem::updateInternalData()
{
	setData(Qt::DisplayRole, this->date.toString(this->displayFormat));
	setData(Qt::EditRole, this->date);
}
