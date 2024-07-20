//------------------------------------------------------------------------------
// www.cpp
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
#include "www.h"

#include "protocols/http.h"
#include "wadseekermessagetype.h"
#include "www/urlparser.h"
#include "www/webquery.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSharedPointer>
#include <QString>
#include <QUrl>

DClass<WWW>
{
public:
	QNetworkAccessManager *net;
	QString userAgent;

	QList<QSharedPointer<WebQuery>> queries;

	QSharedPointer<WebQuery> findQuery(QObject *obj)
	{
		auto *query = qobject_cast<WebQuery *>(obj);
		return query != nullptr ? findQuery(query) : QSharedPointer<WebQuery>();
	}

	QSharedPointer<WebQuery> findQuery(WebQuery *pQuery)
	{
		for (auto query : queries)
		{
			if (query.data() == pQuery)
				return query;
		}
		return QSharedPointer<WebQuery>();
	}
};
DPointeredNoCopy(WWW);

WWW::WWW(QNetworkAccessManager *net)
{
	d->net = net;
}

WWW::~WWW()
{
}

QSharedPointer<WebQuery> WWW::get(const QUrl &url)
{
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", d->userAgent.toUtf8());

	auto reply = d->net->get(request);

	QSharedPointer<WebQuery> query = QSharedPointer<WebQuery>::create(request, reply);

	d->queries << query;
	connect(query.data(), &WebQuery::finished,
		this, [this, query]()
		{
			d->queries.removeOne(query);
		});
	return query;
}

unsigned WWW::inFlightCount() const
{
	return d->queries.size();
}

bool WWW::isHostInflight(const QUrl &url) const
{
	for (QSharedPointer<WebQuery> query : d->queries)
	{
		if (UrlParser::hasSameHost(query->url(), url))
		{
			return true;
		}
	}
	return false;
}

void WWW::setUserAgent(const QString &userAgent)
{
	d->userAgent = userAgent;
}
