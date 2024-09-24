//------------------------------------------------------------------------------
// doomseekerconfig.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __DOOMSEEKERCONFIG_H__
#define __DOOMSEEKERCONFIG_H__

#include "customservers.h"
#include "dptr.h"
#include "gui/entity/serverlistfilterinfo.h"
#include "patternlist.h"
#include <QScopedPointer>
#include <QSettings>
#include <QString>
#include <QStringList>

#define gConfig DoomseekerConfig::config()

class ColumnSort;
class Ini;
class EnginePlugin;
class FileAlias;
class FileSearchPath;
struct QuerySpeed;
class SettingsProviderQt;

/**
 *	@brief This Singleton holds entire Doomseeker configuration in memory.
 *
 *	The first time it is instantiated the cfg variables are set to default
 *	values. Then they can be either read from or saved to a .ini file.
 *	The save is handled internally and external access to the .ini file
 *	should remain minimal.
 */
class DoomseekerConfig
{
public:
	class DoomseekerCfg
	{
	public:
		static const QString OS_USERNAME_TMPL;
		static const QString SECTION_NAME;

		bool bBotsAreNotPlayers;
		bool bCloseToTrayIcon;
		bool bColorizeServerConsole;
		bool bDrawGridInServerTable;
		bool bHidePasswords;
		bool bHonorServerCountries;
		bool bIP2CountryAutoUpdate;
		bool bLogCreatedServer;
		bool bLookupHosts;
		bool bMarkServersWithBuddies;
		bool bQueryAutoRefreshDontIfActive;
		bool bQueryAutoRefreshEnabled;
		bool bQueryBeforeLaunch;
		bool bQueryOnStartup;
		bool bRecordDemo;
		bool bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn;
		bool bCheckTheIntegrityOfWads;
		bool bResolveTemplatedPathsPlaceholders;
		PatternList buddies;
		bool bUseTrayIcon;
		QString buddyServersColor;
		QVector<CustomServerInfo> customServers;
		QString customServersColor;
		QString lanServersColor;
		QString localization;
		QString mainWindowState;
		QByteArray mainWindowGeometry;
		QByteArray demoManagerGeometry;
		QByteArray demoManagerSplitterState;
		unsigned queryAutoRefreshEverySeconds;
		QString playerName;
		QString previousCreateServerConfigDir;
		QString previousCreateServerExecDir;
		QString previousCreateServerLogDir;
		QString previousCreateServerWadDir;
		QString previousDemoExportDir;
		QString serverListColumnState;
		int serverListSortIndex;
		int serverListSortDirection;
		QString slotStyle;
		QList<FileSearchPath> wadPaths;

		DoomseekerCfg();
		~DoomseekerCfg();

		QList<ColumnSort> additionalSortColumns() const;
		void setAdditionalSortColumns(const QList<ColumnSort> &val);

		void enableFreedoomInstallation(const QString &dir);

		const QuerySpeed &querySpeed() const;
		void setQuerySpeed(const QuerySpeed &val);

		/**
		 * Return the player name as configured and resolved.
		 *
		 * If player name is set to OS_USERNAME_TMPL, it returns
		 * the name of the OS user. Otherwise, it returns the
		 * name as configured.
		 */
		QString realPlayerName() const;

		QList<FileAlias> wadAliases() const;
		void setWadAliases(const QList<FileAlias> &val);

		QStringList wadPathsOnly() const;

		/**
		 *	@brief Initializes values that are not present in the section
		 *	yet.
		 *
		 *	All values already present are left alone.
		 */
		void init(IniSection &section);
		void initWadAlias();
		void load(IniSection &section);
		void save(IniSection &section);

	private:
		DPtr<DoomseekerCfg> d;
	};

	class AutoUpdates
	{
	public:
		enum UpdateMode
		{
			/**
			 * @brief No auto updates.
			 */
			UM_Disabled = 0,
			/**
			 * @brief User is only notified of the update and decides if
			 *        it should be installed.
			 */
			UM_NotifyOnly = 1,
			/**
			 * @brief Update is fully automatic.
			 */
			UM_FullAuto = 2
		};

