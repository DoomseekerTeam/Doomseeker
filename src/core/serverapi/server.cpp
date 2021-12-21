//------------------------------------------------------------------------------
// server.cpp
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
#include "server.h"

#include "configuration/doomseekerconfig.h"
#include "configuration/queryspeed.h"
#include "log.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "plugins/engineplugin.h"
#include "serverapi/exefile.h"
#include "serverapi/gameclientrunner.h"
#include "serverapi/gameexeretriever.h"
#include "serverapi/message.h"
#include "serverapi/playerslist.h"
#include "serverapi/tooltips/tooltipgenerator.h"
#include "strings.hpp"
#include <cassert>
#include <QElapsedTimer>
#include <QUdpSocket>

////////////////////////////////////////////////////////////////////////////////

DClass<Server>
{
public:
	PrivData()
	{
		gameMode = GameMode::mkCooperative();
	}

	/**
	 * This should be set to true upon successful return from doRefresh(),
	 * and to false upon failure. setServers() protected slot handles this.
	 * Example usage: Skulltag servers can use this to update ping
	 * if the server responds with "wait before refreshing".
	 */
	bool bKnown;

	/**
	 * Refresher sets this to false before calling the virtual
	 * readRequest() method. If this method sets this to true, Refresher
	 * will not modify the ping field assuming that readRequest()
	 * set ping to a correct value. If it remains false after the
	 * readRequest() call Doomseeker will use a global method to determine
	 * ping, which may be less accurate.
	 */
	bool bPingIsSet;

	bool bSecure;
	QString country;
	GameMode gameMode;
	unsigned int ping;
	bool custom;
	QList<DMFlagsSection> dmFlags;
	QString email;
	QElapsedTimer lastRefreshClock;
	QString iwad;
	bool lan;
	bool locked;
	bool lockedInGame;
	QStringList mapList;
	QString mapName;
	unsigned short maxClients;
	unsigned short maxPlayers;
	QString motd;
	QString name;
	QElapsedTimer pingClock;
	PlayersList players;
	bool randomMapRotation;
	Server::Response response;
	QList<int> scores;
	unsigned int scoreLimit;
	unsigned short timeLeft;
	unsigned short timeLimit;
	unsigned char skill;
	bool testingServer;
	QString version;
	QList<PWad> wads;
	QString webSite;
	QStringList additionalWebSites;
	/**
	 * This is used to make
	 * sure that refresh isn't run on
	 * server that is already refreshing.
	 */
	bool bIsRefreshing;
	QHostAddress address;
	QHostInfo host;
	unsigned short port;
	/**
	 * @brief Track how many resends we should try.
	 */
	int triesLeft;
	QWeakPointer<Server> self;

	QString (Server::*customDetails)();
	QByteArray (Server::*createSendRequest)();
	Server::Response (Server::*readRequest)(const QByteArray &);
};

DPointeredNoCopy(Server)

////////////////////////////////////////////////////////////////////////////////

QString Server::teamNames[] =
{
	"Blue",
	"Red",
	"Green",
	"Gold"
};

Server::Server(const QHostAddress &address, unsigned short port)
	: QObject()
{
	d->address = address;
	d->port = port;
	d->bIsRefreshing = false;
	d->lan = false;
	d->locked = false;
	d->lockedInGame = false;
	d->testingServer = false;
	d->triesLeft = 0;
	d->maxClients = 0;
	d->maxPlayers = 0;
	d->name = tr("<< ERROR >>");
	d->response = RESPONSE_NO_RESPONSE_YET;
	d->scoreLimit = 0;
	d->timeLeft = 0;
	d->timeLimit = 0;
	d->ping = 999;
	for (int i = 0; i < MAX_TEAMS; ++i)
	{
		d->scores << 0;
	}
	d->bSecure = false;
	d->randomMapRotation = false;
	d->skill = 3;
	d->bKnown = false;
	d->custom = false;
	d->lastRefreshClock.invalidate();

	set_customDetails(&Server::customDetails_default);
	set_createSendRequest(&Server::createSendRequest_default);
	set_readRequest(&Server::readRequest_default);

	if (gConfig.doomseeker.bLookupHosts)
	{
		lookupHost();
	}
}

Server::~Server()
{
	clearDMFlags();
}

