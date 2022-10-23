//------------------------------------------------------------------------------
// gamecreateparams.h
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
#ifndef idF7CFB784_3591_4B2C_B5529AE7FBCC2568
#define idF7CFB784_3591_4B2C_B5529AE7FBCC2568

#include "dptr.h"
#include "global.h"
#include <QList>
#include <QString>
#include <QStringList>

class DMFlagsSection;
class GameCVar;
class GameDemo;
class GameMode;
class PWad;

/**
 * @ingroup group_pluginapi
 * @brief Game parametrization data used when creating new games.
 */
class MAIN_EXPORT GameCreateParams
{
public:
	enum HostMode
	{
		Host,
		Offline,
		Demo,
		Remote
	};

	GameCreateParams();
	~GameCreateParams();

	/**
	 * @brief Password that allows clients to connect to the server.
	 */
	const QString &connectPassword() const;
	void setConnectPassword(const QString &pass);

	QStringList &customParameters();
	const QStringList &customParameters() const;
	void setCustomParameters(const QStringList &customParameters);

	/**
	 * @brief Contents of this list will be passed as "+consoleCommand value"
	 *        to the command line.
	 */
	QList<GameCVar> &cvars();
	const QList<GameCVar> &cvars() const;
	void setCvars(const QList<GameCVar> &cvars);

	/**
	 * @brief Use if running in HostMode::Demo mode or recording a demo.
	 *
	 * Used either as a path to already existing demo that should
	 * be played back, or as a path to a new demo that should be
	 * recorded.
	 *
	 * When hostMode() is HostMode::Demo then this points to a
	 * demo for playback.
	 *
	 * When hostMode() is HostMode::Offline and demoRecord() is
	 * not GameDemo::NoDemo then this points to a new demo to
	 * record.
	 *
	 * @see hostMode()
	 * @see demoRecord()
	 */
	const QString &demoPath() const;
	void setDemoPath(const QString &demoPath);

	/**
	 * @brief Type of demo to record; applicable only in Offline game.
	 *
	 * If set to record a demo, then demoPath() must also be set.
	 * Default is GameDemo::NoDemo.
	 */
	const GameDemo &demoRecord() const;
	void setDemoRecord(const GameDemo &demo);


	QList<DMFlagsSection> &dmFlags();
	const QList<DMFlagsSection> &dmFlags() const;

	const QString &email() const;
	void setEmail(const QString &email);

	/**
	 * @brief Path to the game executable.
	 */
	const QString &executablePath() const;
	void setExecutablePath(const QString &executablePath);

	const GameMode &gameMode() const;
	void setGameMode(const GameMode &mode);

	HostMode hostMode() const;
	void setHostMode(HostMode mode);

	bool isBroadcastToLan() const;
	void setBroadcastToLan(bool b);

	bool isBroadcastToMaster() const;
	void setBroadcastToMaster(bool b);

	bool isRandomMapRotation() const;
	void setRandomMapRotation(bool b);

	/**
	 * @brief Internal game password.
	 */
	const QString &ingamePassword() const;
	void setIngamePassword(const QString &pass);

	const QString &iwadPath() const;
	void setIwadPath(const QString &iwadPath);

	/**
	 * @brief Name of IWAD, derived from iwadPath().
	 */
	QString iwadName() const;

	/**
	 * @brief Path to the directory where logs should be saved.
	 */
	const QString &loggingPath() const;
	void setLoggingPath(const QString &iwadPath);
	/**
	 * @brief Level name as in E1M1 or MAP01.
	 */
	const QString &map() const;
	void setMap(const QString &map);

	/**
	 * @brief List of maps in cycle, as in MAP01, MAP02, MAP03, and so on.
	 */
	const QStringList &mapList() const;
	void setMapList(const QStringList &mapList);

	int maxClients() const;
	void setMaxClients(int num);

	int maxPlayers() const;
	void setMaxPlayers(int num);

	/**
	 * @brief Derived basing on maxClients() and maxPlayers() value;
	 *        higher value wins.
	 */
	int maxTotalClientSlots() const;
	/**
	 * @brief Message of the Day.
	 */
	const QString &motd() const;
	void setMotd(const QString &motd);

	const QString &name() const;
	void setName(const QString &name);

	/**
	 * @brief Option is a means for game plugin to set plugin specific settings.
	 *
	 * Plugins can implement custom "create game" pages and through
	 * setOption() and option() pass options to their GameHost
	 * implementation where they can be further processed.
	 *
	 * These options do not necessarily need to produce any game
	 * command line arguments, but they may be used to alter
	 * the generation of others. The exact usage is up to the plugin.
	 */
	QVariant option(const QString &name) const;
	void setOption(const QString &name, const QVariant &value);

	unsigned short port() const;
	void setPort(unsigned short port);

	QList<bool> &pwadsOptional();
	const QList<bool> &pwadsOptional() const;
	void setPwadsOptional(const QList<bool> &pwadsOptional);

	QStringList &pwadsPaths();
	const QStringList &pwadsPaths() const;
	void setPwadsPaths(const QStringList &pwadsPaths);

	/**
	 * @brief PWad objects with just the file names and optional statuses.
	 */
	QList<PWad> pwads() const;

	/**
	 * @brief Names of PWADs, derived from pwadsPaths().
	 */
	QStringList pwadsNames() const;

	/**
	 * @brief Password required to connect to remote admin console.
	 */
	const QString &rconPassword() const;
	void setRconPassword(const QString &pass);

	/**
	 * @brief Difficulty level.
	 */
	int skill() const;
	void setSkill(int skill);

	/**
	 * @brief URL for server's website or for WADs download.
	 */
	const QString &url() const;
	void setUrl(const QString &url);

	/**
	 * @brief If set, the game should try to enable UPnP.
	 */
	bool upnp() const;
	void setUpnp(bool upnp);

	/**
	 * @brief Game or plugin can use this network port as it sees fit within
	 * the UPnP context.
	 *
	 * This value should be ignored if upnp() is false.
	 */
	quint16 upnpPort() const;
	void setUpnpPort(quint16 port);

private:
	DPtr<GameCreateParams> d;
};



#endif // header
