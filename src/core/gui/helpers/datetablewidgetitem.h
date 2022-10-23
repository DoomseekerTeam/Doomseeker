//------------------------------------------------------------------------------
// datetablewidgetitem.h
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
#ifndef id7D370D9A_DF04_4500_AB6AFF4589B86AF5
#define id7D370D9A_DF04_4500_AB6AFF4589B86AF5

#include <QDateTime>
#include <QTableWidgetItem>

class DateTableWidgetItem : public QTableWidgetItem
{
public:
	DateTableWidgetItem(const QDateTime &date,
		const QString &displayFormat = "yyyy-MM-dd hh:mm:ss");

	bool operator<(const QTableWidgetItem &other) const override;

	QString displayedText() const;
	void setDateTime(const QDateTime &date);
	void setDisplayFormat(const QString &format);

private:
	QDateTime date;
	QString displayFormat;

	void updateInternalData();
};

#endif
