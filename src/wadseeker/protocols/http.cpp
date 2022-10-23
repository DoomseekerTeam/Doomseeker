//------------------------------------------------------------------------------
// http.cpp
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
#include "http.h"
#include "protocols/networkreply.h"
#include "wwwseeker/htmlparser.h"

#include <QFileInfo>

Http::Http(const NetworkReply *pReply)
{
	this->pReply = pReply;
}

QString Http::attachmentName() const
{
	QStringList contentDisposition = this->contentDisposition();

	int attachmentIndex = contentDisposition.indexOf("attachment");
	if (attachmentIndex < 0 || attachmentIndex == contentDisposition.length() - 1)
	{
		// No attachment found or no attachment info available as the attachment
		// is the last element on the list.
		return "";
	}

	QString attachmentInfo = contentDisposition[attachmentIndex + 1];

	// Now we need to extract the filename from the INI-like list.
	// HtmlParser already has appropriate method.
	HtmlParser parser(attachmentInfo.toUtf8());
	return parser.htmlValue("filename").trimmed();
}

QStringList Http::contentDisposition() const
{
	// Byte array will be empty if header doesn't exist.
	QByteArray headerData = pReply->rawHeader("Content-Disposition");
	QString headerContent(headerData);

	QStringList contentDisposition = headerContent.split(";");

	QStringList filteredContentDisposition;

	// Check if there is any real content and not just 1 size list with nothing
	// inside.
	if (contentDisposition.size() == 1 && contentDisposition.first().trimmed().isEmpty())
	{
		// No real content. Return empty list.
		return filteredContentDisposition;
	}

	for (QString value : contentDisposition)
	{
		value = value.trimmed();
		filteredContentDisposition << value;
	}

	return filteredContentDisposition;
}

qint64 Http::contentLength() const
{
	return pReply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
}

QString Http::contentType() const
{
	return pReply->header(QNetworkRequest::ContentTypeHeader).toString();
}

bool Http::hasAttachment() const
{
	QStringList disposition = contentDisposition();
	return disposition.contains("attachment");
}

bool Http::isApplicationContentType() const
{
	return contentType().startsWith("application/");
}

bool Http::isHtmlContentType() const
{
	return contentType().contains("text/html");
}

QString Http::urlFilename() const
{
	const QUrl &url = pReply->request().url();
	QFileInfo fileInfo(url.path());

	return fileInfo.fileName();
}
