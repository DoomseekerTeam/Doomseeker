//------------------------------------------------------------------------------
// networkreplysignalwrapper.cpp
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
#include "networkreplysignalwrapper.h"

#include "protocols/networkreply.h"

NetworkReplySignalWrapper::NetworkReplySignalWrapper(NetworkReply *pReply)
{
	this->pReply = pReply;
	QNetworkReply *qReply = pReply->reply;

	this->connect(qReply, SIGNAL(downloadProgress(qint64,qint64)),
		SLOT(downloadProgressSlot(qint64,qint64)));
	this->connect(qReply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(errorSlot(QNetworkReply::NetworkError)));
	this->connect(qReply, SIGNAL(finished()),
		SLOT(finishedSlot()));
	this->connect(qReply, SIGNAL(metaDataChanged()),
		SLOT(metaDataChangedSlot()));
	this->connect(qReply, SIGNAL(sslErrors(const QList<QSslError>&)),
		SLOT(sslErrorsSlot(const QList<QSslError>&)));
	this->connect(qReply, SIGNAL(uploadProgress(qint64,qint64)),
		SLOT(uploadProgressSlot(qint64,qint64)));
}

void NetworkReplySignalWrapper::downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal)
{
	emit downloadProgress(pReply, bytesReceived, bytesTotal);
}

void NetworkReplySignalWrapper::errorSlot(QNetworkReply::NetworkError code)
{
	emit error(pReply, code);
}

void NetworkReplySignalWrapper::finishedSlot()
{
	emit finished(pReply);
}

void NetworkReplySignalWrapper::metaDataChangedSlot()
{
	emit metaDataChanged(pReply);
}

void NetworkReplySignalWrapper::sslErrorsSlot(const QList<QSslError> &errors)
{
	emit sslErrors(pReply, errors);
}

void NetworkReplySignalWrapper::uploadProgressSlot(qint64 bytesSent, qint64 bytesTotal)
{
	emit uploadProgress(pReply, bytesSent, bytesTotal);
}