POLYMORPHIC_DEFINE(QString, Server, customDetails, (), ())
POLYMORPHIC_DEFINE(QByteArray, Server, createSendRequest, (), ())
POLYMORPHIC_DEFINE(Server::Response, Server, readRequest, (const QByteArray &data), (data))

void Server::addPlayer(const Player &player)
{
	d->players << player;
}

void Server::addWad(const PWad &wad)
{
	d->wads << wad;
}

const QHostAddress &Server::address() const
{
	return d->address;
}

QString Server::addressWithPort() const
{
	return QString("%1:%2").arg(address().toString()).arg(port());
}

QStringList Server::allWadNames() const
{
	QStringList result;
	if (!d->iwad.trimmed().isEmpty())
	{
		result << d->iwad;
	}
	for (const PWad &wad : d->wads)
	{
		result << wad.name();
	}
	return result;
}

bool Server::anyWadnameContains(const QString &text, Qt::CaseSensitivity cs) const
{
	if (d->iwad.contains(text, cs))
	{
		return true;
	}

	for (int j = 0; j < numWads(); ++j)
	{
		const PWad &pwad = wad(j);
		if (pwad.name().contains(text, cs))
		{
			return true;
		}
	}
	return false;
}

void Server::clearDMFlags()
{
	d->dmFlags.clear();
}

QByteArray Server::createSendRequest_default()
{
	assert(0 && "Server::createSendRequest() is not implemented");
	return QByteArray();
}

void Server::clearPlayersList()
{
	d->players.clear();
}

void Server::clearWads()
{
	d->wads.clear();
}

ExeFile *Server::clientExe()
{
	auto f = new ExeFile();
	// TODO: Figure out a way so that plugins don't have to reset following
	// values if they don't change:
	f->setProgramName(plugin()->data()->name);
	f->setExeTypeName(tr("client"));
	f->setConfigKey("BinaryPath");
	return f;
}

const QString &Server::country() const
{
	return d->country;
}

QString Server::customDetails_default()
{
	return "";
}

const QList<DMFlagsSection> &Server::dmFlags() const
{
	return d->dmFlags;
}

const QString &Server::email() const
{
	return d->email;
}

QString Server::engineName() const
{
	if (plugin() != nullptr)
	{
		return plugin()->data()->name;
	}
	else
	{
		return tr("Undefined");
	}
}

const GameMode &Server::gameMode() const
{
	return d->gameMode;
}

GameClientRunner *Server::gameRunner()
{
	return new GameClientRunner(self());
}

const QString &Server::gameVersion() const
{
	return d->version;
}

QString Server::hostName(bool forceAddress) const
{
	if (!forceAddress && gConfig.doomseeker.bLookupHosts &&
		d->host.error() == QHostInfo::NoError && d->host.lookupId() != -1)
	{
		return QString("%1:%2").arg(d->host.hostName()).arg(port());
	}
	return QString("%1:%2").arg(address().toString()).arg(port());
}

const QPixmap &Server::icon() const
{
	return plugin()->icon();
}

bool Server::isCustom() const
{
	return d->custom;
}

bool Server::isEmpty() const
{
	return d->players.numClients() == 0;
}

bool Server::isFull() const
{
	return d->players.numClients() == maxClients();
}

bool Server::isKnown() const
{
	return d->bKnown;
}

bool Server::isLockedAnywhere() const
{
	return isLocked() || isLockedInGame();
}

bool Server::isLocked() const
{
	return d->locked;
}

bool Server::isLockedInGame() const
{
	return d->lockedInGame;
}

bool Server::isRandomMapRotation() const
{
	return d->randomMapRotation;
}

bool Server::isRefreshing() const
{
	return d->bIsRefreshing;
}

bool Server::isSecure() const
{
	return d->bSecure;
}

bool Server::isSpecial() const
{
	return isLan() || isCustom();
}

bool Server::isTestingServer() const
{
	return d->testingServer;
}

const QString &Server::iwad() const
{
	return d->iwad;
}

Server::Response Server::lastResponse() const
{
	return d->response;
}

void Server::lookupHost()
{
	QHostInfo::lookupHost(address().toString(), this,
		SLOT(setHostName(QHostInfo)));
}

const QStringList &Server::mapList() const
{
	return d->mapList;
}

const QString &Server::map() const
{
	return d->mapName;
}

unsigned short Server::maxClients() const
{
	return d->maxClients;
}

unsigned short Server::maxPlayers() const
{
	return d->maxPlayers;
}

