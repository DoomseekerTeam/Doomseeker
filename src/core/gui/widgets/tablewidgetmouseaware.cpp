//------------------------------------------------------------------------------
// tablewidgetmouseaware.cpp
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
#include "tablewidgetmouseaware.h"

#include <QModelIndex>
#include <QMouseEvent>

TableWidgetMouseAware::TableWidgetMouseAware(QWidget *parent)
	: QTableWidget(parent)
{
	bEmitSignalsEvenIfIndexIsInvalid = false;
}

void TableWidgetMouseAware::mouseReleaseEvent(QMouseEvent *event)
{
	QModelIndex index = indexAt(event->pos());
	switch (event->button())
	{
	case Qt::MiddleButton:
		if (bEmitSignalsEvenIfIndexIsInvalid || index.isValid())
			emit middleMouseClick(index, event->pos());
		break;

	case Qt::RightButton:
		if (bEmitSignalsEvenIfIndexIsInvalid || index.isValid())
			emit rightMouseClick(index, event->pos());
		break;

	default:
		QTableView::mouseReleaseEvent(event);
		break;
	}
}

void TableWidgetMouseAware::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		QTableView::mouseDoubleClickEvent(event);
	else
	{
		QModelIndex index = indexAt(event->pos());
		if (bEmitSignalsEvenIfIndexIsInvalid || index.isValid())
			emit leftMouseDoubleClicked(index, event->pos());
	}
}

QList<int> TableWidgetMouseAware::selectedRows() const
{
	QItemSelectionModel *pSelectionModel = this->selectionModel();

	QList<int> rowIndices;
	QModelIndexList rows = pSelectionModel->selectedRows();
	for (const QModelIndex &index : rows)
	{
		rowIndices << index.row();
	}

	return rowIndices;
}
