//------------------------------------------------------------------------------
// comboboxex.cpp
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
#include "comboboxex.h"

#include <QLineEdit>

ComboBoxEx::ComboBoxEx(QComboBox &comboBox)
	: box(comboBox)
{
}

QStringList ComboBoxEx::allItems() const
{
	QStringList items;
	for (int i = 0; i < box.count(); ++i)
		items << box.itemText(i);
	return items;
}

bool ComboBoxEx::caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
	return s1.toLower() < s2.toLower();
}

bool ComboBoxEx::removeCurrentItem()
{
	return removeItem(box.currentText());
}

bool ComboBoxEx::removeItem(const QString &item)
{
	int idx = box.findText(item);
	if (idx >= 0)
	{
		// Simply removing current index won't give proper results
		// if user edits the contents of the combo box.
		box.removeItem(idx);
	}
	return idx >= 0;
}

void ComboBoxEx::setCurrentOrAddNewAndSelect(const QString &item)
{
	int idx = box.findText(item);
	if (idx >= 0)
		box.setCurrentIndex(idx);
	else
	{
		box.insertItem(0, item);
		box.setCurrentIndex(0);
	}
	box.lineEdit()->selectAll();
}

void ComboBoxEx::setItemsSorted(QStringList items)
{
	std::sort(items.begin(), items.end(), caseInsensitiveLessThan);
	box.clear();
	for (const QString &item : items)
	{
		if (box.findText(item) < 0)
			box.addItem(item);
	}
}
