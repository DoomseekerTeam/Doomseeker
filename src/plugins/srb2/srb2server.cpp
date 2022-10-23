//------------------------------------------------------------------------------
// srb2server.cpp
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
#include "srb2server.h"

#include <climits>
#include <cstring>
#include <QBuffer>
#include <QDataStream>
#include <QFileInfo>
#include <QScopedPointer>

#include "srb2engineplugin.h"
#include "srb2gameclientrunner.h"
#include "srb2gameinfo.h"
#include <datastreamoperatorwrapper.h>
#include <plugins/engineplugin.h>
#include <serverapi/exefile.h>
#include <serverapi/message.h>
#include <serverapi/playerslist.h>
#include <strings.hpp>

using namespace Srb2ServerPacket;

struct Srb2ServerPacket::Header
{
	enum Type
	{
		SERVERINFO = 0xd,
		PLAYERINFO = 0xe
	};

	quint32 checksum;
	quint8 ack;
	quint8 ackReturn;
	quint8 packetType;
	quint8 padding;

	Header()
	{
		memset(this, 0, sizeof(Header));
	}
};

QDataStream &operator>>(QDataStream &stream, Header &header)
{
	stream >> header.checksum;
	stream >> header.ack;
	stream >> header.ackReturn;
	stream >> header.packetType;
	stream >> header.padding;
	return stream;
}

///////////////////////////////////////////////////////////////////////////

struct Srb2ServerPacket::FileNeeded
{
	friend QDataStream &::operator>>(QDataStream &stream, FileNeeded &info);

	enum Download
	{
		DownloadDisabled = 0,
		Downloadable = 0x10,
		TooLarge = 0x20,

		DownloadSum = DownloadDisabled | Downloadable | TooLarge
	};

	struct Status
	{
		bool required;
		Download download;

		Status() : required(true), download(DownloadDisabled)
		{
		}
	};

	Status status;
	quint32 size;
	QString name; // null-terminated, max is 128-bytes
	QString md5;

	FileNeeded() : size(0)
	{
	}

private:
	void readRawStatus(quint8 raw)
	{
		status.required = (raw & 0x01);
		status.download = static_cast<Download>(raw & DownloadSum);
	}
};

QDataStream &operator>>(QDataStream &stream, FileNeeded &info)
{
	quint8 status;
	stream >> status;
	info.readRawStatus(status);
	stream >> info.size;

	DataStreamOperatorWrapper reader(&stream);
	info.name = reader.readRawMaxUntilByte('\0', 128);
	info.md5 = reader.readRaw(16);

	return stream;
}

///////////////////////////////////////////////////////////////////////////

struct Srb2ServerPacket::ServerInfo
{
	friend QDataStream &::operator>>(QDataStream &stream, ServerInfo &info);

	struct D
	{
		quint8 num255 = 0; // (since 2.2.6)
		quint8 packetVersion; // (since 2.2.6)
		quint8 version;
		quint8 subversion;
		quint8 numPlayers;
		quint8 maxPlayers;
		union
		{
			quint8 gameType,
				refuseReason; // (since 2.2.6)
		};
		quint8 modifiedGame;
		quint8 cheatsEnabled;
		quint8 dedicated;
		quint8 numVerifiedPlayers;
		quint32 time;
		quint32 levelTime;
		quint8 act;
		quint8 isZone;

		D()
		{
			memset(this, 0, sizeof(D));
		}
	};

	D d;
	QString gameTypeString; // 24 bytes (since 2.2.6)
	QString application; // 16 bytes (since 2.2.6)
	QString name; // 32 bytes
	QString mapName; // 8 bytes
	QString mapTitle; // 33 bytes
	QByteArray mapMd5; // 16 bytes
	QList<FileNeeded> files; // as many as numModFiles.

	GameMode gameMode() const
	{
		QList<GameMode> modes = Srb2GameInfo::gameModes();
		if (d.num255 == 255)
		{
			for (auto &mode : modes)
			{
				if (gameTypeString == mode.name())
					return mode;
			}
		}
		else
		{
			if (d.gameType < modes.length())
				return modes[d.gameType];
		}
		return GameMode::mkUnknown();
	}
};

