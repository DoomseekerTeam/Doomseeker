//------------------------------------------------------------------------------
// www.h
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
#ifndef WADSEEKER_WWW_WWW_H
#define WADSEEKER_WWW_WWW_H

#include "dptr.h"

#include <QObject>

class QNetworkAccessManager;
class QString;
class QUrl;
class WebQuery;

/**
 * @brief Wadseeker's gateway to World-Wide-Web.
 *
 * Sends out URL queries and wraps them in a WebQuery tracking object.
 * It tracks all WebQuery objects that are still in-flight.
 */
class WWW : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(WWW)

public:
	WWW(QNetworkAccessManager *net);
	~WWW();

	/**
	 * @brief Start a QNetworkAccessManager::get() query for this URL.
	 */
	QSharedPointer<WebQuery> get(const QUrl &url);
	/**
	 * @brief How many queries are currently running?
	 */
	unsigned inFlightCount() const;
	/**
	 * @brief Is a query to the host denoted by this URL running?
	 */
	bool isHostInflight(const QUrl &url) const;
	/**
	 * @brief Set the User-Agent for all the outgoing queries.
	 */
	void setUserAgent(const QString &userAgent);

private:
	DPtr<WWW> d;
};

#endif
