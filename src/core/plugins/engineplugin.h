//------------------------------------------------------------------------------
// engineplugin.h
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
// Copyright (C) 2011 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef __ENGINEPLUGIN_H__
#define __ENGINEPLUGIN_H__

#include "global.h"
#include "serverapi/serverptr.h"
#include "serverapi/serverstructs.h"
#include "serverapi/textprovider.h"
#include <QString>
#include <QStringList>
#include <QtContainerFwd>
#include <QVector>

// Bump whenever the ABI changes in order to reject old plugins
#define DOOMSEEKER_ABI_VERSION 2

#define DECLARE_PLUGIN(XEnginePlugin) \
	friend PLUGIN_EXPORT EnginePlugin * doomSeekerInit(); \
public: \
	static EnginePlugin *staticInstance() { return __Static_Instance; } \
private: \
	static XEnginePlugin *__Static_Instance;

#define INSTALL_PLUGIN(XEnginePlugin) \
	XEnginePlugin *XEnginePlugin::__Static_Instance; \
	extern "C" PLUGIN_EXPORT unsigned int doomSeekerABI() { return DOOMSEEKER_ABI_VERSION; } \
	extern "C" PLUGIN_EXPORT EnginePlugin *doomSeekerInit() \
	{ \
		XEnginePlugin::__Static_Instance = new XEnginePlugin(); \
		return XEnginePlugin::staticInstance(); \
	}

class Broadcast;
class ConfigPage;
class CreateServerDialog;
class CreateServerDialogPage;
class GameCVarProvider;
class GameCVar;
class GameExeFactory;
class GameHost;
class GameMode;
class IniSection;
class IRCNetworkEntity;
class MasterClient;
class Server;
class QHostAddress;
class QPixmap;
class QWidget;
class TextProvider;

/**
 * @ingroup group_pluginapi
 */
class MAIN_EXPORT EnginePlugin
{
protected:
	/**
	 * We will use this to tell Init what features we have, new features
	 * can be added to this list, but the position of any feature must not
	 * change.
	 */
	enum InitFeatures
	{
		EP_Done, ///< Signals the end of init parameters.

		EP_Author, ///< (const char*) Author of the plugin.
		EP_Version, ///< (unsigned int) Single version number for plugin.

		EP_AllowsConnectPassword, ///< Signifies that servers can be created with a connection password.
		EP_AllowsEmail, ///< Signifies that servers can have an administrative contact email attached.
		EP_AllowsURL, ///< Signifies that servers can provide a URL for potential wad downloads.
		EP_AllowsJoinPassword, ///< Signifies that servers can be created with a join password.
		EP_AllowsRConPassword, ///< Signifies that servers can be created for remote console access.
		EP_AllowsMOTD, ///< Signifies that servers can have a message of the day.
		EP_DefaultMaster, ///< (const char*) Default ip address and port ("address:port") for master server.  Requires EP_HasMasterServer.
		EP_DefaultServerPort, ///< (quint16) Default port for custom server creation.
		EP_InGameFileDownloads, ///< Allows the player to join a server without downloading files through Wadseeker.
		EP_IRCChannel, ///< (const char*)server, (const char*)channel - Can be repeated. Default IRC channels.
		EP_SupportsRandomMapRotation, ///< Signifies that a server can be created with a random map rotation.
		EP_RefreshThreshold, ///< (quint8) The amount of time (in seconds) that must pass before a server can be requeried.
		EP_URLScheme, ///< (const char*) Overrides the URL scheme which Doomseeker sets for this plugin. By default it is the port's name without spaces.
		EP_DemoExtension, ///< (bool)auto, (const char*)extension Sets the extension that will be used for demos (default is true and lmp).

