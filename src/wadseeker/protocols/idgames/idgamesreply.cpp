//------------------------------------------------------------------------------
// idgamesreply.cpp
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
#include "idgamesreply.h"

#include "protocols/idgames/idgamesresult.h"
#include <QDebug>
#include <QJsonDocument>

static const int MAX_REDIRECTS = 10;

class IdgamesReply::PrivData
{
public:
	QNetworkReply *reply;
	QNetworkRequest request;
	IdgamesResult result;
	int redirects;
};

IdgamesReply::IdgamesReply(const QNetworkRequest &request, QNetworkReply *reply)
{
	d = new PrivData();
	d->reply = nullptr;
	d->request = request;
	d->redirects = 0;
	setReply(reply);
}

IdgamesReply::~IdgamesReply()
{
	d->reply->deleteLater();
	delete d;
}

void IdgamesReply::abort()
{
	d->reply->abort();
}

void IdgamesReply::onNetworkFinished()
{
	if (d->reply->error() == QNetworkReply::NoError)
	{
		QVariant possibleRedirect = d->reply->attribute(
			QNetworkRequest::RedirectionTargetAttribute);
		if (possibleRedirect.isValid())
		{
			#ifndef NDEBUG
			qDebug() << "idgames redirect: " << possibleRedirect;
			#endif
			redirect(possibleRedirect.toUrl());
		}
		else
		{
			QString json = d->reply->readAll();
			#ifndef NDEBUG
			qDebug() << "idgames onNetworkFinished()" << json;
			qDebug() << "idgames onNetworkFinished() parsed" << QJsonDocument::fromJson(json.toUtf8());
			#endif
			finish(IdgamesResult(QJsonDocument::fromJson(json.toUtf8())));
		}
	}
	else
	{
		finish(IdgamesResult::mkError(tr("Network Error"), d->reply->errorString()));
	}
}

void IdgamesReply::finish(const IdgamesResult &result)
{
	d->result = result;
	emit finished();
}

void IdgamesReply::redirect(QUrl redirectUrl)
{
	if (d->redirects++ >= MAX_REDIRECTS)
	{
		finish(IdgamesResult::mkError(tr("Service error"), tr("Too many redirects")));
		return;
	}
	if (redirectUrl.isRelative())
	{
		redirectUrl = d->request.url().resolved(redirectUrl);
	}
	QNetworkRequest request = d->request;
	request.setUrl(redirectUrl);
	QNetworkReply *redirectedReply = d->reply->manager()->get(request);
	d->request = request;
	setReply(redirectedReply);
}

IdgamesResult IdgamesReply::result() const
{
	return d->result;
}

void IdgamesReply::setReply(QNetworkReply *reply)
{
	if (d->reply != nullptr)
	{
		d->reply->disconnect();
		d->reply->deleteLater();
	}
	d->reply = reply;
	this->connect(reply, SIGNAL(finished()), SLOT(onNetworkFinished()));
}
