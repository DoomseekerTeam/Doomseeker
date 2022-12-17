//------------------------------------------------------------------------------
// exefile.cpp
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
#include "exefile.h"

#include "configuration/doomseekerconfig.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "message.h"
#include "plugins/engineplugin.h"
#include "templatedpathresolver.h"

#include <QFileInfo>
#include <QString>

DClass<ExeFile>
{
public:
	QString configKey;
	QString exeTypeName;
	QString programName;
};

DPointered(ExeFile)

ExeFile::ExeFile()
{
}

ExeFile::~ExeFile()
{
}

const QString &ExeFile::configKey() const
{
	return d->configKey;
}

const QString &ExeFile::exeTypeName() const
{
	return d->exeTypeName;
}

Message ExeFile::install(QWidget *parent)
{
	Q_UNUSED(parent);
	return Message();
}

QString ExeFile::pathToExe(Message &message)
{
	IniSection config = gConfig.iniSectionForPlugin(programName());
	IniVariable setting = config[configKey()];

	message = Message();
	QString path = setting.value().toString();
	QString error = "";
	if (path.trimmed().isEmpty())
	{
		error = tr("The %1 file is not set for %2.").arg(exeTypeName()).arg(programName());
		message = Message::customError(error);
		return QString();
	}

	QFileInfo fi(gDoomseekerTemplatedPathResolver().resolve(path));
	if (!fi.exists())
	{
		error = tr("The %1 file for %2 doesn't exist:\n%3")
			.arg(exeTypeName())
			.arg(programName())
			.arg(fi.absoluteFilePath());
		message = Message::customError(error);
		return QString();
	}

	if (fi.isDir() && !fi.isBundle())
	{
		error = tr("The path to %1 file for %2 is a directory:\n%3")
			.arg(exeTypeName())
			.arg(programName())
			.arg(fi.absoluteFilePath());
		message = Message::customError(error);
		return QString();
	}

	return fi.absoluteFilePath();
}

const QString &ExeFile::programName() const
{
	return d->programName;
}

void ExeFile::setConfigKey(const QString &keyName)
{
	d->configKey = keyName;
}

void ExeFile::setExeTypeName(const QString &name)
{
	d->exeTypeName = name;
}

void ExeFile::setProgramName(const QString &name)
{
	d->programName = name;
}

QString ExeFile::workingDirectory(Message &message)
{
	QFileInfo fi(pathToExe(message));
	return fi.path();
}

///////////////////////////////////////////////////////////////////////////

DClass<ExeFilePath>
{
public:
	QString path;
	QString workingDir;
};

DPointered(ExeFilePath)

ExeFilePath::ExeFilePath()
{
}

ExeFilePath::ExeFilePath(const QString &path)
{
	d->path = path;
	d->workingDir = QFileInfo(path).path();
}

ExeFilePath::ExeFilePath(const ExeFilePath &other)
{
	this->d = other.d;
}

ExeFilePath::~ExeFilePath()
{
}

ExeFilePath &ExeFilePath::operator=(const ExeFilePath &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

QString ExeFilePath::path() const
{
	return d->path;
}

ExeFilePath &ExeFilePath::setPath(const QString &path)
{
	d->path = path;
	return *this;
}

QString ExeFilePath::workingDir() const
{
	return d->workingDir;
}

ExeFilePath &ExeFilePath::setWorkingDir(const QString &path)
{
	d->workingDir = path;
	return *this;
}
