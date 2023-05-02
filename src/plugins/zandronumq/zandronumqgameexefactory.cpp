//------------------------------------------------------------------------------
// zandronumqgameexefactory.cpp
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
#include "zandronumqgameexefactory.h"

#include "zandronumqbinaries.h"
#include <ini/inisection.h>
#include <plugins/engineplugin.h>
#include <QDir>
#include <serverapi/exefile.h>
#include <serverapi/gamefile.h>

DClass<ZandronumqGameExeFactory>
{
public:
};
DPointered(ZandronumqGameExeFactory)

ZandronumqGameExeFactory::ZandronumqGameExeFactory(EnginePlugin *plugin)
	: GameExeFactory(plugin)
{
}

ZandronumqGameExeFactory::~ZandronumqGameExeFactory()
{
}

QList<ExeFilePath> ZandronumqGameExeFactory::scanSubdir(const QDir &mainDir,
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
					subdirName + ZandronumqClientExeFile::scriptFileExtension());
		}
	}
	if (execType & GameFile::Server)
	{
		if (!subdir.entryList(QStringList(serverExecName()), QDir::Files).isEmpty())
		{
			#ifdef Q_OS_WIN32
			paths << mainDir.absoluteFilePath(
					subdirName + ZandronumqClientExeFile::scriptFileExtension());
			#else
			paths << mainDir.absoluteFilePath(subdirName + "/q-zandronum-server");
			#endif
		}
	}
	return paths;
}

QString ZandronumqGameExeFactory::execName() const
{
	#ifdef Q_OS_WIN32
	return "q-zandronum.exe";
	#else
	return "q-zandronum";
	#endif
}

QString ZandronumqGameExeFactory::serverExecName() const
{
	#ifdef Q_OS_WIN32
	return "q-zandronum.exe";
	#else
	return "q-zandronum-server";
	#endif
}
