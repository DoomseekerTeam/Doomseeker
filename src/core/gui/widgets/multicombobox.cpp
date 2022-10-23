//------------------------------------------------------------------------------
// multicombobox.cpp
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
#include "multicombobox.h"
#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QItemDelegate>
#include <QStylePainter>

// internal private editor
class MultiComboBoxEditor : public QCheckBox
{
public:
	MultiComboBoxEditor(QWidget *parent)
		: QCheckBox(parent) {}

protected:
	bool hitButton(const QPoint &pos) const override
	{
		// Omit QCheckBox::hitButton() check as it returns true only if
		// user actually clicked on the checkbox or on its title. In this
		// case, we want to detect hits on entire widget, which spans
		// as a whitespace area covering entire row in combobox' list.
		return QAbstractButton::hitButton(pos);
	}
};


// internal private delegate
class MultiComboBoxDelegate : public QItemDelegate
{
public:

	MultiComboBoxDelegate(QObject *parent)
		: QItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index) const override
	{
		//Get item data
		bool value = index.data(Qt::UserRole).toBool();
		QString text = index.data(Qt::DisplayRole).toString();

		// fill style options with item data
		const QStyle *style = QApplication::style();
		QStyleOptionButton opt;
		opt.state |= value ? QStyle::State_On : QStyle::State_Off;
		opt.state |= QStyle::State_Enabled;
		opt.text = text;
		opt.rect = option.rect;

		// draw item data as CheckBox
		style->drawControl(QStyle::CE_CheckBox, &opt, painter);
	}

	QWidget *createEditor(QWidget *parent,
		const QStyleOptionViewItem &option,
		const QModelIndex &index ) const override
	{
		Q_UNUSED(option)
		Q_UNUSED(index)
		return new MultiComboBoxEditor(parent);
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const override
	{
		auto myEditor = static_cast<QCheckBox *>(editor);
		myEditor->setText(index.data(Qt::DisplayRole).toString());
		myEditor->setChecked(index.data(Qt::UserRole).toBool());
	}

	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const override
	{
		//get the value from the editor (CheckBox)
		auto myEditor = static_cast<QCheckBox *>(editor);
		bool value = myEditor->isChecked();


		//set model data
		QMap<int, QVariant> data;
		data.insert(Qt::DisplayRole, myEditor->text());
		data.insert(Qt::UserRole, value);
		model->setItemData(index, data);
	}

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index ) const override
	{
		Q_UNUSED(index);
		editor->setGeometry(option.rect);
	}
};


//min-width:10em;
MultiComboBox::MultiComboBox(QWidget *widget )
	: QComboBox(widget)
{
	view()->setItemDelegate(new MultiComboBoxDelegate(this));
	// Enable editing on items view
	view()->setEditTriggers(QAbstractItemView::CurrentChanged);
	view()->viewport()->installEventFilter(this);
	view()->setAlternatingRowColors(true);
}


MultiComboBox::~MultiComboBox()
{
}

QString MultiComboBox::displayText() const
{
	return selectedItemTexts().join(", ");
}

bool MultiComboBox::eventFilter(QObject *object, QEvent *event)
{
	if (object == view()->viewport())
	{
		if (handleViewViewportEvent(event))
			return true;
	}
	return QComboBox::eventFilter(object, event);
}

bool MultiComboBox::handleViewViewportEvent(QEvent *event)
{
	switch (event->type())
	{
	default:
		break;

	case QEvent::Hide:
		// TODO: Have this react only if value actually changed.
		emit valueChanged();
		break;

	case QEvent::Show:
		if (count() >= 1)
		{
			// Without this, the first object on the list will be
			// impossible to edit unless other object is hovered
			// over first, or in case if there's only one object
			// on the list, the list won't be editable at all.
			view()->edit(model()->index(0, 0));
		}
		break;

	case QEvent::MouseButtonRelease:
		// Don't close items view after we release the mouse button
		// by simple eating MouseButtonRelease in viewport of items
		// view.
		return true;
	}
	return false;
}

void MultiComboBox::paintEvent(QPaintEvent *)
{
	QStylePainter painter(this);
	painter.setPen(palette().color(QPalette::Text));

	// draw the combobox frame, focusrect and selected etc.
	QStyleOptionComboBox opt;
	initStyleOption(&opt);

	// draw the icon and text
	opt.currentText = displayText();
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);
	painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

QStringList MultiComboBox::selectedItemTexts() const
{
	QStringList result;
	for (int i = 0; i < count(); ++i)
	{
		bool checked = itemData(i).toBool();
		if (checked)
			result << itemText(i);
	}
	return result;
}

void MultiComboBox::setSelectedTexts(const QStringList &texts)
{
	for (int i = 0; i < count(); ++i)
		setItemData(i, static_cast<bool>(texts.contains(itemText(i))));
	// Prompt widget repaint or the display text may not change.
	update();
}
