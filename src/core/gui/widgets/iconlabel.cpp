//------------------------------------------------------------------------------
// iconlabel.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "iconlabel.h"

IconLabel::IconLabel(QWidget *pParent)
	: QWidget(pParent)
{
	lblIcon = new QLabel(this);
	lblIcon->setPixmap(QPixmap(":/icons/help"));
	lblIcon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	lblText = new QLabel(this);
	lblText->setText("TextLabel");

	pLayout = new QHBoxLayout(this);
	pLayout->addWidget(lblIcon);
	pLayout->addWidget(lblText);
	pLayout->setContentsMargins(0, 0, 0, 0);

	this->setLayout(pLayout);
	this->setContentsMargins(0, 0, 0, 0);
}

QString IconLabel::text() const
{
	return lblText->text();
}

void IconLabel::setPixmap(const QPixmap &pixmap)
{
	lblIcon->setPixmap(pixmap);
}

void IconLabel::setText(const QString &str)
{
	lblText->setText(str);
}

void IconLabel::setWordWrap(bool wrap)
{
	lblText->setWordWrap(wrap);
}