QList<GameCVar> Server::modifiers() const
{
	return QList<GameCVar>();
}

const QString &Server::motd() const
{
	return d->motd;
}

const QString &Server::name() const
{
	return d->name;
}

int Server::numFreeClientSlots() const
{
	int returnValue = numTotalSlots() - d->players.numClients();
	return (returnValue < 0) ? 0 : returnValue;
}

int Server::numFreeJoinSlots() const
{
	int returnValue = d->maxPlayers - d->players.numClients();
	return (returnValue < 0) ? 0 : returnValue;
}

int Server::numFreeSpectatorSlots() const
{
	int returnValue = numFreeClientSlots() - numFreeJoinSlots();
	return (returnValue < 0) ? 0 : returnValue;
}

unsigned int Server::ping() const
{
	return d->ping;
}

const Player &Server::player(int index) const
{
	return d->players[index];
}

const PlayersList &Server::players() const
{
	return d->players;
}

unsigned short Server::port() const
{
	return d->port;
}

Server::Response Server::readRefreshQueryResponse(const QByteArray &data)
{
	return readRequest(data);
}

Server::Response Server::readRequest_default(const QByteArray &data)
{
	Q_UNUSED(data);
	assert(0 && "Server::readRequest(const QByteArray&) is not implemented.");
	return RESPONSE_BAD;
}

void Server::refreshStarts()
{
	d->bIsRefreshing = true;

	emit begunRefreshing(ServerPtr(self()));
	d->triesLeft = gConfig.doomseeker.querySpeed().attemptsPerServer;
	// Limit the maximum number of tries
	d->triesLeft = qMin(d->triesLeft, QuerySpeed::MAX_ATTEMPTS_PER_SERVER);
	// Sanity.
	d->triesLeft = qMax(d->triesLeft, 1);
}

void Server::refreshStops(Response response)
{
	d->lastRefreshClock.start();
	setResponse(response);
	if (!d->bPingIsSet)
	{
		// Set the current ping, if plugin didn't do so already.
		d->ping = d->pingClock.elapsed();
		d->bPingIsSet = true;
	}
	d->bIsRefreshing = false;
	d->iwad = d->iwad.toLower();
	emit updated(self(), response);
}

unsigned int Server::score(int team) const
{
	return d->scores[team];
}

unsigned int Server::scoreLimit() const
{
	return d->scoreLimit;
}

const QList<int> &Server::scores() const
{
	return d->scores;
}

QList<int> &Server::scoresMutable()
{
	return d->scores;
}

QWeakPointer<Server> Server::self() const
{
	return d->self;
}

bool Server::sendRefreshQuery(QUdpSocket *socket)
{
	if (d->triesLeft <= 0)
	{
		refreshStops(Server::RESPONSE_TIMEOUT);
		return false;
	}
	--d->triesLeft;

	QByteArray request = createSendRequest();
	if (request.isEmpty())
	{
		refreshStops(Server::RESPONSE_BAD);
		return false;
	}

	d->bPingIsSet = false;
	d->pingClock.start();

	socket->writeDatagram(request, address(), port());

	return true;
}

void Server::setCountry(const QString &country)
{
	d->country = country.trimmed().toUpper();
}

void Server::setCustom(bool custom)
{
	d->custom = custom;
}

void Server::setDmFlags(const QList<DMFlagsSection> &dmFlags)
{
	d->dmFlags = dmFlags;
}

void Server::setEmail(const QString &email)
{
	d->email = email;
}

void Server::setGameMode(const GameMode &gameMode)
{
	d->gameMode = gameMode;
}

void Server::setGameVersion(const QString &version)
{
	d->version = version;
}

void Server::setHostName(QHostInfo host)
{
	d->host = host;
	if (!d->bIsRefreshing)
		emit updated(self(), lastResponse());
}

void Server::setIwad(const QString &iwad)
{
	d->iwad = iwad;
}

void Server::setLocked(bool locked)
{
	d->locked = locked;
}

void Server::setLockedInGame(bool locked)
{
	d->lockedInGame = locked;
}

void Server::setMapList(const QStringList &mapList)
{
	d->mapList = mapList;
}

void Server::setMap(const QString &mapName)
{
	d->mapName = mapName;
}

void Server::setMaxClients(unsigned short maxClients)
{
	d->maxClients = maxClients;
}

