//------------------------------------------------------------------------------
// server.h
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

#ifndef __SERVER_H__
#define __SERVER_H__

#include "dptr.h"
#include "global.h"
#include "serverapi/polymorphism.h"
#include "serverapi/serverptr.h"

#include <QColor>
#include <QHostAddress>
#include <QHostInfo>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QTime>
#include <QUdpSocket>

class DMFlagsSection;
class EnginePlugin;
class ExeFile;
class GameCVar;
class GameHost;
class GameMode;
class GameClientRunner;
class PathFinder;
class Player;
class PlayersList;
class PWad;
class RConProtocol;
class TooltipGenerator;

/**
 * @ingroup group_pluginapi
 * @brief A representation of a server for a given game.
 *
 * This is a multipurpose class that represents a game server in general.
 * Server data is stored here, as well as interfaces for following operations
 * are present:
 * - parsing packets received when challenging the server, and creating contents
 *   of the challenge packets;
 * - accessing game ExeFile for current Server (which can be a non-standard
 *   executable derived from information provided by the server itself);
 * - spawning GameClientRunner object that generates "join" command line;
 * - RConProtocol;
 * - Server's tooltip generator: TooltipGenerator;
 * - PathFinder configured to find WADs for this server, where plugins
 *   may define a customized behavior.
 *
 * Doomseeker is designed in a way that ownership of Server instances are
 * relegated to QSharedPointer objects. These objects allow Doomseeker
 * to push the Server to various asynchronous operations and not worry
 * that the server object will be deleted before it stops being needed.
 * This way we avoid references to dangling pointers, but there's also a price.
 * Any class that requires a QSharedPointer<Server> will lead to undefined
 * behavior (crash) if plain Server* pointer is passed. Instead, Server
 * provides a self() method that returns a reference that is safe to be casted
 * to QSharedPointer. This method should also be used instead of 'this' pointer.
 * The drawback is that <b>self() is not available until the object is fully
 * constructed, thus its use in constructor is prohibited.</b>
 *
 * ServerPtr is basically Doomseeker's alias for QSharedPointer<Server>.
 *
 * @attention
 * Some methods in this class are virtual by the natural means provided by C++,
 * but some are marked as virtual in documentation only and can be customized
 * through method pointer setters.
 */
class MAIN_EXPORT Server : public QObject
{
	Q_OBJECT

	friend class ServerPointer;

public:
	/**
	 * @brief Type of response that is extracted by parsing the packet
	 *        that came from the Server.
	 *
	 * Response is returned from readRequest() which needs to be customized
	 * by each plugin.
	 */
	enum Response
	{
		/**
		 * @brief Response was parsed properly and Server information is
		 *        available.
		 */
		RESPONSE_GOOD,
		/**
		 * @brief Server didn't respond at all.
		 */
		RESPONSE_TIMEOUT,
		/**
		 * @brief Server responded with "wait", may happen when we refresh
		 *        too quickly.
		 */
		RESPONSE_WAIT,
		/**
		 * @brief Response from the server was erroneous.
		 *
		 * Such servers are marked as invalid by Doomseeker.
		 */
		RESPONSE_BAD,
		/**
		 * @brief Player is banned from that server.
		 */
		RESPONSE_BANNED,
		/**
		 * @brief "Dummy" response for servers that weren't refreshed yet.
		 */
		RESPONSE_NO_RESPONSE_YET,
		/**
		 * @brief Waiting for additional packets.
		 *
		 * Some games might return challenge result in more than one packet.
		 * This response marks that we're awaiting more packets.
		 */
		RESPONSE_PENDING,
		/**
		 * @brief Doomseeker needs to send some more challenge data
		 *        to the server.
		 *
		 * When this is returned, createSendRequest() will be called again.
		 * It's up to the plugin to know whether any modification to the
		 * generated request needs to be applied.
		 */
		RESPONSE_REPLY,
	};

	/**
	 * @brief Spawn server with given address and port.
	 */
	Server(const QHostAddress &address, unsigned short port);
	virtual ~Server() override;