QDataStream &operator>>(QDataStream &stream, ServerInfo &info)
{
	DataStreamOperatorWrapper reader(&stream);

	stream >> info.d.version;
	if (info.d.version == 255)
	{
		info.d.num255 = info.d.version;
		stream >> info.d.packetVersion;
		info.application = Srb2::asciiOnly(reader.readRaw(16));
		stream >> info.d.version;
	}
	stream >> info.d.subversion;
	stream >> info.d.numPlayers;
	stream >> info.d.maxPlayers;
	stream >> info.d.refuseReason;
	if (info.d.num255 == 255)
		info.gameTypeString = Srb2::asciiOnly(reader.readRaw(24));
	stream >> info.d.modifiedGame;
	stream >> info.d.cheatsEnabled;
	stream >> info.d.dedicated;

	quint8 numModFiles = 0;
	stream >> numModFiles;
	if (info.d.num255 != 255)
		stream >> info.d.numVerifiedPlayers;
	stream >> info.d.time;
	stream >> info.d.levelTime;
	info.name = Srb2::asciiOnly(reader.readRaw(32));
	info.mapName = Srb2::asciiOnly(reader.readRaw(8));
	info.mapTitle = Srb2::asciiOnly(reader.readRaw(33));
	info.mapMd5 = reader.readRaw(16);
	stream >> info.d.act;
	stream >> info.d.isZone;
	for (int i = 0; i < numModFiles; ++i)
	{
		FileNeeded file;
		stream >> file;
		info.files << file;
	}
	return stream;
}

///////////////////////////////////////////////////////////////////////////

struct Srb2ServerPacket::PlayerInfo
{
	struct Player
	{
		struct D
		{
			quint8 node;
			quint8 team;
			quint8 skin; // Skin or class? Docs say "0 == This player is using Sonic"
			quint8 data; // Has player color. Let's ignore this for now.
			quint32 score;
			quint16 secondsInServer;

			D()
			{
				memset(this, 0, sizeof(D));
			}
		};

		static const int SIZE = 36;

		D d;
		QString name;
		QHostAddress address; // Yes, really!

		bool isEmptyNode() const
		{
			return d.node == 0xff;
		}

		bool isSpectator() const
		{
			return d.team == 0xff;
		}

		::Player::PlayerTeam team() const
		{
			switch (d.team)
			{
			case 0:
			default:
				return ::Player::TEAM_NONE;
			case 1:
				return ::Player::TEAM_RED;
			case 2:
				return ::Player::TEAM_BLUE;
			}
		}
	};

	QList<Player> players;
};

QDataStream &operator>>(QDataStream &stream, PlayerInfo &info)
{
	DataStreamOperatorWrapper reader(&stream);
	for (int i = 0; i < 32; ++i)
	{
		PlayerInfo::Player player;
		stream >> player.d.node;
		if (player.isEmptyNode())
		{
			reader.readRaw(PlayerInfo::Player::SIZE - 1);
			continue;
		}
		player.name = Srb2::asciiOnly(reader.readRaw(22));

		quint32 ip = 0x7f000001;
		stream.setByteOrder(QDataStream::BigEndian);
		stream >> ip;
		stream.setByteOrder(QDataStream::LittleEndian);
		player.address = QHostAddress(ip);

		stream >> player.d.team;
		stream >> player.d.skin;
		stream >> player.d.data;
		stream >> player.d.score;
		stream >> player.d.secondsInServer;

		info.players << player;
	}
	return stream;
}

///////////////////////////////////////////////////////////////////////////

class Srb2Server::PrivData
{
public:
	QScopedPointer<Header> header;
	QScopedPointer<ServerInfo> serverInfo;
	QScopedPointer<PlayerInfo> playerInfo;
};

Srb2Server::Srb2Server(const QHostAddress &address, unsigned short port)
	: Server(address, port)
{
	d = new PrivData();
	set_customDetails(&Srb2Server::customDetails);
	set_readRequest(&Srb2Server::readRequest);
	set_createSendRequest(&Srb2Server::createSendRequest);
}

Srb2Server::~Srb2Server()
{
	delete d;
}

QString Srb2Server::customDetails()
{
	return "";
}

GameClientRunner *Srb2Server::gameRunner()
{
	return new Srb2GameClientRunner(
		self().toStrongRef().staticCast<Srb2Server>());
}

EnginePlugin *Srb2Server::plugin() const
{
	return Srb2EnginePlugin::staticInstance();
}

