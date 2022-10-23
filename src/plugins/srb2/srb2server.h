//------------------------------------------------------------------------------
// srb2server.h
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef SRB2SERVER_H
#define SRB2SERVER_H

#include <QDataStream>
#include <QMap>
#include <serverapi/server.h>

class GameHost;
class GameClientRunner;
class EnginePlugin;

namespace Srb2ServerPacket
{
struct Header;
struct ServerInfo; /// PT_SERVERINFO
struct FileNeeded; /// PT_SERVERINFO
struct PlayerInfo; /// PT_PLAYERINFO
}

QDataStream &operator>>(QDataStream &stream, Srb2ServerPacket::Header &header);
QDataStream &operator>>(QDataStream &stream, Srb2ServerPacket::ServerInfo &info);
QDataStream &operator>>(QDataStream &stream, Srb2ServerPacket::FileNeeded &info);
QDataStream &operator>>(QDataStream &stream, Srb2ServerPacket::PlayerInfo &info);

class Srb2Server : public Server
{
	Q_OBJECT

public:
	Srb2Server(const QHostAddress &address, unsigned short port);
	~Srb2Server() override;

	QString customDetails();

	GameClientRunner *gameRunner() override;

	EnginePlugin *plugin() const override;

	void setGameVersion(const QString &version);

protected:
	Response readRequest(const QByteArray &data);
	QByteArray createSendRequest();
	PathFinder wadPathFinder() override;

private:
	class PrivData;
	PrivData *d;

	Response processInfoPackets();
	Response processServerInfo(const Srb2ServerPacket::ServerInfo &info);
	Response processPlayerInfo(const Srb2ServerPacket::PlayerInfo &info);
};

#endif
