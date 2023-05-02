//------------------------------------------------------------------------------
// multicombobox.h
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
#ifndef id61914372_2F5E_42A3_9CCC3ADF106B6FF2
#define id61914372_2F5E_42A3_9CCC3ADF106B6FF2

#include <QComboBox>

class QEvent;

/**
 * Based on:
 * http://stackoverflow.com/questions/6505627/how-to-make-qcombobox-as-multiselect-in-qt
 *
 * This probably shouldn't be a subclass of QComboBox because there are
 * plenty QComboBox native methods that won't work here. Current item index,
 * and current item text have no meaning here. Instead, use getters defined
 * directly in this class.
 */
class MultiComboBox : public QComboBox
{
	Q_OBJECT

public:
	MultiComboBox(QWidget *widget = nullptr);
	~MultiComboBox() override;
	QString displayText() const;
	bool eventFilter(QObject *object, QEvent *event) override;
	void paintEvent(QPaintEvent *) override;
	QStringList selectedItemTexts() const;
	void setSelectedTexts(const QStringList &texts);

signals:
	void valueChanged();

private:
	bool handleViewViewportEvent(QEvent *event);
};

#endif
