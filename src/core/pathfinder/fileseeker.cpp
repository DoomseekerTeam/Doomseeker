//------------------------------------------------------------------------------
// fileseeker.cpp
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
#include "fileseeker.h"

#include "caseinsensitivefsfileseeker.h"
#include "casesensitivefsfileseeker.h"
#include "pathfinder/filesearchpath.h"

FileSeeker::FileSeeker(QSharedPointer <QList<FileSearchPath> > paths)
{
	this->paths = paths;
}

FileSeeker::~FileSeeker()
{
}

FileSeeker *FileSeeker::createSeeker(QSharedPointer <QList<FileSearchPath> > paths)
{
	#ifdef Q_OS_WIN32
	return new CaseInsensitiveFSFileSeeker(paths);
	#else
	return new CaseSensitiveFSFileSeeker(paths);
	#endif
}

QString FileSeeker::findFile(const QString &fileName)
{
	for (int i = 0; i < paths->size(); ++i)
	{
		QString result = findFileInPath(fileName, (*paths.data())[i]);
		if (!result.isNull())
			return result;
	}
	return QString();
}