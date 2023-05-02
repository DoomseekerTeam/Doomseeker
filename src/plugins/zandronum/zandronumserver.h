//------------------------------------------------------------------------------
// zandronumserver.h
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net> (skulltagserver.h)
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUMSERVER_H
#define DOOMSEEKER_PLUGIN_ZANDRONUMSERVER_H

#include <QColor>
#include <QString>

#include "serverapi/server.h"

#define NUM_ZANDRONUM_GAME_MODES 16
#define NUM_ZANDRONUM_GAME_MODIFIERS 2
#define ST_MAX_TEAMS 4U

class GameClientRunner;
class RConProtocol;
class ZandronumServer;

/**
 * Compares versions of Zandronum.
 */
class ZandronumVersion
{
public:
	ZandronumVersion(QString version);

	unsigned short int majorVersion() const { return major; }
	unsigned short int minorVersion() const { return minor; }
	unsigned short int revisionVersion() const { return revision; }
	unsigned int hgVersionDate() const { return hgRevisionDate; }
	unsigned short int hgVersionTime() const { return hgRevisionTime; }

	bool operator> (const ZandronumVersion &other) const;

protected:
	static const QRegExp versionExpression;
	QString version;

	unsigned short int major;
	unsigned short int minor;
	unsigned short int revision;
	unsigned short int build;
	QString tag;
	unsigned int hgRevisionDate;
	unsigned short int hgRevisionTime;
};

class TeamInfo
{
public:
	TeamInfo(QString name = QObject::tr("<< Unknown >>"), QColor color = QColor(0, 0, 0), unsigned int score = 0);

	const QString &name() const { return teamName; }
	const QColor &color() const { return teamColor; }
	unsigned int score() const { return teamScore; }

protected:
	void setName(const QString &name) { teamName = name; }
	void setColor(const QColor &color) { teamColor = color; }
	void setScore(int score) { teamScore = score; }

	friend class ZandronumServer;

private:
	QString teamName;
	QColor teamColor;
	unsigned int teamScore;
};

class ZandronumServer : public Server
{
	Q_OBJECT

public:


	enum ZandronumQueryFlags
	{
		SQF_NAME = 0x00000001,
		SQF_URL = 0x00000002,
		SQF_EMAIL = 0x00000004,
		SQF_MAPNAME = 0x00000008,
		SQF_MAXCLIENTS = 0x00000010,
		SQF_MAXPLAYERS = 0x00000020,
		SQF_PWADS = 0x00000040,
		SQF_GAMETYPE = 0x00000080,
		SQF_GAMENAME = 0x00000100,
		SQF_IWAD = 0x00000200,
		SQF_FORCEPASSWORD = 0x00000400,
		SQF_FORCEJOINPASSWORD = 0x00000800,
		SQF_GAMESKILL = 0x00001000,
		SQF_BOTSKILL = 0x00002000,
		SQF_DMFLAGS = 0x00004000, // Deprecated
		SQF_LIMITS = 0x00010000,
		SQF_TEAMDAMAGE = 0x00020000,
		SQF_TEAMSCORES = 0x00040000,
		SQF_NUMPLAYERS = 0x00080000,
		SQF_PLAYERDATA = 0x00100000,
		SQF_TEAMINFO_NUMBER = 0x00200000,
		SQF_TEAMINFO_NAME = 0x00400000,
		SQF_TEAMINFO_COLOR = 0x00800000,
		SQF_TEAMINFO_SCORE = 0x01000000,
		SQF_TESTING_SERVER = 0x02000000,
		SQF_DATA_MD5SUM = 0x04000000,
		SQF_ALL_DMFLAGS = 0x08000000,
		SQF_SECURITY_SETTINGS = 0x10000000,
		SQF_OPTIONAL_WADS = 0x20000000,
		SQF_DEH = 0x40000000,
		SQF_EXTENDED_INFO = 0x80000000,

		SQF_STANDARDQUERY =
			SQF_NAME | SQF_URL | SQF_EMAIL | SQF_MAPNAME | SQF_MAXCLIENTS |
			SQF_MAXPLAYERS | SQF_PWADS | SQF_GAMETYPE | SQF_IWAD |
			SQF_FORCEPASSWORD | SQF_FORCEJOINPASSWORD | SQF_LIMITS |
			SQF_NUMPLAYERS | SQF_PLAYERDATA | SQF_TEAMINFO_NUMBER |
			SQF_TEAMINFO_NAME | SQF_TEAMINFO_SCORE | SQF_GAMESKILL |
			SQF_TESTING_SERVER | SQF_ALL_DMFLAGS | SQF_SECURITY_SETTINGS |
			SQF_OPTIONAL_WADS | SQF_DEH | SQF_EXTENDED_INFO,

		SQF2_PWAD_HASHES = 0x00000001,

		SQF2_STANDARDQUERY = SQF2_PWAD_HASHES
	};

	ZandronumServer(const QHostAddress &address, unsigned short port);

	ExeFile *clientExe() override;

	GameHost *gameHost();
	GameClientRunner *gameRunner() override;

	bool hasRcon() const override { return true; }

	QList<GameCVar> modifiers() const override;
	EnginePlugin *plugin() const override;

	RConProtocol *rcon() override;

	QRgb teamColor(unsigned team) const;
	QString teamName(unsigned team) const;

	PathFinder wadPathFinder() override;

protected slots:
	void updatedSlot(ServerPtr server, int response);

protected:
	bool buckshot;
	bool instagib;

	float teamDamage;

	unsigned short botSkill;

	QByteArray lastReadRequest;

	unsigned short duelLimit;
	unsigned short fragLimit;
	unsigned short pointLimit;
	unsigned short winLimit;

	unsigned int numTeams;
	TeamInfo teamInfo[ST_MAX_TEAMS];

	QString testingArchive;

	QByteArray createSendRequest();
	static unsigned int millisecondTime();
	Response readRequest(const QByteArray &data);

private:
	void resetPwadsList(const QList<PWad> &wads);
};

#endif
