//------------------------------------------------------------------------------
// networkreply.cpp
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
#include "networkreply.h"

#include "protocols/networkreplysignalwrapper.h"
#include "protocols/networkreplytimeouter.h"

NetworkReply::NetworkReply(const QNetworkRequest &request, QNetworkReply *reply)
{
	this->reply = reply;
	this->request_ = request;

	if (reply != nullptr)
	{
		signalWrapper = new NetworkReplySignalWrapper(this);
		timeouter = new NetworkReplyTimeouter(reply);
	}
	else
	{
		signalWrapper = nullptr;
		timeouter = nullptr;
	}
}

NetworkReply::~NetworkReply()
{
	if (reply != nullptr)
	{
		delete signalWrapper;
		delete timeouter;
		delete reply;
	}
}

void NetworkReply::abort()
{
	reply->abort();
}

QVariant NetworkReply::attribute(QNetworkRequest::Attribute code) const
{
	return reply->attribute(code);
}

qint64 NetworkReply::bytesAvailable() const
{
	return reply->bytesAvailable();
}

void NetworkReply::deleteMembersLater()
{
	if (reply != nullptr)
	{
		delete signalWrapper;
		delete timeouter;
		reply->abort();
		reply->deleteLater();

		reply = nullptr;
	}
}

QNetworkReply::NetworkError NetworkReply::error() const
{
	return reply->error();
}

QString NetworkReply::errorString() const
{
	return reply->errorString();
}

QVariant NetworkReply::header(QNetworkRequest::KnownHeaders header) const
{
	return reply->header(header);
}

QByteArray NetworkReply::rawHeader(const QByteArray &headerName) const
{
	return reply->rawHeader(headerName);
}

QList<QByteArray> NetworkReply::rawHeaderList() const
{
	return reply->rawHeaderList();
}

QByteArray NetworkReply::readAll()
{
	return reply->readAll();
}

const QNetworkRequest &NetworkReply::request() const
{
	return request_;
}

void NetworkReply::setProgressTimeout(unsigned timeoutMsecs)
{
	if (timeouter != nullptr)
	{
		timeouter->setProgressTimeout(timeoutMsecs);
	}
}

void NetworkReply::startConnectionTimeoutTimer(unsigned timeoutMsecs)
{
	if (timeouter != nullptr)
	{
		timeouter->startConnectionTimeoutTimer(timeoutMsecs);
	}
}

QUrl NetworkReply::url() const
{
	return reply->url();
}

bool NetworkReply::operator==(const NetworkReply &other) const
{
	return *this == other.reply;
}

bool NetworkReply::operator==(const QNetworkReply *reply) const
{
	return this->reply == reply;
}
