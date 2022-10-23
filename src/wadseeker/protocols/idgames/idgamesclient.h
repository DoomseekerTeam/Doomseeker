//------------------------------------------------------------------------------
// idgamesclient.h
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
#ifndef id6233A6A9_0AAC_4407_82962B12627B23AE
#define id6233A6A9_0AAC_4407_82962B12627B23AE

#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>

class IdgamesReply;

/**
 * @brief Client for idgames archive public API.
 *
 * Ownership of all IdgamesReply objects is returned to the caller and
 * caller is responsible for destroying these objects.
 */
class IdgamesClient
{
public:
	static const QUrl DEFAULT_URL;

	IdgamesClient(QNetworkAccessManager *nam, const QUrl &baseUrl);
	~IdgamesClient();

	const QUrl &baseUrl() const;
	IdgamesReply *search(const QString &filename);
	void setBaseUrl(const QUrl &baseUrl);
	void setUserAgent(const QString &userAgent);

private:
	class PrivData;
	PrivData *d;
};


#endif
