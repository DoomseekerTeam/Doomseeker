//------------------------------------------------------------------------------
// commongui.cpp
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
#include "commongui.h"
#include <QComboBox>
#include <QInputDialog>
#include <QListView>
#include <QStandardItemModel>
#include <QStringList>
#include <QTableWidget>

QString CommonGUI::askString(const QString &title, const QString &label,
	bool *ok, const QString &defaultString)
{
	return QInputDialog::getText(nullptr, title, label, QLineEdit::Normal,
		defaultString, ok);
}

QList<bool> CommonGUI::listViewStandardItemsToBoolList(QListView *listview)
{
	QList<bool> list;
	auto model = static_cast<QStandardItemModel *>(
		listview->model());
	for (int i = 0; i < model->rowCount(); ++i)
		list << (model->item(i)->checkState() == Qt::Checked);

	return list;
}

QStringList CommonGUI::listViewStandardItemsToStringList(QListView *listview)
{
	QStringList list;
	auto model = static_cast<QStandardItemModel *>(
		listview->model());
	for (int i = 0; i < model->rowCount(); ++i)
		list << model->item(i)->text();

	return list;
}

void CommonGUI::removeSelectedRowsFromQTableWidget(QTableWidget *table)
{
	// Rows can't be just deleted with items from selectedItems()
	// because the program will crash. This solution is so stupid
	// that there must be another one, but nobody knows...
	QMap<int, QTableWidgetItem *> uniqueRowsItems;
	for (QTableWidgetItem *item : table->selectedItems())
	{
		uniqueRowsItems.insert(item->row(), item);
	}
	for (QTableWidgetItem *item : uniqueRowsItems.values())
	{
		int row = table->row(item);
		if (row >= 0)
			table->removeRow(row);
	}
}

void CommonGUI::removeSelectedRowsFromStandardItemView(QAbstractItemView *view,
	bool bSelectNextItem)
{
	QItemSelectionModel *selModel = view->selectionModel();
	QModelIndexList indexList = selModel->selectedIndexes();
	selModel->clear();

	auto model = static_cast<QStandardItemModel *>(view->model());
	QList<QStandardItem *> itemList;
	int lowestRemovedRow = 0;
	for (int i = 0; i < indexList.count(); ++i)
	{
		const QModelIndex &index = indexList[i];
		itemList << model->itemFromIndex(index);
		if (index.row() > lowestRemovedRow)
			lowestRemovedRow = index.row();
	}

	for (int i = 0; i < itemList.count(); ++i)
	{
		QModelIndex index = model->indexFromItem(itemList[i]);
		model->removeRow(index.row());
	}

	if (bSelectNextItem && !indexList.isEmpty())
	{
		int selectRowIdx = lowestRemovedRow;
		selectRowIdx -= indexList.size();

		if (selectRowIdx + 1 < model->rowCount())
			++selectRowIdx;

		if (selectRowIdx >= 0)
		{
			QModelIndex newIdx = model->index(selectRowIdx, 0);
			selModel->select(newIdx, QItemSelectionModel::ClearAndSelect);
		}
	}
}

void CommonGUI::stringListToStandardItemsListView(QListView *targetListview,
	const QStringList &stringList)
{
	auto model = static_cast<QStandardItemModel *>(
		targetListview->model());
	model->removeRows(0, model->rowCount());

	for (const QString &str : stringList)
	{
		auto pItem = new QStandardItem();
		pItem->setText(str);
		model->appendRow(pItem);
	}
}
