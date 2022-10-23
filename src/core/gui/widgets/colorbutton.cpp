//------------------------------------------------------------------------------
// colorbutton.cpp
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
#include "colorbutton.h"
#include <QColorDialog>

#define UTF8_FULL_BLOCK "\xE2\x96\x88"

ColorButton::ColorButton(QWidget *parent)
	: QPushButton(QString::fromUtf8(UTF8_FULL_BLOCK UTF8_FULL_BLOCK), parent)
{
	connect( this, SIGNAL(clicked()), this, SLOT(thisClicked()));

	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
	color.setNamedColor("#ffffff");
	updateAppearance();
}

QString ColorButton::colorHtml() const
{
	return color.name();
}

unsigned ColorButton::colorUnsigned() const
{
	return color.rgb();
}

void ColorButton::setColor(unsigned colorValue)
{
	QColor newColor;
	newColor.setRgb(colorValue);
	updateColor(newColor);
}

void ColorButton::setColorHtml(const QString &colorHtml)
{
	QColor newColor;
	newColor.setNamedColor(colorHtml);
	updateColor(newColor);
}

QSize ColorButton::sizeHint() const
{
	QSize size = QPushButton::sizeHint();
	// 50 should be the smallest size before the Mac changes to ugly buttons
	size.setWidth(50);
	return size;
}

void ColorButton::thisClicked()
{
	QColor colorTmp = QColorDialog::getColor(QColor(color), parentWidget());

	if (colorTmp.isValid())
		updateColor(colorTmp);
}

void ColorButton::updateAppearance()
{
	static const QString COLOR_STYLE = "QPushButton { color : %1; }";

	QString styleSheet = COLOR_STYLE.arg(color.name());
	setStyleSheet(styleSheet);
}

void ColorButton::updateColor(const QColor &newColor)
{
	QColor oldColor = color;
	color = newColor;
	updateAppearance();

	emit colorUpdated(oldColor, color);
}
