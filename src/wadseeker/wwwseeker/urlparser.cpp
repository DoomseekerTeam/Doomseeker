//------------------------------------------------------------------------------
// urlparser.cpp
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "urlparser.h"

#include <QFileInfo>

#include "entities/link.h"

UrlParser::UrlParser(const QList<Link> &links)
{
	d.links = links;
}

QList<Link> UrlParser::directLinks(const QStringList &wantedFilenames, const QUrl &baseUrl)
{
	QList<Link> linksList;

	for (Link link : d.links)
	{
		if (isDirectLinkToFile(wantedFilenames, link))
		{
			if (link.url.isRelative())
			{
				link.url = baseUrl.resolved(link.url);
			}

			linksList << link;
		}
	}

	return linksList;
}

bool UrlParser::hasFileReferenceSomewhere(const QStringList &wantedFilenames, const Link &link)
{
	QString strQuery = link.url.query(QUrl::FullyDecoded);

	for (const QString &filename : wantedFilenames)
	{
		if (strQuery.contains(filename, Qt::CaseInsensitive)
			|| link.text.contains(filename, Qt::CaseInsensitive))
		{
			return true;
		}
	}

	return false;
}

bool UrlParser::hasSameHost(const QUrl &url1, const QUrl &url2)
{
	return url1.host().compare(url2.host(), Qt::CaseInsensitive) == 0;
}

bool UrlParser::isDirectLinkToFile(const QStringList &wantedFilenames, const QUrl &url)
{
	// We want path to match raw filename. This means no
	// special percent encoding for characters that are valid on file
	// system.
	QString urlPath = url.path(QUrl::FullyDecoded);
	QFileInfo fi(urlPath);

	for (const QString &filename : wantedFilenames)
	{
		if (fi.fileName().compare(filename, Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}

	return false;
}

bool UrlParser::isDirectLinkToFile(const QStringList &wantedFilenames, const Link &link)
{
	return isDirectLinkToFile(wantedFilenames, link.url);
}

bool UrlParser::isWadnameTemplateUrl(const QString &url)
{
	return url.contains("%WADNAME%") || url.contains("%s");
}

QUrl UrlParser::resolveWadnameTemplateUrl(QString url, const QString &wadname)
{
	QString wadnameEncoded = QUrl::toPercentEncoding(wadname);
	if (url.contains("%WADNAME%"))
	{
		url = url.replace("%WADNAME%", wadnameEncoded);
	}
	else
	{
		url = url.replace("%s", wadnameEncoded);
	}
	return url;
}

QList<Link> UrlParser::siteLinks(const QStringList &wantedFilenames, const QUrl &baseUrl)
{
	QList<Link> linksList;

	for (Link link : d.links)
	{
		// Make sure direct links are not listed here.
		if (hasFileReferenceSomewhere(wantedFilenames, link)
			&& !isDirectLinkToFile(wantedFilenames, link))
		{
			if (link.url.isRelative())
			{
				link.url = baseUrl.resolved(link.url);
			}

			linksList << link;
		}
	}

	return linksList;
}

bool UrlParser::urlEqualsCaseInsensitive(QUrl url1, QUrl url2)
{
	// Normalize URLs to lower-case.
	url1 = QUrl(url1.toString().toLower());
	url2 = QUrl(url2.toString().toLower());

	return url1 == url2;
}
