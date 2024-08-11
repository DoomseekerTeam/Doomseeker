//------------------------------------------------------------------------------
// urlparser.h
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
#ifndef WADSEEKER_WWW_URLPARSER_H
#define WADSEEKER_WWW_URLPARSER_H

#include <QList>
#include <QStringList>
#include <QUrl>

class Link;

/**
 * @brief A utility function-bag for working with URLs.
 */
class UrlParser
{
public:
	static bool hasSameHost(const QUrl &url1, const QUrl &url2);
	/**
	 * @brief Check if URL is a direct link to one of the filenames.
	 *
	 * @return True if it's safe to assume that URL leads to one of the
	 *         files specified on the wantedFilenames list.
	 */
	static bool isDirectLinkToFile(const QStringList &wantedFilenames, const QUrl &url);

	static bool isWadnameTemplateUrl(const QString &url);
	static QUrl resolveWadnameTemplateUrl(QString url, const QString &wadname);
	/**
	 * @brief Check if URLs are the same; ignores character case.
	 */
	static bool urlEqualsCaseInsensitive(QUrl url1, QUrl url2);

	UrlParser(const QList<Link> &links);

	/**
	 * @brief Extract a list of URLs leading directly to any of the
	 *        wanted files.
	 *
	 * The URLs are extracted from the list of URLs passed to the constructor.
	 *
	 * @param wantedFilenames
	 *      A list of filenames we wish to look for.
	 * @param baseUrl
	 *      Relative URLs will be converted to absolute
	 *      URLs using this URL as a base
	 *
	 * @return A list of URLs that match one or more of the filenames
	 *         on the wantedFiles list.
	 */
	QList<Link> directLinks(const QStringList &wantedFilenames, const QUrl &baseUrl);

	/**
	 * @brief Check if the URL scheme points to an FTP.
	 */
	static bool isFtpUrl(const QUrl &url);

	/**
	 * @brief Check if the URL scheme points to a WWW resource.
	 *
	 * In truth, the allowed schemes are narrowed down to "http",
	 * "https" and "ftp". The web has technically more protocols, but
	 * Wadseeker is not interested in them.
	 *
	 * We definitely aren't interested in "mailto" or "file" schemes, oh no.
	 */
	static bool isWwwUrl(const QUrl &url);

private:
	class PrivData
	{
	public:
		QList<Link> links;
	};

	PrivData d;

	static bool isDirectLinkToFile(const QStringList &wantedFilenames, const Link &link);
};

#endif
