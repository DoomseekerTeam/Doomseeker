//------------------------------------------------------------------------------
// vavoommasterclient.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#include "global.h"
#include "vavoomengineplugin.h"
#include "vavoommasterclient.h"
#include "vavoomserver.h"

#define MCREQ_LIST  3
#define MCREP_LIST  1

VavoomMasterClient::VavoomMasterClient() : MasterClient()
{
}

const EnginePlugin *VavoomMasterClient::plugin() const
{
	return VavoomEnginePlugin::staticInstance();
}

QByteArray VavoomMasterClient::createServerListRequest()
{
	char challenge[1];
	WRITEINT8(challenge, MCREQ_LIST);
	return QByteArray(challenge, 1);
}

MasterClient::Response VavoomMasterClient::readMasterResponse(const QByteArray &data)
{
	// Decompress the response.
	const char *in = data.data();

	// Check the response code
	int response = READINT8(&in[0]);

	if (response != MCREP_LIST)
		return RESPONSE_BAD;

	// Make sure we have an empty list.
	emptyServerList();

	int numServers = (data.size() - 1) / 6;
	int pos = 1;
	for (; numServers > 0; numServers--)
	{
		// This might be able to be simplified a little bit...
		QString ip = QString("%1.%2.%3.%4").
			arg(static_cast<quint8>(in[pos]), 1, 10, QChar('0')).arg(static_cast<quint8>(in[pos + 1]), 1, 10, QChar('0')).arg(static_cast<quint8>(in[pos + 2]), 1, 10, QChar('0')).arg(static_cast<quint8>(in[pos + 3]), 1, 10, QChar('0'));
		VavoomServer *server = new VavoomServer(QHostAddress(ip), READBIGINT16(&in[pos + 4]));
		registerNewServer(ServerPtr(server));
		pos += 6;
	}

	emit listUpdated();
	return RESPONSE_GOOD;
}