	// VIRTUALS
	/**
	 * @brief Client executable retriever.
	 *
	 * Can return different types of instances depending on
	 * the information provided by the server. Ownership of the
	 * ExeFile object is returned to the caller.
	 */
	virtual ExeFile *clientExe();
	/**
	 * @brief Creates an instance of GameClientRunner's derivative class.
	 *
	 * Gets a pointer to a new instance of GameClientRunner's
	 * descendant (defined by a plugin). Created instance should be deleted
	 * manually by the programmer (ownership is returned to the caller).
	 */
	virtual GameClientRunner *gameRunner();
	/**
	 * @brief Does this server support remote console connection?
	 *
	 * @see rcon()
	 */
	virtual bool hasRcon() const { return false; }
	/**
	 * @brief What kind of game modifiers are enabled on this server?
	 */
	virtual QList<GameCVar> modifiers() const;
	/**
	 * @brief Spawns RConProtocol instance.
	 *
	 * Ownership of the new object is returned to the caller.
	 *
	 * @see hasRcon()
	 */
	virtual RConProtocol *rcon() { return nullptr; }
	/**
	 * @brief Color of team under given index.
	 *
	 * For example: team 0 is red, team 1 is blue.
	 */
	virtual QRgb teamColor(int team) const;
	/**
	 * @brief Name of team under given index.
	 *
	 * For example: team 0 is "attacking", team 1 is "defending".
	 */
	virtual QString teamName(int team) const;
	/**
	 * This is supposed to return the plugin this Server belongs to.
	 * New instances of EnginePlugin shouldn't be created here. Instead
	 * each plugin should keep a global instance of EnginePlugin (singleton?)
	 * and a pointer to this instance should be returned.
	 */
	virtual EnginePlugin *plugin() const = 0;

	/**
	 * @brief Creates an instance of TooltipGenerator.
	 *
	 * This can be replaced by a plugin to use a custom tooltip generator.
	 * Ownership of TooltipGenerator object is returned to the caller.
	 *
	 * @return Default behaviour returns default implementation of
	 *         TooltipGenerator.
	 */
	virtual TooltipGenerator *tooltipGenerator() const;

	/**
	 * @brief Instantiate and return PathFinder configured to search
	 *        for WADs for this server.
	 *
	 * It's expected that this method will return a PathFinder that is
	 * configured to the best possible extent. If any error is encountered,
	 * it shouldn't result in returning an invalid PathFinder.
	 *
	 * @warning
	 * Be aware that method may be called from UI elements that expect
	 * quick, non-blocking response.
	 *
	 * Default implementation creates PathFinder with paths configured
	 * by the user in configuration box, and with priority search
	 * directories set to wherever client and offline executables
	 * reside, if such executables are available.
	 *
	 * Depending on the underlying OS, extra paths may also be added by
	 * Doomseeker after the user configured directories.
	 *
	 * Additional paths appended on Linux:
	 *
	 * 1. `/usr/local/share/games/doom/`
	 * 2. `/usr/share/games/doom/`
	 */
	virtual PathFinder wadPathFinder();
	// END OF VIRTUALS

	/**
	 * @brief Add new Player to this server's PlayersList.
	 *
	 * Plugins can use this method to add new Player to the list when
	 * parsing the server response packet.
	 *
	 * @see clearPlayersList()
	 */
	void addPlayer(const Player &player);
	/**
	 * @brief Address of this server.
	 */
	const QHostAddress &address() const;
	/**
	 * @brief Returns "address:port" string.
	 */
	QString addressWithPort() const;

	/**
	 * @brief IWAD + PWADs.
	 */
	QStringList allWadNames() const;

	/**
	 * @brief True if name of any WAD of this server contains given text.
	 */
	bool anyWadnameContains(const QString &text,
		Qt::CaseSensitivity cs = Qt::CaseInsensitive) const;

	/**
	 * @brief Removes all players from PlayersList.
	 *
	 * @see addPlayer()
	 */
	void clearPlayersList();

	/**
	 * @brief <b>[Virtual]</b> Allows a plugin to provide custom server details.
	 */
	QString customDetails();

	/**
	 * @brief dmflags used by this server.
	 */
	const QList<DMFlagsSection> &dmFlags() const;
	/**
	 * @brief Email address provided by this server.
	 */
	const QString &email() const;

	/**
	 * @brief Returns name of the engine for this server,
	 *       for example: "Skulltag".
	 *
	 * This returns name defined by the parent plugin itself,
	 * or "Undefined" string if there is no parent plugin.
	 */
	QString engineName() const;

