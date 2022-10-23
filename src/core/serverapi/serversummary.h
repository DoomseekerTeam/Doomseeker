//------------------------------------------------------------------------------
// serversummary.h
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
#ifndef idAA369F27_B8AD_4120_8889175B56406459
#define idAA369F27_B8AD_4120_8889175B56406459

#include <QDateTime>
#include <QString>
#include <QVariant>

class Server;

class ServerSummary
{
public:
	static ServerSummary deserializeQVariant(const QVariant &var);

	ServerSummary()
	{
		d.port = 0;
	}

	ServerSummary(const Server *server);

	const QString &address() const
	{
		return d.address;
	}

	const QString &game() const
	{
		return d.game;
	}

	bool isValid() const
	{
		return !d.address.isEmpty() && d.port != 0;
	}

	const QString &name() const
	{
		return d.name;
	}

	unsigned short port() const
	{
		return d.port;
	}

	void setAddress(const QString &v)
	{
		d.address = v;
	}

	void setGame(const QString &v)
	{
		d.game = v;
	}

	void setName(const QString &v)
	{
		d.name = v;
	}

	void setPort(unsigned short v)
	{
		d.port = v;
	}

	void setTime(const QDateTime &v)
	{
		d.time = v;
	}

	QVariant serializeQVariant() const;
	/**
	 * @brief Similarity to the 'other' server; between 0.0 and 1.0.
	 *
	 * Similarity is calculated basing on address, game, name
	 * and port. Some of these values have more importance than
	 * others, and some require other values to be equal.
	 *
	 * Both servers must be valid.
	 */
	float similarity(const ServerSummary &other) const;

	QDateTime time() const
	{
		return d.time;
	}

private:
	class PrivData
	{
	public:
		QString address;
		QString game;
		QString name;
		unsigned short port;
		QDateTime time;
	};

	PrivData d;
};

#endif
