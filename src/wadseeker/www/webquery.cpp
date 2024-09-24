//------------------------------------------------------------------------------
// webquery.cpp
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
#include "webquery.h"

#include "protocols/http.h"
#include "www/urlparser.h"

#include <QTimer>
#include <cassert>

DClass<WebQuery>
{
public:
	QTimer timeout;
};
DPointeredNoCopy(WebQuery);

WebQuery::WebQuery(const QNetworkRequest &request, QNetworkReply *reply)
	: m_state(State::Running), m_reply(reply), m_request(request),
	  m_resourceType(WebResourceType::Unknown)
{
	assert(reply != nullptr);
	connect(reply, &QNetworkReply::downloadProgress,
		this, &WebQuery::progress);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	connect(reply, &QNetworkReply::errorOccurred,
		this, &WebQuery::onError);
#else
	connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
		this, &WebQuery::onError);
#endif
	connect(reply, &QNetworkReply::finished,
		this, &WebQuery::onFinished);
	connect(reply, &QNetworkReply::downloadProgress,
		this, &WebQuery::onProgress);
	connect(reply, &QNetworkReply::metaDataChanged,
		this, &WebQuery::onMetaDataChanged);
	connect(reply, &QNetworkReply::sslErrors,
		this, &WebQuery::onSslErrors);

	connect(&d->timeout, &QTimer::timeout,
		this, &WebQuery::onTimeout);
	d->timeout.start(DEFAULT_CONNECTION_TIMEOUT_MSECS);
}

WebQuery::~WebQuery()
{
	this->m_reply->deleteLater();
}

void WebQuery::copyErrorOver()
{
	switch (this->m_state)
	{
	case WebQuery::TimedOut:
		this->m_error = QNetworkReply::TimeoutError;
		this->m_errorString = tr("Timed out");
		break;
	default:
		this->m_error = this->m_reply->error();
		this->m_errorString = this->m_reply->errorString();
		break;
	}
}

bool WebQuery::isRedirected() const
{
	QUrl redirect = this->m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	return redirect.isValid();
}

QUrl WebQuery::redirectUrl() const
{
	QUrl redirect = this->m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	return this->m_reply->url().resolved(redirect);
}

void WebQuery::onError(QNetworkReply::NetworkError code)
{
	this->copyErrorOver();
	emit errorOccurred();
}

void WebQuery::onFinished()
{
	d->timeout.stop();
	this->copyErrorOver();
	this->m_state = WebQuery::Finished;
	emit finished();
}

void WebQuery::onMetaDataChanged()
{
	// Ignore a query that has already been recognized.
	if (this->m_state != WebQuery::Running)
		return;

	// Check if this is a redirect.
	if (isRedirected())
	{
		this->m_state = WebQuery::Redirected;
		return;
	}

	if (UrlParser::isFtpUrl(this->url()))
	{
		QString urlPath = this->url().path();
		if (urlPath.endsWith(".htm") || urlPath.endsWith(".html"))
		{
			this->m_resourceType = WebResourceType::Html;
			emit foundType(WebResourceType::Html);
		}
		else
		{
			this->m_resourceType = WebResourceType::Binary;
			emit foundType(WebResourceType::Binary);
		}
		this->m_state = WebQuery::Recognized;
	}
	else
	{
		Http http(this->m_reply);
		if (http.isHtmlContentType())
		{
			this->m_state = WebQuery::Recognized;
			this->m_resourceType = WebResourceType::Html;
			emit foundType(this->m_resourceType);
		}
		else if (http.isApplicationContentType()
			|| http.isUnknownContentType())
		{
			// The HTTP server may not always the Content-Type header,
			// e.g. this is the case with pk3 files. If the header is
			// not present, let's assume that the content is Binary and
			// then deal with it later on.
			this->m_state = WebQuery::Recognized;
			this->m_resourceType = WebResourceType::Binary;
			emit foundType(this->m_resourceType);
		}
	}
}

void WebQuery::onProgress()
{
	d->timeout.start(DEFAULT_PROGRESS_TIMEOUT_MSECS);
}

void WebQuery::onSslErrors(const QList<QSslError> &errors)
{
	this->copyErrorOver();
	emit errorOccurred();
}

void WebQuery::onTimeout()
{
	d->timeout.stop();

	// Defensive programming: if we're finished, we don't care about the
	// timeout anymore.
	if (this->m_state != WebQuery::Finished)
	{
		this->m_state = WebQuery::TimedOut;
		this->m_reply->abort();
	}
}
