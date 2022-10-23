//------------------------------------------------------------------------------
// serversummary.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "serversummary.h"

#include "serverapi/server.h"

ServerSummary::ServerSummary(const Server *server)
{
	setAddress(server->address().toString());
	setPort(server->port());
	setGame(server->engineName());
	setName(server->name());
}

ServerSummary ServerSummary::deserializeQVariant(const QVariant &v)
{
	QVariantMap m = v.toMap();
	ServerSummary o;
	o.setAddress(m["address"].toString());
	o.setGame(m["game"].toString());
	o.setName(m["name"].toString());
	o.setPort(m["port"].toUInt());
	o.setTime(m["time"].toDateTime());
	return o;
}

QVariant ServerSummary::serializeQVariant() const
{
	QVariantMap m;
	m.insert("address", d.address);
	m.insert("game", d.game);
	m.insert("name", d.name);
	m.insert("port", d.port);
	m.insert("time", d.time);
	return m;
}

float ServerSummary::similarity(const ServerSummary &other) const
{
	if (!isValid() || !other.isValid())
	{
		return 0.0f;
	}

	const float MAX_SIMILARITY = 3.0f;
	float similarity = 0.0f;
	if (address() == other.address() && port() == other.port() && game() == other.game())
	{
		similarity += 2.0f;
	}
	if (name() == other.name())
	{
		similarity += 1.0f;
	}
	return similarity / MAX_SIMILARITY;
}
