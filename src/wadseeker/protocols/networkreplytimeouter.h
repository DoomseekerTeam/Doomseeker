//------------------------------------------------------------------------------
// networkreplytimeouter.h
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
#ifndef __NETWORKREPLYTIMEOUTER_H__
#define __NETWORKREPLYTIMEOUTER_H__

#include <QNetworkReply>
#include <QTimer>

#include "../wadseekerexportinfo.h"

/**
 * @brief Wraps the QNetworkReply so timeouts are registered.
 *
 * QNetworkReply::abort() is called when a timeout is registered.
 *
 * Will not delete the underlying QNetworkReply object upon destruction.
 */
class WADSEEKER_API NetworkReplyTimeouter : public QObject
{
	Q_OBJECT

public:
	NetworkReplyTimeouter(QNetworkReply *pReply);
	virtual ~NetworkReplyTimeouter() override;

	QNetworkReply &reply()
	{
		return *pReply;
	}

	/**
	 * @brief Sets timeout that occurs when progress stops.
	 *
	 * This is the timeout between the receptions of either
	 * downloadProgress() or uploadProgress() signals.
	 *
	 * @param timeoutMsecs
	 *      Time in milliseconds after which the timeout will be
	 *      registered and the underlying QNetworkReply will be
	 *      aborted. Also generates a QNetworkReply::TimeoutError error.
	 *      Set to 0 to disable timeout.
	 */
	void setProgressTimeout(unsigned timeoutMsecs);

	/**
	 * @brief Starts the timeout for connection establishment.
	 *
	 * This is the timeout between the call to
	 * startConnectionTimeoutTimer() and first reception of
	 * downloadProgress() or uploadProgress() signals.
	 *
	 * @param timeoutMsecs
	 *      Time in milliseconds after which the timeout will be
	 *      registered and the underlying QNetworkReply will be
	 *      aborted. Also generates a QNetworkReply::TimeoutError error.
	 */
	void startConnectionTimeoutTimer(unsigned timeoutMsecs);

signals:
	/**
	 * @brief This signal is connected directly to QNetworkReply and
	 *        shouldn't really be used.
	 */
	void error(QNetworkReply::NetworkError code);

private:
	/**
	 * @brief Set to true when either downloadProgress() or
	 *        uploadProgress() signal is captured.
	 */
	bool bIsProgressing;

	/**
	 * @brief Timeout before the connection is made.
	 */
	QTimer *pConnectionTimeoutTimer;

	/**
	 * @brief Timeout when download progress stops.
	 */
	QTimer *pProgressTimeoutTimer;

	QNetworkReply *pReply;

	unsigned progressTimeout;

	void restartProgressTimeoutIfAllowed();
	void stopTimerIfNotNull(QTimer *pTimer);

private slots:
	void progressRegistered();
	void finishedSlot();
	void timeout();
};

#endif
