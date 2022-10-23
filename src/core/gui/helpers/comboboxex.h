//------------------------------------------------------------------------------
// comboboxex.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id3185FE61_75A1_4755_AF403E1F7D09E14B
#define id3185FE61_75A1_4755_AF403E1F7D09E14B

#include <QComboBox>
#include <QStringList>

/**
 * @brief Convenience methods for combo box.
 */
class ComboBoxEx
{
public:
	ComboBoxEx(QComboBox &comboBox);

	QStringList allItems() const;
	/**
	 * @brief Removes currently selected item.
	 *
	 * @return true if item was in combobox data source.
	 */
	bool removeCurrentItem();
	/**
	 * @brief Removes item that matches specified one.
	 *
	 * @return true if item was in combobox data source.
	 */
	bool removeItem(const QString &item);
	void setCurrentOrAddNewAndSelect(const QString &item);
	void setItemsSorted(QStringList items);

private:
	QComboBox &box;

	static bool caseInsensitiveLessThan(const QString &s1, const QString &s2);
};

#endif
