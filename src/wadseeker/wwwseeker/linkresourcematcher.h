//------------------------------------------------------------------------------
// linkresourcematcher.h
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
#ifndef WADSEEKER_WWWSEEKER_LINKRESOURCEMATCHER_H
#define WADSEEKER_WWWSEEKER_LINKRESOURCEMATCHER_H

#include "wwwseeker/link.h"

#include <QList>
#include <QStringList>
#include <QUrl>

class WadDownloadInfo;
using FileSeekInfo = WadDownloadInfo;

struct LinkResourceMatch
{
	QString resource;
	Link link;

	bool isValid() const
	{
		return link.url.isValid();
	}
};

class LinkResourceMatcher
{
public:
	/**
	 * @brief Check if Link leads to any of the seeked files.
	 *
	 * It may happen that a single link potentially matches more than
	 * one file, so a list is returned
	 *
	 * @param seekedFiles
	 *     A list of files we wish to extract from the links.
	 * @param link
	 *     The link we wish to match.
	 *
	 * @return One or more match struct. If nothing matches, empty.
	 */
	static QList<LinkResourceMatch> matchLink(
		const QList<FileSeekInfo> &seekedFiles,
		const Link &link);
};

#endif
