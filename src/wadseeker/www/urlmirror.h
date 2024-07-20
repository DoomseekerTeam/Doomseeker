//------------------------------------------------------------------------------
// urlmirror.h
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
#ifndef WADSEEKER_WWW_URLMIRROR_H
#define WADSEEKER_WWW_URLMIRROR_H

#include <QList>
#include <QString>
#include <QUrl>

/**
 * Store different URLs to the (presumed) same resource.
 */
class UrlMirror
{
public:
	QList<QUrl> urls;
	QString resource;

	UrlMirror() {}
	UrlMirror(QList<QUrl> urls, QString resource = QString())
		: urls(urls), resource(resource) {}
	UrlMirror(QUrl url, QString resource = QString())
		: resource(resource)
	{
		this->urls << url;
	}

	bool hasUrl(const QUrl &url) const;
	bool isEmpty() const
	{
		return urls.isEmpty();
	}
};

#endif
