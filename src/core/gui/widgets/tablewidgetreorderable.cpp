//------------------------------------------------------------------------------
// tablewidgetreorderable.cpp
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
#include "tablewidgetreorderable.h"

#include <QCursor>
#include <QDropEvent>
#include <QHeaderView>
#include <QPainter>
#include <QProxyStyle>

class TableWidgetReorderableStyle : public QProxyStyle
{
public:
	using QProxyStyle::QProxyStyle;

	void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
};

void TableWidgetReorderableStyle::drawPrimitive(QStyle::PrimitiveElement element,
	const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if (element == PE_IndicatorItemViewItemDrop && !option->rect.isNull() && widget)
	{
		auto table = static_cast<const TableWidgetReorderable*>(widget);
		QPoint cursorPos = QCursor::pos();
		QPoint tablePos = table->mapFromGlobal(cursorPos);
		tablePos.setX(tablePos.x() - table->verticalHeader()->width());
		tablePos.setY(tablePos.y() - table->horizontalHeader()->height());
		int targetRow = table->determineDragRow(tablePos.y());
		QStyleOption newOption = *option;
		if (targetRow < table->rowCount())
		{
			newOption.rect.setTop(table->rowViewportPosition(targetRow));
		}
		else
		{
			int lastRow = table->rowCount() - 1;
			if (lastRow < 0)
				return;
			int bottom = table->rowViewportPosition(lastRow) + table->rowHeight(lastRow);
			newOption.rect.setTop(bottom);
		}
		newOption.rect.setHeight(0);
		newOption.rect.setLeft(0);
		if (widget)
			newOption.rect.setRight(widget->width());
		QProxyStyle::drawPrimitive(element, &newOption, painter, widget);
	}
	else
	{
		QProxyStyle::drawPrimitive(element, option, painter, widget);
	}
}

TableWidgetReorderable::TableWidgetReorderable(QWidget *parent)
	: QTableWidget(parent)
{
	this->setStyle(new TableWidgetReorderableStyle());
}

void TableWidgetReorderable::dropEvent(QDropEvent *event)
{
	if (event->source() != this)
	{
		event->ignore();
		return;
	}
	int newRow = determineDragRow(event->pos().y());
	QList<QTableWidgetItem*> selectedItems = this->selectedItems();
	QSet<int> uniqueRows;
	for (QTableWidgetItem *item : selectedItems)
		uniqueRows.insert(item->row());
	for (int n = 0; n < uniqueRows.count(); ++n)
		this->insertRow(newRow);

	int currentOldRow = -1;
	int currentNewRow = newRow - 1;
	QList<int> deleteRows;
	for (QTableWidgetItem *selectedItem : selectedItems)
	{
		int column = selectedItem->column();
		if (selectedItem->row() != currentOldRow)
		{
			currentOldRow = selectedItem->row();
			deleteRows.append(currentOldRow);
			currentNewRow++;
		}
		QWidget *cellWidget = this->cellWidget(currentOldRow, column);
		this->takeItem(currentOldRow, column);
		this->setItem(currentNewRow, column, selectedItem);
		this->setCellWidget(currentNewRow, column, cellWidget);
	}

	std::sort(deleteRows.begin(), deleteRows.end());
	for (int i = deleteRows.count() - 1; i >= 0; --i)
		this->removeRow(deleteRows[i]);
	event->ignore();
	emit rowsReordered();
}

int TableWidgetReorderable::determineDragRow(int y) const
{
	/*
	  Split the row height in half. If 'y' is in the upper half,
	  the row at the 'y' is targeted. If 'y' is in the lower half,
	  the row below is targeted. If we can't find any row, assume
	  we're targeting the end of the table.
	*/
	int directRow = this->rowAt(y);
	if (directRow < 0)
		return this->rowCount();
	int rowPosition = this->rowViewportPosition(directRow);
	int rowHeight = this->rowHeight(directRow);
	int rowLocalY = y - rowPosition;
	return (rowLocalY < rowHeight / 2) ? directRow : directRow + 1;
}
