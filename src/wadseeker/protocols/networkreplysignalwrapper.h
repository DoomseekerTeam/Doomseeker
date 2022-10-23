//------------------------------------------------------------------------------
// networkreplysignalwrapper.h
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
#ifndef __NETWORKREPLYSIGNALWRAPPER_H__
#define __NETWORKREPLYSIGNALWRAPPER_H__

#include <QNetworkReply>

class NetworkReply;

/**
 * @brief Wraps the QNetworkReply so signals carry over pointer to the
 *        calling object
 *
 * Will not delete the underlying QNetworkReply object upon destruction.
 */
class NetworkReplySignalWrapper : public QObject
{
	Q_OBJECT

public:
	NetworkReplySignalWrapper(NetworkReply *pReply);

	NetworkReply &reply()
	{
		return *pReply;
	}

signals:
	void downloadProgress(NetworkReply *pCaller, qint64 bytesReceived, qint64 bytesTotal);
	void error(NetworkReply *pCaller, QNetworkReply::NetworkError code);
	void finished(NetworkReply *pCaller);
	void metaDataChanged(NetworkReply *pCaller);
	void sslErrors(NetworkReply *pCaller, const QList<QSslError> &errors);
	void uploadProgress(NetworkReply *pCaller, qint64 bytesSent, qint64 bytesTotal);

private:
	NetworkReply *pReply;

private slots:
	void downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal);
	void errorSlot(QNetworkReply::NetworkError code);
	void finishedSlot();
	void metaDataChangedSlot();
	void sslErrorsSlot(const QList<QSslError> &errors);
	void uploadProgressSlot(qint64 bytesSent, qint64 bytesTotal);
};

#endif