		/**
		 * (bool)
		 * If specified then "Create Game" dialog won't build
		 * flags pages out of the EnginePlugin::dmFlags() list.
		 * Plugin either doesn't want to have the flags pages created
		 * or will provide the pages on its own through
		 * EnginePlugin::createServerDialogPages().
		 */
		EP_DontCreateDMFlagsPagesAutomatic,
		EP_ClientOnly, ///< Indicates that client binary serves the purpose of the client and server.
		/**
		 * @brief LAN Broadcast object.
		 */
		EP_Broadcast,
		/**
		 * @brief MasterClient object.
		 */
		EP_MasterClient,
		/**
		 * @brief File name for game's client (main) executable.
		 *
		 * If defined it will be used to generate default list of
		 * GameExeFactory::gameFiles().
		 *
		 * Platform specific file extension doesn't need to be specified.
		 */
		EP_ClientExeName,
		/**
		 * @brief File name for game's server executable if game has any.
		 *
		 * If defined it will be used to generate default list of
		 * GameExeFactory::gameFiles().
		 *
		 * If EP_ClientOnly is set to true this setting has no effect.
		 *
		 * Platform specific file extension doesn't need to be specified.
		 */
		EP_ServerExeName,
		/**
		 * @brief Default search suffixes used to automatically find
		 * game files.
		 *
		 * Doomseeker may help user in finding necessary game files, such as
		 * executables, by appending these suffixes to certain default
		 * search paths (such as Program Files directory or
		 * /usr/share/games). These paths may differ per platform.
		 *
		 * This is a const char* string where multiple suffixes can be
		 * separated with a semicolon ';'.
		 *
		 * Example for Windows platform: a "MyGame" suffix will prompt
		 * Doomseeker to search for game files in
		 * "C:/Program Files (x86)/MyGame" and "C:/Program Files/MyGame".
		 */
		EP_GameFileSearchSuffixes,
		/**
		 * @brief GameCVarProvider that returns difficulty levels ordered
		 * from easiest to hardest.
		 *
		 * The context param may be a QVariantMap with "iwad" field or may be
		 * empty in which case a default list should be returned.
		 */
		EP_DifficultyProvider,
		/**
		 * @brief Disables map list in create game box.
		 */
		EP_NoMapList,
		/**
		 * @brief Informs that the game has no notion of an IWAD.
		 *
		 * Either IWAD is not used in this game at all or only one
		 * IWAD is supported and it is hardcoded.
		 */
		EP_NoIwad,
		/**
		 * @brief Disables specifying amount of client slots
		 * in create game box.
		 *
		 * Player slots, unless disabled with EP_NoPlayerSlots,
		 * can still be set. This is useful for games that don't
		 * allow to specify spectator-only slots.
		 */
		EP_NoClientSlots,
		/**
		 * @brief Disables specifying amount of player slots
		 * in create game box.
		 *
		 * Combined with EP_NoClientSlots this allows to disable
		 * maximum player controls in create game window for games
		 * that have a hardcoded amount of maximum players.
		 */
		EP_NoPlayerSlots,
		/**
		 * @brief Tells the create game box that this game
		 * allows UPnP.
		 *
		 * By default, UPnP control is hidden.
		 */
		EP_AllowsUpnp,
		/**
		 * @brief Tells the create game box that this game
		 * allows to modify UPnP port.
		 *
		 * What is meant by "UPnP port" is "a network port that can be used
		 * by the game in some relation to UPnP however the game sees fit."
		 *
		 * This setting works in conjuction with EP_AllowsUpnp.
		 * If the plugin specifies this one, it also must specify
		 * the EP_AllowsUpnp.
		 *
		 * By default this is disabled.
		 */
		EP_AllowsUpnpPort,
		/**
		 * @brief Provides a description to be shown in the "About" menu.
		 */
		EP_AboutProvider,
		/**
		 * @brief Plugin canonical name; should match filenames of the
		 * library and translation files.
		 *
		 * This setting is optional. If not specified, Doomseeker will
		 * try to derive the canonical name from the human-readable name.
		 *
		 * Recommendation for canonical names is to use text that is
		 * short, related to the human-readable name, reasonably unique,
		 * all lowercase, has no whitespace and is preferably one word
		 * or an abbreviation.
		 *
		 * Canonical name is used to match names of translation files.
		 * This means that names of the translation files must have the
		 * same prefix as the canonical name. For example, if canonical
		 * name is "spacewar", the Spanish translation file must be named
		 * "spacewar_es_ES.qm".
		 *
		 * While certain plugins may match their canonical name to their
		 * actual, human-readable name in a simple and derivable fashion,
		 * some other plugins may have a canonical name that is drastically
		 * different. The author of the plugin should choose the canonical
		 * name carefully as it may be used and remembered throughout the
		 * program to identify this particular plugin. Changing this name
		 * in a later release may lead to data or settings loss by users.
		 */
		EP_CanonicalName,
		/**
		 * @brief enables logging info (like console output) into a file.
		 * This will show the Logging section in the general game setup
		 * panel.
		 */
		EP_AllowsLogging
	};

