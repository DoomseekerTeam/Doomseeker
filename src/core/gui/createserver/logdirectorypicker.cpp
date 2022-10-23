//------------------------------------------------------------------------------
// logdirectorypicker.cpp
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------
#include "logdirectorypicker.h"
#include "ui_logdirectorypicker.h"

#include "configuration/doomseekerconfig.h"

#include <QDir>
#include <QFileDialog>

DClass<LogDirectoryPicker> : public Ui::LogDirectoryPicker
{
public:
	bool loggingEnabled;
	QString dialogDir;
};

DPointeredNoCopy(LogDirectoryPicker)

LogDirectoryPicker::LogDirectoryPicker(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);

	d->loggingEnabled = gConfig.doomseeker.bLogCreatedServer;
	d->dialogDir = gConfig.doomseeker.previousCreateServerLogDir;

	d->cbLoggingEnabled->setChecked(d->loggingEnabled);
	setPathAndUpdate(d->dialogDir);
}

LogDirectoryPicker::~LogDirectoryPicker()
{
}

void LogDirectoryPicker::cbLoggingEnabledChecked(bool isChecked)
{
	d->loggingEnabled = isChecked;
	gConfig.doomseeker.bLogCreatedServer = d->loggingEnabled;
}

void LogDirectoryPicker::textEdited()
{
	d->dialogDir = d->pathTextBox->text();
	updateLoggingStatus(d->dialogDir);
}

void LogDirectoryPicker::browse()
{
	QString strPath = QFileDialog::getExistingDirectory(this,
		tr("Doomseeker - select Log path"), d->dialogDir);
	if (!strPath.isEmpty())
		setPathAndUpdate(QDir(strPath).absolutePath());
}

void LogDirectoryPicker::updateLoggingStatus(const QString &path)
{
	if (!path.isEmpty())
	{
		bool exists = QDir(path).exists();
		d->pathNotFound->setVisible(!exists);
		d->cbLoggingEnabled->setEnabled(exists);
		if (!exists)
			setLoggingEnabled(false);
	}
	else
	{
		d->pathNotFound->hide();
		d->cbLoggingEnabled->setEnabled(true);
	}
	gConfig.doomseeker.previousCreateServerLogDir = d->dialogDir;
}

void LogDirectoryPicker::setPathAndUpdate(const QString &path)
{
	d->dialogDir = path;
	d->pathTextBox->setText(path);
	updateLoggingStatus(path);
}

void LogDirectoryPicker::setLoggingEnabled(const bool &enabled)
{
	d->loggingEnabled = enabled;
	gConfig.doomseeker.bLogCreatedServer = d->loggingEnabled;
	d->cbLoggingEnabled->setChecked(d->loggingEnabled);
}

const bool &LogDirectoryPicker::isLoggingEnabled() const
{
	return d->loggingEnabled;
}

const QString &LogDirectoryPicker::currentPath() const
{
	return d->dialogDir;
}

const QString LogDirectoryPicker::validatedCurrentPath() const
{
	if (!d->loggingEnabled || !QDir(d->dialogDir).exists())
		return QString();
	else
		return QDir(d->dialogDir).absolutePath();
}