	/**
	 * @brief GameMode that is currently running on this server.
	 */
	const GameMode &gameMode() const;
	/**
	 * @brief Version of the server program (1.0, 0.98-beta, and so on).
	 */
	const QString &gameVersion() const;

	/**
	 * @brief A string that is either the "ipaddress:port"
	 *        or "hostname:port"
	 *
	 * 'hostname' is returned depending on if the hostname information has
	 * been retrieved, if reverse lookups are enabled, and on
	 * the forceAddress parameter.
	 *
	 * @param forceAddress
	 *     Force method to return 'ipaddress' even if 'hostname'
	 *     is available.
	 */
	QString hostName(bool forceAddress = false) const;
	/**
	 * @brief Icon for this server.
	 */
	const QPixmap &icon() const;

	/**
	 * @brief Is this a custom server defined by the user?
	 *
	 * Custom servers can be defined from the configuration box.
	 */
	bool isCustom() const;
	/**
	 * @brief Are there any players on this server?
	 */
	bool isEmpty() const;
	/**
	 * @brief Is this server full?
	 */
	bool isFull() const;
	/**
	 * @brief Is information for this server available?
	 *
	 * This is set to true when parsing of the response packet finishes
	 * successfully.
	 */
	bool isKnown() const;

	/**
	 * @brief True if any "isLocked()" returns true.
	 */
	bool isLockedAnywhere() const;
	/**
	 * @brief "Connect" passworded or not.
	 */
	bool isLocked() const;
	/**
	 * @brief "Join" passworded or not.
	 */
	bool isLockedInGame() const;

	/**
	 * @brief Is random maplist rotation enabled?
	 */
	bool isRandomMapRotation() const;
	/**
	 * @brief Is the server being refreshed at the current moment?
	 */
	bool isRefreshing() const;

	/**
	 * @brief Secure as in 'secure for players', not 'passworded'.
	 *
	 * When this flag is enabled Doomseeker will draw a shield next to the
	 * engine icon on server list. What is actually means depends on the
	 * game. For example, it may mean that this server uses the global
	 * ban list provided by the master server.
	 */
	bool isSecure() const;

	/**
	 * @brief Special servers are custom servers or LAN servers.
	 */
	bool isSpecial() const;

	/**
	 * @brief Does this server run a testing version of the game?
	 */
	bool isTestingServer() const;

	/**
	 * @brief IWAD used by this server.
	 */
	const QString &iwad() const;

	/**
	 * @brief Last response status deduced by parsing of the server
	 *        response packet.
	 */
	Response lastResponse() const;
	/**
	 * @brief Prompts the server to reverse resolve its address to
	 *        a hostname.
	 *
	 * The lookup will be performed asynchronously. When lookup is
	 * finished then updated() signal will be emitted.
	 */
	void lookupHost();

	/**
	 * @brief Name of the current level.
	 */
	const QString &map() const;
	/**
	 * @brief List of all levels that are in map rotation on this server.
	 */
	const QStringList &mapList() const;
	/**
	 * @brief Amount of connection slots for this server, as reported by
	 *        the server.
	 *
	 * @see numTotalSlots().
	 */
	unsigned short maxClients() const;
	/**
	 * @brief Amount of play slots for this server.
	 */
	unsigned short maxPlayers() const;
	/**
	 * @brief Message of the Day.
	 */
	const QString &motd() const;
	/**
	 * @brief Server's name.
	 */
	const QString &name() const;
	/**
	 * @brief Amount of free connection slots.
	 */
	int numFreeClientSlots() const;
	/**
	 * @brief Amount of free play slots.
	 */
	int numFreeJoinSlots() const;
	/**
	 * @brief Amount of free spectator slots.
	 *
	 * Usually numFreeClientSlots() - numFreeJoinSlots().
	 */
	int numFreeSpectatorSlots() const;
	/**
	 * @brief Actual number of free connection slots deduced from
	 *        maxPlayers() and maxClients().
	 */
	int numTotalSlots() const
	{
		return maxPlayers() > maxClients() ? maxPlayers() : maxClients();
	}
	/**
	 * @brief Number of PWADs loaded on this server.
	 */
	int numWads() const { return wads().size(); }
	/**
	 * @brief Ping from local host to this server.
	 */
	unsigned int ping() const;
	/**
	 * @brief Player at given index of PlayersList.
	 */
	const Player &player(int index) const;
	/**
	 * @brief List of players that are on this server currently.
	 */
	const PlayersList &players() const;
	/**
	 * @brief Network port on which this server is hosted.
	 */
	unsigned short port() const;

