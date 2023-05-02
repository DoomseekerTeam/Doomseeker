//------------------------------------------------------------------------------
// serverpassword.cpp
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
#include "serverpassword.h"

class ServerSimilarity
{
public:
	ServerPasswordSummary server;
	float similarity;

	ServerSimilarity(const ServerPasswordSummary &server, float similarity)
	{
		this->server = server;
		this->similarity = similarity;
	}

	bool operator<(const ServerSimilarity &other) const
	{
		return similarity < other.similarity;
	}
};

void ServerPassword::addServer(const ServerPasswordSummary &v)
{
	if (v.isValid())
	{
		removeServer(v.game(), v.address(), v.port());
		d.servers.append(v);
	}
}

ServerPassword ServerPassword::deserializeQVariant(const QVariant &v)
{
	QVariantMap m = v.toMap();
	ServerPassword o;
	o.setPhrase(m["phrase"].toString());
	QVariantList variantServers = m["servers"].toList();
	for (QVariant server : variantServers)
	{
		o.addServer(ServerPasswordSummary::deserializeQVariant(server));
	}
	return o;
}

QString ServerPassword::lastGame() const
{
	return lastServer().game();
}

ServerPasswordSummary ServerPassword::lastServer() const
{
	ServerPasswordSummary lastServer;
	for (const ServerPasswordSummary &s : d.servers)
	{
		if (!lastServer.isValid())
		{
			lastServer = s;
			continue;
		}

		if (s.isValid() && s.time() > lastServer.time())
			lastServer = s;
	}
	return lastServer;
}

QString ServerPassword::lastServerName() const
{
	return lastServer().name();
}

QDateTime ServerPassword::lastTime() const
{
	return lastServer().time();
}

ServerPasswordSummary ServerPassword::mostSimilarServer(const ServerPasswordSummary &other, float *outSimilarity) const
{
	QList<ServerSimilarity> similarities;
	for (const ServerPasswordSummary &candidate : d.servers)
	{
		if (candidate.similarity(other) > 0.0f)
			similarities << ServerSimilarity(candidate, candidate.similarity(other));
	}
	if (!similarities.empty())
	{
		std::sort(similarities.begin(), similarities.end());
		if (outSimilarity != nullptr)
			*outSimilarity = similarities.last().similarity;
		return similarities.last().server;
	}
	else
	{
		if (outSimilarity != nullptr)
			*outSimilarity = 0.0f;
		return ServerPasswordSummary(); // Invalid value.
	}
}

void ServerPassword::removeServer(const QString &game, const QString &address, unsigned short port)
{
	QMutableListIterator<ServerPasswordSummary> it(d.servers);
	while (it.hasNext())
	{
		ServerPasswordSummary server = it.next();
		if (server.game() == game && server.address() == address && server.port() == port)
			it.remove();
	}
}

QVariant ServerPassword::serializeQVariant() const
{
	QVariantMap m;
	m.insert("phrase", d.phrase);
	QVariantList variantServers;
	for (const ServerPasswordSummary &s : d.servers)
	{
		variantServers << s.serializeQVariant();
	}
	m.insert("servers", variantServers);
	return m;
}
