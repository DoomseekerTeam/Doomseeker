//------------------------------------------------------------------------------
// webquery.h
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
#ifndef WADSEEKER_WWW_WEBQUERY_H
#define WADSEEKER_WWW_WEBQUERY_H

#include "dptr.h"
#include "wadseekermessagetype.h"

#include <QObject>
#include <QNetworkReply>
#include <QSharedPointer>
#include <QUrl>

enum class WebResourceType
{
	Binary,
	Html,
	Unknown,
};

/**
 * Tracking object for a single URL query.
 *
 * It functions as a wrapper on a previously created `QNetworkReply` and
 * its corresponding `QNetworkRequest`.
 *
 * Provides a Wadseeker-centric interface to `QNetworkReply` and
 * `QNetworkRequest` combined. It tries to recognize the `WebResourceType`
 * basing on the metadata and emits `foundType()` when it becomes known.
 *
 * Sets a timeout on the query. There's a shorter timeout for the
 * initial connection and a longer timeout if an ongoing communication
 * suddenly stops. A timed out connection will return
 * `QNetworkReply::TimeoutError` from `error()`.
 *
 * Since Wadseeker doesn't ever upload anything, WebQuery doesn't care
 * about `QNetworkReply::uploadProgress()` at all.
 */
class WebQuery : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(WebQuery)

public:
	/**
	 * Default timeout for connection establishment.
	 *
	 * This is the timeout between the construction of WebQuery and
	 * a first reception of `downloadProgress()` signal.
	 */
	static const unsigned DEFAULT_CONNECTION_TIMEOUT_MSECS = 15 * 1000;

	/**
	 * Default timeout for data transfer.
	 *
	 * This is the timeout between the subsequent receptions of
	 * `downloadProgress()` signals.
	 */
	static const unsigned DEFAULT_PROGRESS_TIMEOUT_MSECS = 60 * 1000;

	WebQuery(const QNetworkRequest &request, QNetworkReply *reply);
	~WebQuery();

	void abort()
	{
		this->m_reply->abort();
	}

	QNetworkReply::NetworkError error() const
	{
		return this->m_error;
	}

	QString errorString() const
	{
		return this->m_errorString;
	}

	/**
	 * Is the query being redirected to another URL?
	 *
	 * @see redirectUrl()
	 */
	bool isRedirected() const;

	/**
	 * The redirect URL, if any, already resolved to the base url() if relative.
	 */
	QUrl redirectUrl() const;

	QNetworkReply &reply()
	{
		return *this->m_reply;
	}

	const QNetworkRequest &request()
	{
		return this->m_request;
	}

	QUrl url() const
	{
		return this->m_reply->url();
	}

	WebResourceType resourceType() const
	{
		return this->m_resourceType;
	}

signals:
	void errorOccurred();
	void foundType(WebResourceType type);
	void finished();
	void progress(qint64 bytesReceived, qint64 bytesTotal);

private:
	DPtr<WebQuery> d;

	enum State
	{
		Running,
		Recognized,
		Redirected,
		TimedOut,
		Finished,
	};

	State m_state;
	QNetworkReply *m_reply;
	QNetworkRequest m_request;
	WebResourceType m_resourceType;

	// Because we need to set our own error in some cases,
	// and QNetworkReply::setError() is protected.
	QNetworkReply::NetworkError m_error;
	QString m_errorString;

	void copyErrorOver();

private slots:
	void onError(QNetworkReply::NetworkError code);
	void onFinished();
	void onMetaDataChanged();
	void onProgress();
	void onSslErrors(const QList<QSslError> &errors);
	void onTimeout();
};

#endif