void Server::setMaxPlayers(unsigned short maxPlayers)
{
	d->maxPlayers = maxPlayers;
}

void Server::setMotd(const QString &motd)
{
	d->motd = motd;
}

void Server::setName(const QString &serverName)
{
	d->name = serverName;
	// Don't let servers occupy more than one row with newline chars.
	d->name.replace('\n', ' ').replace('\r', ' ');
}

void Server::setPing(unsigned int ping)
{
	d->ping = ping;
}

void Server::setPingIsSet(bool b)
{
	d->bPingIsSet = b;
}

void Server::setPort(unsigned short i)
{
	d->port = i;
}

void Server::setRandomMapRotation(bool randomMapRotation)
{
	d->randomMapRotation = randomMapRotation;
}

void Server::setResponse(Response response)
{
	d->response = response;
	if (response == RESPONSE_GOOD)
	{
		d->bKnown = true;
	}
	else if (response == RESPONSE_BAD || response == RESPONSE_BANNED || response == RESPONSE_TIMEOUT)
	{
		d->bKnown = false;
	}
}

void Server::setScores(const QList<int> &scores)
{
	d->scores = scores;
}

void Server::setScoreLimit(unsigned int serverScoreLimit)
{
	d->scoreLimit = serverScoreLimit;
}

void Server::setSecure(bool bSecure)
{
	d->bSecure = bSecure;
}

void Server::setSelf(const QWeakPointer<Server> &self)
{
	d->self = self;
}

void Server::setTestingServer(bool b)
{
	d->testingServer = b;
}

void Server::setTimeLeft(unsigned short serverTimeLeft)
{
	d->timeLeft = serverTimeLeft;
}

void Server::setTimeLimit(unsigned short serverTimeLimit)
{
	d->timeLimit = serverTimeLimit;
}

void Server::setSkill(unsigned char skill)
{
	d->skill = skill;
}

void Server::setWads(const QList<PWad> &wads)
{
	d->wads = wads;
}

void Server::setWebSite(const QString &webSite)
{
	d->webSite = webSite;
}

void Server::setAdditionalWebSites(const QStringList &list)
{
	d->additionalWebSites = list;
}

QRgb Server::teamColor(int team) const
{
	switch (team)
	{
	case Player::TEAM_BLUE:
		return qRgb(0, 0, 255);
	case Player::TEAM_RED:
		return qRgb(255, 0, 0);
	case Player::TEAM_GREEN:
		return qRgb(0, 255, 0);
	case Player::TEAM_GOLD:
		return qRgb(255, 255, 0);
	default: break;
	}
	return qRgb(0, 255, 0);
}

QString Server::teamName(int team) const
{
	return team < MAX_TEAMS && team >= 0 ? teamNames[team] : "";
}

unsigned short Server::timeLeft() const
{
	return d->timeLeft;
}

unsigned short Server::timeLimit() const
{
	return d->timeLimit;
}

qint64 Server::timeMsSinceLastRefresh() const
{
	if (d->lastRefreshClock.isValid())
	{
		return d->lastRefreshClock.elapsed();
	}
	else
	{
		return -1;
	}
}

TooltipGenerator *Server::tooltipGenerator() const
{
	return new TooltipGenerator(self());
}

unsigned char Server::skill() const
{
	return d->skill;
}

const PWad &Server::wad(int index) const
{
	return wads()[index];
}

PathFinder Server::wadPathFinder()
{
	PathFinder pathFinder;
	#ifdef Q_OS_WIN32
	{
		GameExeRetriever exeRetriever(*plugin()->gameExe());
		Message msg;
		pathFinder.addPrioritySearchDir(exeRetriever.pathToOfflineExe(msg));
	}
	#endif
	if (isTestingServer())
	{
		QScopedPointer<ExeFile> exeFile(clientExe());
		Message msg;
		pathFinder.addPrioritySearchDir(exeFile->pathToExe(msg));
	}
	return pathFinder;
}

const QList<PWad> &Server::wads() const
{
	return d->wads;
}

const QString &Server::webSite() const
{
	return d->webSite;
}

const QStringList &Server::additionalWebSites() const
{
	return d->additionalWebSites;
}

QStringList Server::allWebSites() const
{
	QStringList sites = additionalWebSites();
	sites.prepend(webSite());
	return sites;
}

bool Server::isLan() const
{
	return d->lan;
}

void Server::setLan(bool b)
{
	d->lan = b;
}
