//------------------------------------------------------------------------------
// wadseekershow.cpp
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
#include "wadseekershow.h"

#include "application.h"
#include "configuration/doomseekerconfig.h"
#include "gui/mainwindow.h"
#include "gui/wadseekerinterface.h"
#include "serverapi/server.h"
#include "templatedpathresolver.h"
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QMessageBox>

DClass<WadseekerShow>
{
};

DPointered(WadseekerShow)

WadseekerShow *WadseekerShow::staticInstance = nullptr;

WadseekerShow::WadseekerShow()
{
}

bool WadseekerShow::checkWadseekerValidity(QWidget *parent)
{
	if (WadseekerInterface::isInstantiated())
	{
		QMessageBox::warning(parent, tr("Wadseeker cannot be launched"),
			tr("Another instance of Wadseeker is already running."), QMessageBox::Ok);
		return false;
	}

	QString targetDirPath = gDoomseekerTemplatedPathResolver().resolve(gConfig.wadseeker.targetDirectory);
	QDir targetDir(targetDirPath);
	QFileInfo targetDirFileInfo(targetDirPath);

	if (targetDirPath.isEmpty() || !targetDir.exists() || !targetDirFileInfo.isWritable())
	{
		QString error = tr("Wadseeker will not work correctly:\n\n"
			"The target directory is either not configured, is invalid or cannot be written to.\n\n"
			"Please review your Configuration and/or refer to the online help available from "
			"the Help menu.");
		QMessageBox::warning(parent, tr("Wadseeker cannot be launched"),
			error, QMessageBox::Ok);
		return false;
	}

	return true;
}

WadseekerShow *WadseekerShow::instance()
{
	if (staticInstance == nullptr)
		staticInstance = new WadseekerShow();
	return staticInstance;
}
