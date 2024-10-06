//------------------------------------------------------------------------------
// iwadpicker.cpp
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
#include "iwadpicker.h"
#include "ui_iwadpicker.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "strings.hpp"
#include <QFileDialog>
#include <QFileInfo>
#include <QStyle>

DClass<IwadPicker> : public Ui::IwadPicker
{
};

DPointered(IwadPicker)

IwadPicker::IwadPicker(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
	d->btnBrowse->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	loadIwads();
}

IwadPicker::~IwadPicker()
{
}

void IwadPicker::addIwad(const QString &path)
{
	if (path.trimmed().isEmpty())
		return;

	for (int i = 0; i < d->cboIwad->count(); ++i)
	{
		if (d->cboIwad->itemText(i).compare(path) == 0)
		{
			d->cboIwad->setCurrentIndex(i);
			return;
		}
	}

	d->cboIwad->addItem(Strings::normalizePath(path));
	d->cboIwad->setCurrentIndex(d->cboIwad->count() - 1);
}

void IwadPicker::browse()
{
	QString dialogDir = gConfig.doomseeker.previousCreateServerWadDir;
	QString path = QFileDialog::getOpenFileName(this, tr("Doomseeker - select IWAD"), dialogDir);

	if (!path.isEmpty())
	{
		path = gDefaultDataPaths->portablizePath(path);
		gConfig.doomseeker.previousCreateServerWadDir = path;

		addIwad(path);
	}
}

QString IwadPicker::currentIwad() const
{
	return d->cboIwad->currentText();
}

void IwadPicker::loadIwads()
{
	const QString iwads[] = {
		"doom.wad", "doom1.wad", "doom2.wad",
		"tnt.wad", "plutonia.wad", "heretic.wad",
		"hexen.wad", "freedoom.wad", "freedm.wad",
		"freedoom1.wad", "freedoom2.wad", "strife1.wad", ""
	};

	QString currentSelection = d->cboIwad->currentText();
	d->cboIwad->clear();
	for (int i = 0; !iwads[i].isEmpty(); ++i)
	{
		WadPathFinder wadPathFinder = WadPathFinder(PathFinder());
		wadPathFinder.setAllowAliases(false);
		QString path = wadPathFinder.find(iwads[i]).path();
		if (!path.isEmpty())
			d->cboIwad->addItem(gDefaultDataPaths->portablizePath(path));
	}
	if (!currentSelection.isEmpty())
	{
		int selectionIdx = d->cboIwad->findText(currentSelection);
		if (selectionIdx >= 0)
			d->cboIwad->setCurrentIndex(selectionIdx);
		else
			d->cboIwad->setEditText(currentSelection);
	}
}

void IwadPicker::setIwadByName(const QString &iwad)
{
	for (int i = 0; i < d->cboIwad->count(); ++i)
	{
		QFileInfo fi(d->cboIwad->itemText(i));
		if (fi.fileName().compare(iwad, Qt::CaseInsensitive) == 0)
		{
			d->cboIwad->setCurrentIndex(i);
			return;
		}
	}
	// If IWAD with given name isn't present on the list try to find it anyway.
	PathFinder pathFinder;
	QString path = pathFinder.findFile(iwad);
	if (!path.isEmpty())
	{
		d->cboIwad->addItem(gDefaultDataPaths->portablizePath(path));
		d->cboIwad->setCurrentIndex(d->cboIwad->count() - 1);
	}
}

void IwadPicker::setIwadByPath(const QString &iwadPath)
{
	for (int i = 0; i < d->cboIwad->count(); ++i)
	{
		if (d->cboIwad->itemText(i).compare(iwadPath, Qt::CaseInsensitive) == 0)
		{
			d->cboIwad->setCurrentIndex(i);
			return;
		}
	}
	d->cboIwad->addItem(iwadPath);
	d->cboIwad->setCurrentIndex(d->cboIwad->count() - 1);
}