Server::Response Srb2Server::readRequest(const QByteArray &data)
{
	QBuffer buffer;
	buffer.setData(data);
	buffer.open(QIODevice::ReadOnly);
	QDataStream stream(&buffer);
	stream.setByteOrder(QDataStream::LittleEndian);

	if (buffer.bytesAvailable() > 0 && d->header.isNull())
	{
		d->header.reset(new Header());
		stream >> *d->header;
	}

	if (buffer.bytesAvailable() > 0 && !d->header.isNull())
	{
		if (d->header->packetType == Header::SERVERINFO)
		{
			d->serverInfo.reset(new ServerInfo());
			stream >> *d->serverInfo;
		}
		else if (d->header->packetType == Header::PLAYERINFO)
		{
			d->playerInfo.reset(new PlayerInfo());
			stream >> *d->playerInfo;
		}
		d->header.reset(nullptr);
	}

	if (!d->serverInfo.isNull() && !d->playerInfo.isNull())
		return processInfoPackets();
	else
		return RESPONSE_PENDING;
}

Server::Response Srb2Server::processInfoPackets()
{
	Response response = processServerInfo(*d->serverInfo);
	if (response != RESPONSE_GOOD)
		return response;
	return processPlayerInfo(*d->playerInfo);
}

Server::Response Srb2Server::processServerInfo(const ServerInfo &info)
{
	if (info.application != "SRB2" && !info.application.isEmpty())
		return RESPONSE_BAD;
	setName(info.name);
	if (gameVersion().isEmpty())
	{
		// A more flexible version string is provided by master server.
		// If this is a custom server we don't have master server info and
		// need to process the version/subversion data provided
		// by the server itself.
		QString version = QString::number(info.d.version);
		QString mainversion = version.left(1);
		QString midversion = QString::number(version.mid(1).toInt());
		QString subversion = QString::number(info.d.subversion);
		setGameVersion(QString("%1.%2.%3").arg(mainversion, midversion, subversion));
	}
	setMaxPlayers(info.d.maxPlayers);
	setMaxClients(info.d.maxPlayers);
	setMap(info.mapName);
	setGameMode(info.gameMode());

	setIwad(!info.files.empty() ? info.files.first().name : "");
	clearWads();
	bool first = true;
	for (const FileNeeded &file : info.files)
	{
		if (first)
		{
			// First is IWAD.
			first = false;
			continue;
		}
		addWad(PWad(file.name, !file.status.required));
	}
	return RESPONSE_GOOD;
}

Server::Response Srb2Server::processPlayerInfo(const PlayerInfo &info)
{
	clearPlayersList();
	for (const PlayerInfo::Player &srbPlayer : info.players)
	{
		addPlayer(Player(srbPlayer.name, srbPlayer.d.score, USHRT_MAX,
			srbPlayer.team(), srbPlayer.isSpectator()));
	}
	return RESPONSE_GOOD;
}

QByteArray Srb2Server::createSendRequest()
{
	// Reset header if it lingers from a previous refresh.
	d->header.reset(nullptr);
	// Also reset the old information.
	d->serverInfo.reset(nullptr);
	d->playerInfo.reset(nullptr);

	/*
	References:
	- https://wiki.srb2.org/wiki/SRB2_network_documentation
	- https://github.com/STJr/SRB2/blob/SRB2_release_2.1.14/src/d_net.c#L689
	 */
	const int length = 13;
	const unsigned char challenge[length] = {
		// Header
		0x8b, 0x45, 0x23, 0x01, // Checksum
		0x00, 0x00, 0x0c, 0x00, // ACK, Ret. ACK, Packet Type, Padding
		// Payload
		0x00, // Version
		0x00, 0x00, 0x00, 0x00 // Time
	};
	return QByteArray(reinterpret_cast<const char *>(challenge), length);
}

void Srb2Server::setGameVersion(const QString &version)
{
	Server::setGameVersion(version);
}

PathFinder Srb2Server::wadPathFinder()
{
	PathFinder pathFinder = Server::wadPathFinder();
	QScopedPointer<ExeFile> exe(clientExe());
	Message message;
	QString exePath = exe->pathToExe(message);
	if (!exePath.isNull())
	{
		QFileInfo fileInfo(exePath);
		QString dirPath = Strings::combinePaths(fileInfo.absolutePath(), "DOWNLOAD");
		pathFinder.addSearchDir(dirPath);
	}
	return pathFinder;
}
