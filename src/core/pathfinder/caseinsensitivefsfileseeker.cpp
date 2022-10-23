//------------------------------------------------------------------------------
// caseinsensitivefsfileseeker.cpp
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
#include "caseinsensitivefsfileseeker.h"

#include "pathfinder/filesearchpath.h"
#include "strings.hpp"

#include <QDir>
#include <QFileInfo>

CaseInsensitiveFSFileSeeker::CaseInsensitiveFSFileSeeker(QSharedPointer <QList<FileSearchPath> > paths) :
	FileSeeker(paths)
{
}

QString CaseInsensitiveFSFileSeeker::findFileInPath(const QString &fileName, FileSearchPath &path)
{
	QFileInfo file(Strings::combinePaths(path.path(), fileName));
	if (file.exists() && file.isFile())
		return file.absoluteFilePath();
	else if (path.isRecursive())
	{
		QDir dir(path.path());
		QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (const QString &subDir : subDirs)
		{
			FileSearchPath subSearchPath(dir.filePath(subDir));
			subSearchPath.setRecursive(true);
			QString result = findFileInPath(fileName, subSearchPath);
			if (!result.isNull())
				return result;
		}
	}
	return QString();
}