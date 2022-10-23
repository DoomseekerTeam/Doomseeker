//------------------------------------------------------------------------------
// zandronumgameexefactory.cpp
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
#include "zandronumgameexefactory.h"

#include "zandronumbinaries.h"
#include <ini/inisection.h>
#include <plugins/engineplugin.h>
#include <QDir>
#include <serverapi/exefile.h>
#include <serverapi/gamefile.h>

DClass<ZandronumGameExeFactory>
{
public:
};
DPointered(ZandronumGameExeFactory)

ZandronumGameExeFactory::ZandronumGameExeFactory(EnginePlugin *plugin)
	: GameExeFactory(plugin)
{
	set_additionalExecutables(&ZandronumGameExeFactory::additionalExecutables);
}

ZandronumGameExeFactory::~ZandronumGameExeFactory()
{
}

QList<ExeFilePath> ZandronumGameExeFactory::additionalExecutables(int execType) const
{
	QList<ExeFilePath> testingExecutables;
	IniSection *cfg = plugin()->data()->pConfig;
	if (cfg != nullptr)
	{
		QDir directory(cfg->value("TestingPath").toString());
		QStringList subdirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
		for (const QString &subdirName : subdirs)
		{
			testingExecutables << scanSubdir(directory, subdirName, execType);
		}
	}
	return testingExecutables;
}

QList<ExeFilePath> ZandronumGameExeFactory::scanSubdir(const QDir &mainDir,
	const QString &subdirName, int execType) const
{
	QList<ExeFilePath> paths;
	QDir subdir = mainDir;
	subdir.cd(subdirName);
	if (execType & GameFile::Client || execType & GameFile::Offline)
	{
		if (!subdir.entryList(QStringList(execName()), QDir::Files).isEmpty())
		{
			paths << mainDir.absoluteFilePath(
					subdirName + ZandronumClientExeFile::scriptFileExtension());
		}
	}
	if (execType & GameFile::Server)
	{
		if (!subdir.entryList(QStringList(serverExecName()), QDir::Files).isEmpty())
		{
			#ifdef Q_OS_WIN32
			paths << mainDir.absoluteFilePath(
					subdirName + ZandronumClientExeFile::scriptFileExtension());
			#else
			paths << mainDir.absoluteFilePath(subdirName + "/zandronum-server");
			#endif
		}
	}
	return paths;
}

QString ZandronumGameExeFactory::execName() const
{
	#ifdef Q_OS_WIN32
	return "zandronum.exe";
	#else
	return "zandronum";
	#endif
}

QString ZandronumGameExeFactory::serverExecName() const
{
	#ifdef Q_OS_WIN32
	return "zandronum.exe";
	#else
	return "zandronum-server";
	#endif
}
