//------------------------------------------------------------------------------
// commongui.h
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
#ifndef DOOMSEEKER_GUI_COMMONGUI_H
#define DOOMSEEKER_GUI_COMMONGUI_H

#include <QString>

class QAbstractItemView;
class QComboBox;
class QListView;
class QStringList;
class QTableWidget;

class CommonGUI
{
public:
	/**
	 * @brief Calls QInputDialog::getText().
	 */
	static QString askString(const QString &title, const QString &label,
		bool *ok = nullptr, const QString &defaultString = "");

	/**
	 * Reads items from a QListView with QStandardItemModel and pulls the
	 * checked state and converts to a QList<bool>.
	 */
	static QList<bool> listViewStandardItemsToBoolList(QListView *listview);

	/**
	 * @brief Reads items from a QListView that uses QStandardItemModel
	 * and puts texts of these items into a list of strings.
	 * @param listview - QListView that uses QStandardItemModel.
	 */
	static QStringList listViewStandardItemsToStringList(QListView *listview);

	static void removeSelectedRowsFromQTableWidget(QTableWidget *table);

	/**
	 * @brief Removes all selected rows from a QAbstractItemView.
	 *
	 * @param view
	 *      QAbstractItemView from which items will be removed.
	 * @param bSelectNextItem
	 *      If true then next lowest item on the list is selected.
	 *      Default: false.
	 */
	static void removeSelectedRowsFromStandardItemView(
		QAbstractItemView *view, bool bSelectNextItem = false);

	/**
	 * @brief Puts a list of strings into a QListView that uses
	 *        QStandardItemModel.
	 * @param targetListview
	 *     QListView that uses QStandardItemModel. This list view will
	 *     be filled with data.
	 * @param stringList
	 *     Source data.
	 */
	static void stringListToStandardItemsListView(QListView *targetListview,
		const QStringList &stringList);
};

#endif
