//------------------------------------------------------------------------------
// htmlparser.h
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
#ifndef __HTML_H_
#define __HTML_H_

#include "wwwseeker/link.h"
#include <QByteArray>
#include <QString>
#include <QStringList>

class HtmlParser
{
public:
	/**
	 * @brief Construct a HtmlParser.
	 *
	 * HtmlParser creates a copy of passed data and then performs all
	 * operations on that copy.
	 *
	 * @param siteContents
	 *      Site's HTML code.
	 */
	HtmlParser(const QByteArray &siteContents);

	/**
	 * @brief Find any HTML tag starting from index in the byte array.
	 *
	 * Search is performed by finding the first occurence of '&lt;' character
	 * and then continuing the search from that point until the first
	 * occurence of '&gt;' character is found.
	 *
	 * @param beginAt
	 *      Index from which to begin the search.
	 * @param [out] end
	 *      End index of the found tag.
	 * @return
	 *      Begin index of the found tag
	 */
	int findTag(int beginAt, int *end);

	/**
	 * @brief Extract tag attribute's value at specific location.
	 *
	 * You put something like HREF="http://127.0.0.1" and
	 * it retrieves the thing after '=' without the "".
	 *
	 * @param beginIndex
	 *      Index from which parsing starts
	 * @param endIndex
	 *      Index at which parsing ends
	 * @return
	 *      Trimmed value, without white-spaces and quotes.
	 */
	QString htmlValue(int beginIndex, int endIndex);

	/**
	 * @brief Extract tag attribute's value.
	 *
	 * You put a string of values, for example
	 * <A HREF="http://127.0.0.1/" TARGET="_blank"> and it
	 * retrieves the value for the specified attribute.

	 * @param attribute
	 *      Attribute that will be searched for (for example: HREF)
	 */
	QString htmlValue(const QString &attribute);

	/**
	 * @brief Extract all links from the HTML.
	 */
	QList<Link> linksFromHtml();

private:
	QByteArray data;

	/**
	 * @brief Captialize HTML tags in HTML data array.
	 *
	 * This is required to perform uniform searches for HTML tags.
	 */
	void capitalizeHtmlTags();
};

#endif
