//------------------------------------------------------------------------------
// pathfind.cpp
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
#include "pathfind.h"

#include "serverapi/gamefile.h"
#include <QDir>
#include <QFileInfo>
#include <QStringList>

QString PathFind::findExe(const PathFinder &pathFinder, const QString &name)
{
	#if defined(Q_OS_WIN32)
	if (!name.endsWith(".exe"))
	{
		QString file = pathFinder.findFile(name + ".exe");
		if (!file.isEmpty())
		{
			return file;
		}
	}
	#endif
	return pathFinder.findFile(name);
}

QString PathFind::findGameFile(const QStringList &knownPaths, const GameFile &gameFile)
{
	PathFinder pathFinder = PathFinder::genericPathFinder(gameFile.searchSuffixes());
	QStringList knownDirs;
	for (const QString &path : knownPaths)
	{
		if (!path.trimmed().isEmpty())
		{
			QFileInfo fileInfo(path);
			if (fileInfo.isDir())
			{
				pathFinder.addPrioritySearchDir(fileInfo.filePath());
			}
			if (fileInfo.dir().exists())
			{
				pathFinder.addPrioritySearchDir(fileInfo.path());
			}
		}
	}
	return findGameFile(pathFinder, gameFile);
}

QString PathFind::findGameFile(const PathFinder &pathFinder, const GameFile &gameFile)
{
	if (gameFile.executable() != 0)
	{
		return findExe(pathFinder, gameFile.fileName());
	}
	else
	{
		return pathFinder.findFile(gameFile.fileName());
	}
}
