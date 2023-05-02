//------------------------------------------------------------------------------
// tablewidgetmouseaware.h
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
#ifndef __TABLEWIDGETMOUSEAWARE_H__
#define __TABLEWIDGETMOUSEAWARE_H__

#include <QtContainerFwd>
#include <QTableWidget>

class TableWidgetMouseAware : public QTableWidget
{
	Q_OBJECT

public:
	TableWidgetMouseAware(QWidget *parent = nullptr);

	bool isEmitSignalsEvenIfIndexIsInvalidEnabled() const
	{
		return bEmitSignalsEvenIfIndexIsInvalid;
	}

	/**
	 * @brief Returns indices of currently selected rows.
	 */
	QList<int> selectedRows() const;
	void setEmitSignalsEvenIfIndexIsInvalid(bool b)
	{
		bEmitSignalsEvenIfIndexIsInvalid = b;
	}

private:
	/**
	 * @brief If set to true signals will be emitted without prior check
	 * to see if a table item was clicked.
	 */
	bool bEmitSignalsEvenIfIndexIsInvalid;

protected:
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

signals:
	void leftMouseDoubleClicked(const QModelIndex &index, const QPoint &cursorPosition);
	void middleMouseClick(const QModelIndex &index, const QPoint &cursorPosition);
	void rightMouseClick(const QModelIndex &index, const QPoint &cursorPosition);
};

#endif
