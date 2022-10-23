//------------------------------------------------------------------------------
// zandronumqbinaries.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "log.h"

#include "../wadseeker/zip/unarchive.h"
#include "application.h"
#include "plugins/engineplugin.h"
#include "serverapi/message.h"
#include "version.h"
#include "zandronumqbinaries.h"
#include "zandronumqengineplugin.h"
#include "zandronumqserver.h"

#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QScopedPointer>
#include <strings.hpp>

#ifdef Q_OS_WIN32
#define ZANDRONUMQ_BINARY_NAME "q-zandronum.exe"
#define SCRIPT_FILE_EXTENSION ".bat"
#else

#ifndef Q_OS_DARWIN
#define ZANDRONUMQ_BINARY_NAME "q-zandronum"
#else
#define ZANDRONUMQ_BINARY_NAME "Q-Zandronum.app"
#define ZANDRONUMQ_APP_BUNDLE_BIN "/Contents/MacOS/q-zandronum"
#endif
#define SCRIPT_FILE_EXTENSION ".sh"
#endif

class ZandronumqClientExeFile::PrivData
{
public:
	QSharedPointer<const ZandronumqServer> server;

	bool isEmptyDir(const QString &path) const
	{
		QDir dir(path);
		return dir.exists() && dir.entryList(
			QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}

	bool isNonEmptyDir(const QString &path) const
	{
		QDir dir(path);
		return dir.exists() && !dir.entryList(
			QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}
};

ZandronumqClientExeFile::ZandronumqClientExeFile(const QSharedPointer<const ZandronumqServer> &server)
{
	d = new PrivData();
	d->server = server;
	setProgramName(server->plugin()->data()->name);
	setExeTypeName(tr("client"));
	setConfigKey("BinaryPath");
}

ZandronumqClientExeFile::~ZandronumqClientExeFile()
{
	delete d;
}

IniSection &ZandronumqClientExeFile::config()
{
	return *ZandronumqEnginePlugin::staticInstance()->data()->pConfig;
}

Message ZandronumqClientExeFile::install(QWidget *parent)
{
	return Message(Message::Type::CANCELLED);
}

QString ZandronumqClientExeFile::pathToExe(Message &message)
{
	return ExeFile::pathToExe(message);
}

QString ZandronumqClientExeFile::workingDirectory(Message &message)
{
	Q_UNUSED(message);
	IniSection &config = *ZandronumqEnginePlugin::staticInstance()->data()->pConfig;

	QFileInfo fi((QString)config["BinaryPath"]);
	return fi.absolutePath();
}

QString ZandronumqClientExeFile::scriptFileExtension()
{
	return SCRIPT_FILE_EXTENSION;
}
