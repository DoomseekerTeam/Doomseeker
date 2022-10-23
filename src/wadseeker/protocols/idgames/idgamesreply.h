//------------------------------------------------------------------------------
// idgamesreply.h
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
#ifndef idBBCCEAFB_FDA6_4F7F_9EBCDED64CF25C0C
#define idBBCCEAFB_FDA6_4F7F_9EBCDED64CF25C0C

#include <QNetworkReply>

class IdgamesResult;

/**
 * @brief Reply akin to QNetworkReply created by IdgamesClient.
 */
class IdgamesReply : public QObject
{
	Q_OBJECT

	friend class IdgamesClient;

public:
	~IdgamesReply() override;

	void abort();
	IdgamesResult result() const;

signals:
	void finished();

private:
	class PrivData;
	PrivData *d;

	IdgamesReply(const QNetworkRequest &request, QNetworkReply *reply);
	void finish(const IdgamesResult &result);
	void redirect(QUrl redirectUrl);
	void setReply(QNetworkReply *reply);

private slots:
	void onNetworkFinished();
};


#endif // header
