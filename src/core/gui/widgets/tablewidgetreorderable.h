//------------------------------------------------------------------------------
// tablewidgetreorderable.h
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
// Copyright (C) 2019 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#pragma once
#include <QTableWidget>

/**
 * Allows to reorder rows by drag'n'drop.
 */
class TableWidgetReorderable : public QTableWidget
{
	Q_OBJECT

	friend class TableWidgetReorderableStyle;
public:
	TableWidgetReorderable(QWidget *parent = nullptr);

	void dropEvent(QDropEvent *event) override;

signals:
	void rowsReordered();

private:
	int determineDragRow(int y) const;
};
