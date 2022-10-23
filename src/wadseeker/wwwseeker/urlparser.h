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
#ifndef __URLPARSER_H__
#define __URLPARSER_H__

#include <QList>
#include <QStringList>
#include <QUrl>

class Link;

/**
 * @brief Parses URLs and splits them to groups of interest.
 *
 * This class operates on a list of URLs to divide them into groups.
 *
 * URLs are parsed to the Wadseeker needs. Direct links to files are extracted
 * as one group and links to subsites where the files may possibly reside
 * are extracted as the other group.
 */
class UrlParser
{
public:
	static bool hasSameHost(const QUrl &url1, const QUrl &url2);
	/**
	 * @brief Checks if URL is a direct link to one of the filenames.
	 *
	 * @return True if it's safe to assume that URL leads to one of the
	 *         files specified on the wantedFilenames list.
	 */
	static bool isDirectLinkToFile(const QStringList &wantedFilenames, const QUrl &url);

	static bool isWadnameTemplateUrl(const QString &url);
	static QUrl resolveWadnameTemplateUrl(QString url, const QString &wadname);
	/**
	 * @brief Checks if URLs are the same. Ignores character case.
	 */
	static bool urlEqualsCaseInsensitive(QUrl url1, QUrl url2);

	UrlParser(const QList<Link> &links);

	/**
	 * @brief Extracts a list of URLs leading directly to any of the
	 *        wanted files.
	 *
	 * @param wantedFilenames
	 *      A list of filenames we wish to extract from the URLs specified
	 *      in the constructor.
	 * @param baseUrl
	 *      Relative URLs will be converted to absolute
	 *      URLs using this URL as a base
	 *
	 * @return A list of URLs that match one or more of the filenames
	 *         on the wantedFiles list.
	 */
	QList<Link> directLinks(const QStringList &wantedFilenames, const QUrl &baseUrl);

	/**
	 * @brief Extracts a list of URLs leading to subsites that may contain
	 *        any of the wanted files.
	 *
	 * @param wantedFilenames
	 *      A list of filenames we wish to extract from the URLs specified
	 *      in the constructor.
	 * @param baseUrl
	 *      Relative URLs will be converted to absolute
	 *      URLs using this URL as a base
	 *
	 * @return A list of URLs that match one or more of the filenames
	 *         on the wantedFiles list.
	 */
	QList<Link> siteLinks(const QStringList &wantedFilenames, const QUrl &baseUrl);

private:
	class PrivData
	{
	public:
		QList<Link> links;
	};

	PrivData d;

	static bool hasFileReferenceSomewhere(const QStringList &wantedFilenames, const Link &link);
	static bool isDirectLinkToFile(const QStringList &wantedFilenames, const Link &link);
};

#endif
