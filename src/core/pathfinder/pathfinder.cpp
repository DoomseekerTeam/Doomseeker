//------------------------------------------------------------------------------
// pathfinder.cpp
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "pathfinder.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "fileseeker.h"
#include "log.h"
#include "pathfinder/filesearchpath.h"
#include "strings.hpp"
#include "templatedpathresolver.h"

#include <cstdlib>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QScopedPointer>
#include <QSharedPointer>

DClass<PathFinderResult>
{
public:
	QStringList foundFiles;
	QStringList missingFiles;
};


DPointered(PathFinderResult)


PathFinderResult::PathFinderResult()
{
}

PathFinderResult::~PathFinderResult()
{
}

QStringList &PathFinderResult::foundFiles()
{
	return d->foundFiles;
}

const QStringList &PathFinderResult::foundFiles() const
{
	return d->foundFiles;
}

QStringList &PathFinderResult::missingFiles()
{
	return d->missingFiles;
}

const QStringList &PathFinderResult::missingFiles() const
{
	return d->missingFiles;
}

////////////////////////////////////////////////////////////////////////////////

DClass<PathFinder>
{
public:
	QSharedPointer <QList<FileSearchPath> > searchPaths;

	QString resolveDir(const QString &dir)
	{
		QFileInfo fileInfo(gDoomseekerTemplatedPathResolver().resolve(dir));
		if (fileInfo.isSymLink())
			fileInfo = QFileInfo(fileInfo.symLinkTarget());

		if (fileInfo.isBundle())
			return fileInfo.absoluteFilePath() + "/Contents/MacOS";
		else
		{
			if (fileInfo.isFile())
				return fileInfo.absoluteDir().absolutePath();
			else
				return fileInfo.absoluteFilePath();
		}
	}
};


DPointered(PathFinder)


PathFinder::PathFinder()
{
	d->searchPaths.reset(new QList<FileSearchPath>());
	auto resolver = gDoomseekerTemplatedPathResolver();
	d->searchPaths->append(
		FileSearchPath::resolveTemplated(resolver,
			gConfig.combinedWadseekPaths()));
	FileSearchPath::merge(*d->searchPaths);
}

PathFinder::PathFinder(const QStringList &paths)
{
	d->searchPaths.reset(new QList<FileSearchPath>());
	for (const QString &path : paths)
		d->searchPaths->append(path);
	FileSearchPath::merge(*d->searchPaths);
}

PathFinder::~PathFinder()
{
}

PathFinder PathFinder::genericPathFinder(const QStringList &suffixes)
{
	QStringList paths;
	#if defined(Q_OS_WIN32)
	paths << "." << ".."
		<< QCoreApplication::applicationDirPath()
		<< QDir(QCoreApplication::applicationDirPath()).filePath("..")
		<< DataPaths::programFilesDirectory(DataPaths::x64)
		<< DataPaths::programFilesDirectory(DataPaths::x86);
	#else
	paths << "/usr/bin" << "/usr/local/bin" << "/usr/share/bin"
		<< "/usr/games/" << "/usr/local/games/"
		<< "/usr/share/games/" << QCoreApplication::applicationDirPath() << ".";
	#endif
	QStringList pathsCopy(paths);
	for (const QString &path : pathsCopy)
	{
		for (const QString &suffix : suffixes)
		{
			paths << Strings::combinePaths(path, suffix);
		}
	}
	return PathFinder(paths);
}

void PathFinder::addPrioritySearchDir(const QString &dir)
{
	d->searchPaths->prepend(d->resolveDir(dir));
	FileSearchPath::merge(*d->searchPaths);
}

void PathFinder::addSearchDir(const QString &dir)
{
	d->searchPaths->append(d->resolveDir(dir));
	FileSearchPath::merge(*d->searchPaths);
}

QString PathFinder::findFile(const QString &fileName) const
{
	if (d->searchPaths->count() == 0)
		return QString();
	QScopedPointer<FileSeeker> seeker(FileSeeker::createSeeker(d->searchPaths));
	QString result = seeker->findFile(fileName);
	return result;
}

PathFinderResult PathFinder::findFiles(const QStringList &files) const
{
	PathFinderResult result;
	for (const QString file : files)
	{
		QString filePath = findFile(file);
		if (filePath.isNull())
		{
			result.missingFiles() << file;
		}
		else
		{
			result.foundFiles() << filePath;
		}
	}

	return result;
}