	/// Reimplement if you want to perform some ini initialization manually.
	virtual void setupConfig(IniSection &config);

public:
	/**
	 * Store the information about the plugin in a structure so that we can
	 * freely add features without invalidating existing plugins.
	 */
	class Data
	{
	public:
		unsigned int abiVersion;
		bool allowsConnectPassword;
		bool allowsEmail;
		bool allowsJoinPassword;
		bool allowsMOTD;
		bool allowsRConPassword;
		bool allowsURL;
		QString author;
		/// Default port on which servers for given engine are hosted.
		QString defaultMaster;
		quint16 defaultServerPort;
		/// icon of the engine
		QPixmap *icon;
		bool inGameFileDownloads;
		QVector<IRCNetworkEntity> ircChannels;
		MasterClient *masterClient;
		QString name;
		IniSection *pConfig;
		quint8 refreshThreshold;
		QString scheme;
		bool supportsRandomMapRotation;
		bool valid;
		unsigned int version;
		bool demoExtensionAutomatic;
		QString demoExtension;
		/**
		 * @brief Description of the plugin, intended to be displayed in
		 * the "About" dialog.
		 */
		QScopedPointer<TextProvider> aboutProvider;
		/**
		 * @brief Controls behavior of "Create Game" dialog.
		 *
		 * If true then "Create Game" dialog will build
		 * flags pages out of the dmFlags list. If false then plugin
		 * either doesn't want to have the flags pages created or will
		 * provide the pages on its own.
		 *
		 * Default: true.
		 */
		bool createDMFlagsPagesAutomatic;
		bool clientOnly;
		/**
		 * @brief Factory of executable retrievers ExeFile objects.
		 *
		 * By default this is a simple instance of GameExeFactory.
		 * If custom behavior is needed, plugins should overwrite
		 * this instance with own pointer.
		 *
		 * This factory should be set only once during plugin
		 * initialization and then left untouched.
		 */
		QSharedPointer<GameExeFactory> gameExeFactory;
		Broadcast *broadcast;

		QString clientExeName;
		QString serverExeName;
		QStringList gameFileSearchSuffixes;

		/**
		 * @brief Difficulty levels provider for this game.
		 *
		 * Defaults to returning Doom difficulty levels.
		 */
		QSharedPointer<GameCVarProvider> difficulty;
		bool hasMapList;
		bool hasIwad;
		bool allowsClientSlots;
		bool allowsPlayerSlots;
		bool allowsUpnp;
		bool allowsUpnpPort;
		bool allowsLogging;
		QString canonicalName;

		Data();

		bool hasBroadcast() const
		{
			return broadcast != nullptr;
		}

		bool hasMasterClient() const
		{
			return masterClient != nullptr;
		}
	};

	EnginePlugin();
	virtual ~EnginePlugin();

	/**
	 * Initializes a plugin based on a feature set passed in. A name for
	 * the plugin and its icon (in XPM format) must be provided. The list
	 * of features must be terminated with EP_Done.
	 *
	 * @see InitFeatures
	 * @param name Name of the plugin which will be presented to the user.
	 * @param icon XPM icon used to represent this plugin.
	 */
	void init(const char *name, const char *const icon[], ...);

	/**
	 *	@brief Engine's configuration widget.
	 */
	virtual ConfigPage *configuration(QWidget *parent);

