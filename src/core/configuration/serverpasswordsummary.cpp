//------------------------------------------------------------------------------
// serverpasswordsummary.cpp
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
#include "serverpasswordsummary.h"

#include "serverapi/serversummary.h"

const QString ServerPasswordType::CONNECT = "Connect";
const QString ServerPasswordType::INGAME = "InGame";

DClass<ServerPasswordSummary>
{
public:
	ServerSummary serverSummary;
	// 'type' is a QString in case if we ever need to expand this
	// in plugins.
	// It's easier to keep unique values with this than with enums
	// considering that this needs to be stored in a config file.
	QString type;
};

DPointered(ServerPasswordSummary)

ServerPasswordSummary::ServerPasswordSummary()
{
}

ServerPasswordSummary::ServerPasswordSummary(const Server *server, const QString &type)
{
	d->serverSummary = ServerSummary(server);
	d->type = type;
}

ServerPasswordSummary::~ServerPasswordSummary()
{
}

ServerPasswordSummary ServerPasswordSummary::deserializeQVariant(const QVariant &v)
{
	QVariantMap map = v.toMap();
	ServerPasswordSummary obj;
	obj.setServerSummary(ServerSummary::deserializeQVariant(v));
	obj.setType(map["passwordType"].toString());
	return obj;
}

const QString &ServerPasswordSummary::address() const
{
	return serverSummary().address();
}

const QString &ServerPasswordSummary::game() const
{
	return serverSummary().game();
}

bool ServerPasswordSummary::isValid() const
{
	return serverSummary().isValid();
}

const QString &ServerPasswordSummary::name() const
{
	return serverSummary().name();
}

unsigned short ServerPasswordSummary::port() const
{
	return serverSummary().port();
}

QVariant ServerPasswordSummary::serializeQVariant() const
{
	QVariantMap var = serverSummary().serializeQVariant().toMap();
	var["passwordType"] = type();
	return var;
}

const ServerSummary &ServerPasswordSummary::serverSummary() const
{
	return d->serverSummary;
}

void ServerPasswordSummary::setServerSummary(const ServerSummary &val)
{
	d->serverSummary = val;
}

void ServerPasswordSummary::setType(const QString &val)
{
	d->type = val;
}

float ServerPasswordSummary::similarity(const ServerPasswordSummary &other) const
{
	if (!isValid() || !other.isValid())
		return 0.0f;

	if (typeWithCompatibility() != other.typeWithCompatibility())
		return 0.0f;

	return serverSummary().similarity(other.serverSummary());
}

QDateTime ServerPasswordSummary::time() const
{
	return serverSummary().time();
}

const QString &ServerPasswordSummary::type() const
{
	return d->type;
}

const QString &ServerPasswordSummary::typeWithCompatibility() const
{
	if (type().isEmpty())
		return ServerPasswordType::CONNECT;
	return d->type;
}