	/**
	 * @brief Entry point for Refresher that pushes response packet for
	 *        parsing.
	 */
	Response readRefreshQueryResponse(const QByteArray &data);

	/**
	 * @brief Called when server begins refreshing routine.
	 */
	void refreshStarts();

	/**
	 * @brief Called when server finishes refreshing routine.
	 */
	void refreshStops(Response response);

	/**
	 * @brief Total score of a given team, by default team 0 is used.
	 */
	unsigned int score(int team = 0) const;
	/**
	 * @brief Scores for all teams.
	 */
	const QList<int> &scores() const;
	/**
	 * @brief Current score limit.
	 *
	 * This might mean something different depending on current game mode.
	 */
	unsigned int scoreLimit() const;

	/**
	 * @brief Reference to this server made available ONLY after the server
	 *        is fully created.
	 *
	 * This needs to be used instead of "this" pointer when Server needs
	 * to be passed to a QSharedPointer.
	 * <b>Result is undefined when this method is called in
	 * the constructor.</b>
	 */
	QWeakPointer<Server> self() const;

	/**
	 * @brief Method called by the refreshing thread; sends the challenge
	 *        query through refresher's socket.
	 *
	 * @return false if it's impossible to send the query (fail)
	 */
	bool sendRefreshQuery(QUdpSocket *socket);

	/**
	 * @brief Set whether this server is custom, should be called
	 *        by Doomseeker only.
	 */
	void setCustom(bool custom);
	/**
	 * @brief Set email parsed from the response packet.
	 */
	void setEmail(const QString &mail);
	/**
	 * @brief Set game mode parsed from the response packet.
	 */
	void setGameMode(const GameMode &gameMode);
	/**
	 * @brief Set current level parsed from the response packet.
	 */
	void setMap(const QString &name);
	/**
	 * @brief Set map rotation list parsed from the response packet.
	 */
	void setMapList(const QStringList &mapList);
	/**
	 * @brief Set amount of slots for client parsed from the response packet.
	 */
	void setMaxClients(unsigned short i);
	/**
	 * @brief Set amount of slots for players parsed from the response packet.
	 */
	void setMaxPlayers(unsigned short i);
	/**
	 * @brief Set Message of the Day parsed from the response packet.
	 */
	void setMotd(const QString &message);
	/**
	 * @brief Set server's name parsed from the response packet.
	 */
	void setName(const QString &name);
	/**
	 * @brief Set network port, should be called by Doomseeker only.
	 */
	void setPort(unsigned short i);
	/**
	 * @brief Set random map rotation status parsed from the response packet.
	 */
	void setRandomMapRotation(bool b);
	/**
	 * @brief Set "self" reference, should by called by Doomseeker only.
	 *
	 * @see self()
	 */
	void setSelf(const QWeakPointer<Server> &self);
	/**
	 * @brief Set skill level parsed from the response packet.
	 */
	void setSkill(unsigned char newSkill);
	/**
	 * @brief Set web site URL parsed from the response packet.
	 */
	void setWebSite(const QString &site);

	/**
	 * @brief Amount of time until the round is over, expressed in minutes.
	 */
	unsigned short timeLeft() const;
	/**
	 * @brief Round time limit, expressed in minutes.
	 */
	unsigned short timeLimit() const;

	/**
	 * @brief Milliseconds elapsed since last refresh.
	 *
	 * Underlying code uses QElapsedTimer to provide
	 * unified results that ignore events like daylight savings time change
	 * or user manipulating with system clock.
	 *
	 * @return A number of milliseconds since the completion of the last
	 *     refresh operation. If server wasn't refreshed yet
	 *     or is currently refreshing for the first time, then
	 *     this will return a negative value.
	 */
	qint64 timeMsSinceLastRefresh() const;

