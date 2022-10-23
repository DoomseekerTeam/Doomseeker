//------------------------------------------------------------------------------
// fontbutton.cpp
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
#include "fontbutton.h"
#include <QFontDialog>

FontButton::FontButton(QWidget *parent)
	: QPushButton(parent)
{
	connect(this, SIGNAL(clicked()), SLOT(thisClicked()));

	this->updateAppearance();
}

void FontButton::setSelectedFont(const QFont &font)
{
	this->currentFont = font;
	this->updateAppearance();
}

void FontButton::thisClicked()
{
	bool bOk = false;
	QFont fontTmp = QFontDialog::getFont(&bOk, this->currentFont, this->parentWidget());

	if (bOk)
		this->updateFont(fontTmp);
}

void FontButton::updateAppearance()
{
	QString text = QString("%1, %2").arg(this->currentFont.family()).arg(this->currentFont.pointSize());
	QFont textFont = this->currentFont;

	this->setFont(textFont);
	this->setText(text);
}

void FontButton::updateFont(const QFont &newFont)
{
	QFont oldFont = this->currentFont;
	this->currentFont = newFont;
	updateAppearance();

	emit fontUpdated(oldFont, this->currentFont);
}
