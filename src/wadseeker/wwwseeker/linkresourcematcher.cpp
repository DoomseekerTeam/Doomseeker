//------------------------------------------------------------------------------
// linkresourcematcher.cpp
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "linkresourcematcher.h"

#include "wwwseeker/fileseekinfo.h"

#include <QFileInfo>
#include <QString>

static bool isDirectLinkToFile(const QStringList &seekedFilenames, const Link &link)
{
	// We want path to match the raw filename. This means no
	// special percent encoding for characters that are valid on file
	// system.
	QString urlPath = link.url.path(QUrl::FullyDecoded);
	QFileInfo fi(urlPath);
	for (const QString &filename : seekedFilenames)
	{
		if (fi.fileName().compare(filename, Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}
	return false;
}

static bool hasFileReferenceSomewhere(const QStringList &seekedFilenames, const Link &link)
{
	QString strQuery = link.url.query(QUrl::FullyDecoded);
	for (const QString &filename : seekedFilenames)
	{
		if (strQuery.contains(filename, Qt::CaseInsensitive)
			|| link.text.contains(filename, Qt::CaseInsensitive))
		{
			return true;
		}
	}
	return false;
}

QList<LinkResourceMatch> LinkResourceMatcher::matchLink(
	const QList<FileSeekInfo> &seekedFiles,
	const Link &link)
{
	QList<LinkResourceMatch> matches;
	for (const FileSeekInfo &seekedFile : seekedFiles)
	{
		const QStringList &possibleFilenames = seekedFile.possibleFilenames();

		if (isDirectLinkToFile(possibleFilenames, link)
			|| hasFileReferenceSomewhere(possibleFilenames, link))
		{
			LinkResourceMatch match;
			match.resource = seekedFile.file();
			match.link = link;
			matches << match;
		}
	}

	return matches;
}
