//------------------------------------------------------------------------------
// odamexmasterclient.cpp
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
// Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#include "chocolatedoomengineplugin.h"
#include "chocolatedoommasterclient.h"
#include "chocolatedoomserver.h"
#include "global.h"

#define NET_MASTER_PACKET_TYPE_QUERY 2
#define NET_MASTER_PACKET_TYPE_QUERY_RESPONSE 3

ChocolateDoomMasterClient::ChocolateDoomMasterClient() : MasterClient()
{
}

QByteArray ChocolateDoomMasterClient::createServerListRequest()
{
	char challenge[2] = {0, NET_MASTER_PACKET_TYPE_QUERY};
	return QByteArray(challenge, 2);
}

const EnginePlugin *ChocolateDoomMasterClient::plugin() const
{
	return ChocolateDoomEnginePlugin::staticInstance();
}

MasterClient::Response ChocolateDoomMasterClient::readMasterResponse(const QByteArray &data)
{
	// Decompress the response.
	const char *in = data.data();

	// Check the response code
	if (in[1] != NET_MASTER_PACKET_TYPE_QUERY_RESPONSE)
		return RESPONSE_BAD;

	// Make sure we have an empty list.
	emptyServerList();

	int pos = 2;
	while (pos < data.size())
	{
		QString address(&in[pos]);
		pos += address.length() + 1;
		QStringList ip = address.split(":");
		ChocolateDoomServer *server = new ChocolateDoomServer(QHostAddress(ip[0]), ip[1].toUShort());
		registerNewServer(ServerPtr(server));
	}

	emit listUpdated();
	return RESPONSE_GOOD;
}
