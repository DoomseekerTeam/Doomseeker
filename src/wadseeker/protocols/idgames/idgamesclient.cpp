//------------------------------------------------------------------------------
// idgamesclient.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "idgamesclient.h"

#include "protocols/idgames/idgamesreply.h"
#include <QDebug>

const QUrl IdgamesClient::DEFAULT_URL("https://www.doomworld.com/idgames/api/api.php");

class IdgamesClient::PrivData
{
public:
	QUrl baseUrl;
	QNetworkAccessManager *nam;
	QString userAgent;
};

IdgamesClient::IdgamesClient(QNetworkAccessManager *nam, const QUrl &baseUrl)
{
	d = new PrivData();
	d->nam = nam;
	d->baseUrl = baseUrl;
}

IdgamesClient::~IdgamesClient()
{
	delete d;
}

const QUrl &IdgamesClient::baseUrl() const
{
	return d->baseUrl;
}

IdgamesReply *IdgamesClient::search(const QString &filename)
{
	QString url = d->baseUrl.toString().trimmed();
	url += QString("?out=json&action=search&query=%1&dir=desc").arg(filename);
	#ifndef NDEBUG
	qDebug() << "idgames search:" << url;
	#endif
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", d->userAgent.toUtf8());
	QNetworkReply *reply = d->nam->get(request);
	return new IdgamesReply(request, reply);
}

void IdgamesClient::setBaseUrl(const QUrl &baseUrl)
{
	d->baseUrl = baseUrl;
}

void IdgamesClient::setUserAgent(const QString &userAgent)
{
	d->userAgent = userAgent;
}
