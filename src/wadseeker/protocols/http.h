//------------------------------------------------------------------------------
// http.h
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
#ifndef __HTTP_H_
#define __HTTP_H_

#include <QStringList>

class QNetworkReply;

/**
 * @brief Extracts information from QNetworkReply assuming it's a HTTP reply.
 */
class Http
{
public:
	enum HttpResponseCodes
	{
		OK = 200,
		PermanentlyMoved = 301,
		Redirect = 302,
	};

	Http(const QNetworkReply *pReply);

	/**
	 * @brief Extract the attachment name from contentDisposition() list.
	 *
	 * Attachment must be present -- hasAttachment() must return true.
	 * Attachment name is located as the next element on the content
	 * disposition list.
	 *
	 * @return Attachment name is returned if attachment is present
	 *         and name can be extracted.
	 */
	QString attachmentName() const;

	/**
	 * @brief Get "Content-Disposition" split by the `';'` delimiter.
	 *
	 * The returned values are trimmed of white-space.
	 */
	QStringList contentDisposition() const;

	/**
	 * @brief Value under "Content-Length" field.
	 */
	qint64 contentLength() const;

	/**
	 * @brief Value under "Content-Type" field.
	 */
	QString contentType() const;

	/**
	 * @brief Check for "attachment" value in contentDisposition() list.
	 */
	bool hasAttachment() const;

	/**
	 * @brief True if Content-Type starts with "application/"
	 */
	bool isApplicationContentType() const;

	/**
	 * @brief True if Content-Type indicates HTML content.
	 *
	 * @return True for "text/html".
	 */
	bool isHtmlContentType() const;

	/**
	 * @brief True if Content-Type is not defined.
	 */
	bool isUnknownContentType() const;

	/**
	 * @brief Extract the filename from URL.
	 *
	 * The filename is extracted from URL's path. If the filename
	 * exists somewhere in the URL's query, it won't be extracted
	 * properly.
	 */
	QString urlFilename() const;

private:
	const QNetworkReply *pReply;
};

#endif