		static const QString SECTION_NAME;

		/**
		 * @brief Used to check if there are new updates since
		 *        last reject.
		 *
		 * - Key - package name.
		 * - Value - package revision.
		 */
		QMap<QString, QString> lastKnownUpdateRevisions;
		UpdateMode updateMode;
		/**
		 * @brief Pass this to UpdateChannel::fromName() .
		 */
		QString updateChannelName;
		bool bPerformUpdateOnNextRun;

		void init(IniSection &section);
		void load(IniSection &section);
		void save(IniSection &section);
	};

	class ServerFilter
	{
	public:
		static const QString SECTION_NAME;

		ServerListFilterInfo info;
		QString currentPreset;
		QVariantMap presets;

		void init(IniSection &section);
		void load(IniSection &section);
		void save(IniSection &section);
	};

	class WadseekerCfg
	{
	public:
		static const QString SECTION_NAME;

		bool bAlwaysUseDefaultSites;
		bool bSearchInIdgames;
		QString colorMessageCriticalError;
		QString colorMessageError;
		QString colorMessageNotice;
		QString colorMessageNavigation;
		int maxConcurrentSiteDownloads;
		int maxConcurrentWadDownloads;
		QString idgamesURL;
		QStringList searchURLs;
		QString targetDirectory;

		WadseekerCfg();

		/**
		 *	@brief Initializes values that are not present in the section
		 *	yet.
		 *
		 *	All values already present are left alone.
		 */
		void init(IniSection &section);
		void load(IniSection &section);
		void save(IniSection &section);
	};

	/**
	 *	@brief Returns the Singleton.
	 */
	static DoomseekerConfig &config();

	/**
	 *	@brief Disposes of the Singleton.
	 *
	 *	This will @b NOT save the current config to a file.
	 *	It is safe to call even if the config was not initialized
	 *	first.
	 */
	static void dispose();

	DoomseekerCfg doomseeker;
	AutoUpdates autoUpdates;
	ServerFilter serverFilter;
	WadseekerCfg wadseeker;

	Ini *ini()
	{
		return this->pIni.data();
	}

	/**
	 *	@brief This will assume that the .ini file is initialized.
	 *
	 *	If the pIni pointer is nullptr this will call setIniFile() with
	 *	empty string parameter to prevent crash.
	 *
	 *	@param pluginName
	 *		Name of the plugin, equals name of the Ini section stripped
	 *		of all spacebars. Cannot be equal to "wadseeker" or
	 *		"doomseeker". If invalid pluginName is specified a dummy
	 *		trashcan IniSection is returned and a proper log message
	 *		is generated.
	 *
	 *	@see dummySection
	 */
	IniSection iniSectionForPlugin(const QString &pluginName);
	IniSection iniSectionForPlugin(const EnginePlugin *plugin);

	/**
	 *	@brief Reads settings from ini file. This file must be
	 *	previously set by setIniFile() method.
	 */
	bool readFromFile();

	/**
	 *	@brief Saves current settings to ini file. This file must
	 *	be previously set by setIniFile() method.
	 */
	bool saveToFile();

	/**
	 *	@brief Initializes the Ini class instance to point
	 *	to a file.
	 *
	 *	This method will properly dispose of the previous
	 *	ini file. However it will neither save the old ini file
	 *	or read the new one.
	 */
	bool setIniFile(const QString &filePath);

	QList<FileSearchPath> combinedWadseekPaths() const;

private:
	static DoomseekerConfig *instance;

	bool isValidPluginName(const QString &pluginName) const;

	/**
	 *	@brief Exists to allow iniSectionForPlugin() to return
	 *	a trash can if meaningless plugin name is specified.
	 */
	IniSection *dummySection;

	QScopedPointer<QSettings> settings;
	QScopedPointer<SettingsProviderQt> settingsProvider;
	QScopedPointer<Ini> pIni;

	DoomseekerConfig();
	~DoomseekerConfig();
};

#endif
