//------------------------------------------------------------------------------
// networkreplytimeouter.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "networkreplytimeouter.h"


NetworkReplyTimeouter::NetworkReplyTimeouter(QNetworkReply *pReply)
{
	this->bIsProgressing = false;
	this->pReply = pReply;
	this->pConnectionTimeoutTimer = nullptr;
	this->pProgressTimeoutTimer = nullptr;
	this->progressTimeout = 0;

	this->connect(pReply,
		SIGNAL(downloadProgress(qint64,qint64)),
		SLOT(progressRegistered()));
	this->connect(pReply,
		SIGNAL(uploadProgress(qint64,qint64)),
		SLOT(progressRegistered()));


	// This little monster here allows us to emit custom generated
	// signals from QNetworkReply!
	pReply->connect(this,
		SIGNAL(error(QNetworkReply::NetworkError)),
		SIGNAL(error(QNetworkReply::NetworkError)));
}

NetworkReplyTimeouter::~NetworkReplyTimeouter()
{
	if (pConnectionTimeoutTimer != nullptr)
	{
		delete pConnectionTimeoutTimer;
	}

	if (pProgressTimeoutTimer != nullptr)
	{
		delete pProgressTimeoutTimer;
	}
}

void NetworkReplyTimeouter::finishedSlot()
{
	stopTimerIfNotNull(pConnectionTimeoutTimer);
	stopTimerIfNotNull(pProgressTimeoutTimer);
}

void NetworkReplyTimeouter::progressRegistered()
{
	bIsProgressing = true;
	stopTimerIfNotNull(pConnectionTimeoutTimer);
	restartProgressTimeoutIfAllowed();
}

void NetworkReplyTimeouter::restartProgressTimeoutIfAllowed()
{
	if (progressTimeout != 0 && bIsProgressing)
	{
		if (pProgressTimeoutTimer == nullptr)
		{
			pProgressTimeoutTimer = new QTimer();
			pProgressTimeoutTimer->setSingleShot(true);
			this->connect(pProgressTimeoutTimer,
				SIGNAL(timeout()),
				SLOT(timeout()));
		}

		pProgressTimeoutTimer->start(progressTimeout);
	}
}

void NetworkReplyTimeouter::setProgressTimeout(unsigned timeoutMsecs)
{
	stopTimerIfNotNull(pProgressTimeoutTimer);
	progressTimeout = timeoutMsecs;
	restartProgressTimeoutIfAllowed();
}

void NetworkReplyTimeouter::startConnectionTimeoutTimer(unsigned timeoutMsecs)
{
	bIsProgressing = true;
	if (pConnectionTimeoutTimer == nullptr)
	{
		pConnectionTimeoutTimer = new QTimer();
		pConnectionTimeoutTimer->setSingleShot(true);
		this->connect(pConnectionTimeoutTimer,
			SIGNAL(timeout()),
			SLOT(timeout()));
	}

	pConnectionTimeoutTimer->start(timeoutMsecs);
}

void NetworkReplyTimeouter::stopTimerIfNotNull(QTimer *pTimer)
{
	if (pTimer != nullptr)
	{
		pTimer->stop();
	}
}

void NetworkReplyTimeouter::timeout()
{
	emit error(QNetworkReply::TimeoutError);
	pReply->abort();
}
