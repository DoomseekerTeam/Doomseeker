//------------------------------------------------------------------------------
// cfgwadseekeridgames.cpp
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
#include "cfgwadseekeridgames.h"
#include "configuration/doomseekerconfig.h"
#include "ui_cfgwadseekeridgames.h"
#include "wadseeker/wadseeker.h"
#include <QCompleter>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>

DClass<CFGWadseekerIdgames> : public Ui::CFGWadseekerIdgames
{
};

DPointered(CFGWadseekerIdgames)

CFGWadseekerIdgames::CFGWadseekerIdgames(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);

	connect(d->btnIdgamesURLDefault, SIGNAL(clicked()), this, SLOT(btnIdgamesURLDefaultClicked()));
	d->frameWithContent->setEnabled(d->cbUseIdgames->isChecked());
}

CFGWadseekerIdgames::~CFGWadseekerIdgames()
{
}

void CFGWadseekerIdgames::btnIdgamesURLDefaultClicked()
{
	d->leIdgamesURL->setText(Wadseeker::defaultIdgamesUrl());
}

void CFGWadseekerIdgames::readSettings()
{
	d->cbUseIdgames->setChecked(gConfig.wadseeker.bSearchInIdgames);
	d->cbUseWadArchive->setChecked(gConfig.wadseeker.bSearchInWadArchive);
	d->leIdgamesURL->setText(gConfig.wadseeker.idgamesURL);
}

void CFGWadseekerIdgames::saveSettings()
{
	gConfig.wadseeker.bSearchInIdgames = d->cbUseIdgames->isChecked();
	gConfig.wadseeker.bSearchInWadArchive = d->cbUseWadArchive->isChecked();
	gConfig.wadseeker.idgamesURL = d->leIdgamesURL->text();
}