	/**
	 * @brief Game skill level, starting from zero.
	 *
	 * 0 - I'm too young to die,
	 *    ...
	 * 4 - Nightmare!.
	 */
	unsigned char skill() const;
	/**
	 * @brief PWAD under given index on the PWADs list.
	 */
	const PWad &wad(int index) const;
	/**
	 * @brief List of all PWADs loaded on this server.
	 */
	const QList<PWad> &wads() const;
	/**
	 * @brief Website URL provided by this server.
	 */
	const QString &webSite() const;

	/**
	 * @brief Does this server come from LAN.
	 */
	bool isLan() const;
	void setLan(bool b);

signals:
	/**
	 * @brief Emitted when refresh process begins for the current server.
	 *
	 * @param server
	 *     Pointer to 'this'.
	 */
	void begunRefreshing(ServerPtr server);
	/**
	 * @brief Emitted when a refresh has been completed.
	 *
	 * Be sure to check the response to see if anything has actually
	 * changed.
	 *
	 * @param server
	 *     Pointer to 'this'.
	 *
	 * @see Response
	 */
	void updated(ServerPtr server, int response);

protected:
	POLYMORPHIC_SETTER_DECLARE(QString, Server, customDetails, ())
	QString customDetails_default();

	/**
	 * @brief <b>[Pure Virtual]</b> Reads response packet.
	 *
	 * @return The resposne that should be emitted. Do NOT perform any
	 *         signal emissions from within this functions. This is not
	 *         thread safe and may lead to a crash.
	 */
	Response readRequest(const QByteArray &data);
	POLYMORPHIC_SETTER_DECLARE(Response, Server, readRequest, (const QByteArray &data))

	/**
	 * @brief <b>[Pure Virtual]</b> Prepares challenge data.
	 *
	 * @return A prepared packet. Return empty buffer to signal error.
	 */
	QByteArray createSendRequest();
	POLYMORPHIC_SETTER_DECLARE(QByteArray, Server, createSendRequest, ())

	/**
	 * @brief Add PWAD to the list of this server's PWADs.
	 */
	void addWad(const PWad &wad);
	/**
	 * @brief Clear PWADs list.
	 */
	void clearWads();
	/**
	 * @brief Mutable reference to the scores list, available for contents
	 *        modification.
	 */
	QList<int> &scoresMutable();
	/**
	 * @brief Set dmFlags().
	 */
	void setDmFlags(const QList<DMFlagsSection> &dmFlags);
	/**
	 * @brief Set gameVersion().
	 */
	void setGameVersion(const QString &version);
	/**
	 * @brief Set iwad().
	 */
	void setIwad(const QString &iwad);
	/**
	 * @brief Set isLocked().
	 */
	void setLocked(bool locked);
	/**
	 * @brief Set isLockedInGame().
	 */
	void setLockedInGame(bool locked);
	/**
	 * @brief Set ping().
	 *
	 * If this value is meant to be used instead of automatic ping
	 * calculation then remember to also set setPingIsSet() to true.
	 */
	void setPing(unsigned int currentPing);

	/**
	 * @brief Plugins should set this to true to prevent default ping
	 *        calculation.
	 *
	 * This is useful if plugin has its own way of determining ping.
	 */
	void setPingIsSet(bool b);

	void setTestingServer(bool b);
	/**
	 * @brief Set timeLeft().
	 */
	void setTimeLeft(unsigned short timeLeft);
	/**
	 * @brief Set timeLimit().
	 */
	void setTimeLimit(unsigned short timeLimit);
	/**
	 * @brief Set scoreLimit().
	 */
	void setScoreLimit(unsigned int scoreLimit);
	/**
	 * @brief Set isSecure().
	 */
	void setSecure(bool bSecureServer);

private:
	Q_DISABLE_COPY(Server)

	static QString teamNames[];

	DPtr<Server> d;

	void clearDMFlags();

	QByteArray createSendRequest_default();
	Response readRequest_default(const QByteArray &data);

	void setResponse(Response response);
	void setScores(const QList<int> &scores);
	void setWads(const QList<PWad> &wads);

private slots:
	void setHostName(QHostInfo host);
};

Q_DECLARE_METATYPE(ServerPtr)
Q_DECLARE_METATYPE(ServerCPtr)

#endif /* __SERVER_H__ */