	/**
	 * @brief Creates a list of custom Create Game dialog pages.
	 *
	 * These pages need to be instantiated with the passed pointer as
	 * a parent. Once instantiated, Doomseeker will take care of the
	 * deletion of their objects. Pages are valid only as long as the
	 * dialog box itself is valid.
	 *
	 * Pages are always inserted before the "Custom parameters" page
	 * in the Create Game dialog.
	 *
	 * Default behavior assumes that the plugin doesn't define any custom
	 * pages and returns an empty list.
	 *
	 * @see CreateServerDialog
	 * @see CreateServerDialogPage
	 */
	virtual QList<CreateServerDialogPage *> createServerDialogPages(CreateServerDialog *pDialog)
	{
		Q_UNUSED(pDialog);
		return QList<CreateServerDialogPage *>();
	}

	const Data *data() const
	{
		return d;
	}
	const QPixmap &icon() const
	{
		return *d->icon;
	}
	void setConfig(IniSection &cfg);

	/**
	 * @brief Game settings flags.
	 */
	virtual QList<DMFlagsSection> dmFlags() const;

	GameExeFactory *gameExe();

	/**
	 * @brief Creates an instance of GameHost derivative class.
	 *
	 * Gets a pointer to a new instance of GameHost's
	 * descendant (defined by a plugin). Created instance should be deleted
	 * manually by the programmer.
	 */
	virtual GameHost *gameHost();

	/**
	 * @brief Game modes (cooperative, deathmatch, ctf).
	 */
	virtual QList<GameMode> gameModes() const;
	/**
	 * @brief Modifier that apply to all game modes (ex. instagib).
	 *
	 * Modifiers are used and displayed in Create Game dialog.
	 * If an empty list (or nullptr) is returned, Modifier combo box
	 * will be disabled.
	 */
	virtual QList<GameCVar> gameModifiers() const;

	/**
	 *	@brief Returns a list of limits (like fraglimit) supported by passed
	 *	gamemode.
	 */
	virtual QList<GameCVar> limits(const GameMode &mode) const
	{
		Q_UNUSED(mode);
		return QList<GameCVar>();
	}

	/**
	 * Fills the variables with information about the master's address.
	 */
	void masterHost(QString &host, unsigned short &port) const;

	/**
	 * @brief Either specified explicitly by plugin or derived from
	 * the actual plugin name.
	 *
	 * If plugin specifies a canonical name, it's taken as-is.
	 *
	 * If name is derived from the actual name, the following rules apply:
	 * - All characters are lower-cased.
	 * - All whitespace is replaced with underscores.
	 *
	 * Doomseeker uses this name when loading translation files
	 * for plugins.
	 */
	QString nameCanonical() const;

	/**
	 * @brief Creates an instance of server object from this plugin.
	 *
	 * This might be useful for custom servers.
	 *
	 * @return instance of plugin's server object
	 */
	virtual ServerPtr server(const QHostAddress &address, unsigned short port) const;

	/**
	 * @brief Start services, init data structures.
	 *
	 * This is a good place to initialize whatever cannot be
	 * initialized in the constructor. As constructor is called
	 * during DLL init, it might be a bad place to call certain
	 * functions. For example, calling QObject::tr() may lead to
	 * Access Violation exception and failure to load the plugin
	 * in some compiler/OS environment setups (Visual Studio 2013
	 * in Debug mode). start() is a safer place to do this. It's
	 * also a good place to start LAN broadcast capture.
	 *
	 * Default implementation does nothing but reimplementations
	 * should still call the original implementation for future
	 * compatibility.
	 */
	virtual void start();

protected:
	/**
	 * @brief Create an instance of local Server subclass and return
	 *        a ServerPtr.
	 */
	virtual ServerPtr mkServer(const QHostAddress &address, unsigned short port) const = 0;
	void initDefaultGameFiles();
	void setGameExeFactory(QSharedPointer<GameExeFactory> factory);

private:
	Data *d;

	QStringList collectKnownPaths(const IniSection &ini) const;
	void findGameFiles(IniSection &ini);
};

extern "C" PLUGIN_EXPORT EnginePlugin *doomSeekerInit();

#endif
