//------------------------------------------------------------------------------
// doomseekerconfig.cpp
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
#include "doomseekerconfig.h"

#include "configuration/queryspeed.h"
#include "datapaths.h"
#include "fileutils.h"
#include "gui/models/serverlistproxymodel.h"
#include "ini/ini.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "ini/settingsproviderqt.h"
#include "localizationinfo.h"
#include "log.h"
#include "pathfinder/filealias.h"
#include "pathfinder/filesearchpath.h"
#include "plugins/engineplugin.h"
#include "scanner.h"
#include "strings.hpp"
#include "updater/updatechannel.h"
#include "version.h"
#include "wadseeker/wadseeker.h"

#include <QLocale>
/**
 * @deprecated This format for this config entry was valid before version 1.2.
 */
static PatternList readPre1Point2BuddiesList(const QString &configEntry)
{
	PatternList patterns;

	Scanner listReader(configEntry.toUtf8().constData(), configEntry.length());
	// Syntax: {basic|advanced} "pattern";...
	while (listReader.tokensLeft())
	{
		if (!listReader.checkToken(TK_Identifier))
			break; // Invalid so lets just use what we have.

		QRegExp::PatternSyntax syntax;
		if (listReader->str().compare("basic") == 0)
			syntax = QRegExp::Wildcard;
		else
			syntax = QRegExp::RegExp;

		if (!listReader.checkToken(TK_StringConst))
			break;

		QRegExp pattern(listReader->str(), Qt::CaseInsensitive, syntax);
		if (pattern.isValid())
			patterns << pattern;

		if (!listReader.checkToken(';'))
			break;
	}
	return patterns;
}
//////////////////////////////////////////////////////////////////////////////
DoomseekerConfig *DoomseekerConfig::instance = nullptr;

DoomseekerConfig::DoomseekerConfig()
{
	this->dummySection = new IniSection(nullptr, QString());
}

DoomseekerConfig::~DoomseekerConfig()
{
	delete this->dummySection;
}

DoomseekerConfig &DoomseekerConfig::config()
{
	if (instance == nullptr)
		instance = new DoomseekerConfig();

	return *instance;
}

void DoomseekerConfig::dispose()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

IniSection DoomseekerConfig::iniSectionForPlugin(const QString &pluginName)
{
	if (pluginName.isEmpty())
	{
		gLog << QObject::tr("DoomseekerConfig.iniSectionForPlugin(): empty plugin name has been specified, returning dummy IniSection.");
		return *dummySection;
	}

	if (!isValidPluginName(pluginName))
	{
		gLog << QObject::tr("DoomseekerConfig.iniSectionForPlugin(): plugin name is invalid: %1").arg(pluginName);
		return *dummySection;
	}

	if (this->pIni == nullptr)
		setIniFile("");

	QString sectionName = pluginName;
	sectionName = sectionName.replace(' ', "");
	return this->pIni->section(sectionName);
}

IniSection DoomseekerConfig::iniSectionForPlugin(const EnginePlugin *plugin)
{
	return iniSectionForPlugin(plugin->data()->name);
}

bool DoomseekerConfig::isValidPluginName(const QString &pluginName) const
{
	QString invalids[] = { "doomseeker", "wadseeker", "" };

	for (int i = 0; !invalids[i].isEmpty(); ++i)
	{
		if (pluginName.compare(invalids[i], Qt::CaseInsensitive) == 0)
			return false;
	}

	return true;
}

bool DoomseekerConfig::readFromFile()
{
	if (pIni == nullptr)
		return false;

	IniSection sectionDoomseeker = pIni->section(doomseeker.SECTION_NAME);
	doomseeker.load(sectionDoomseeker);

	IniSection sectionServerFilter = pIni->section(serverFilter.SECTION_NAME);
	serverFilter.load(sectionServerFilter);

	IniSection sectionWadseeker = pIni->section(wadseeker.SECTION_NAME);
	wadseeker.load(sectionWadseeker);

	IniSection sectionAutoUpdates = pIni->section(autoUpdates.SECTION_NAME);
	autoUpdates.load(sectionAutoUpdates);

	// Plugins should read their sections manually.

	return true;
}

bool DoomseekerConfig::saveToFile()
{
	if (pIni == nullptr)
		return false;

	/*
	TODO:
	Find a way to work around this.
	const QString TOP_COMMENT = QObject::tr("This is %1 configuration file.\n\
	Any modification done manually to this file is on your own risk.").arg(Version::fullVersionInfo());

	pIni->setIniTopComment(TOP_COMMENT);
	*/
	IniSection sectionDoomseeker = pIni->section(doomseeker.SECTION_NAME);
	doomseeker.save(sectionDoomseeker);

	IniSection sectionServerFilter = pIni->section(serverFilter.SECTION_NAME);
	serverFilter.save(sectionServerFilter);

	IniSection sectionWadseeker = pIni->section(wadseeker.SECTION_NAME);
	wadseeker.save(sectionWadseeker);

	IniSection sectionAutoUpdates = pIni->section(autoUpdates.SECTION_NAME);
	autoUpdates.save(sectionAutoUpdates);

	// Plugins should save their sections manually.
	if (this->settings->isWritable())
	{
		this->settings->sync();
		return true;
	}
	return false;
}

bool DoomseekerConfig::setIniFile(const QString &filePath)
{
	// Delete old instances if necessary.
	this->pIni.reset();
	this->settingsProvider.reset();
	this->settings.reset();

	gLog << QObject::tr("Setting INI file: %1").arg(filePath);
	// Create new instances.
	this->settings.reset(new QSettings(filePath, QSettings::IniFormat));
	this->settingsProvider.reset(new SettingsProviderQt(this->settings.data()));
	this->pIni.reset(new Ini(this->settingsProvider.data()));

	// Init.
	IniSection section;

	section = this->pIni->section(doomseeker.SECTION_NAME);
	doomseeker.init(section);

	section = this->pIni->section(serverFilter.SECTION_NAME);
	serverFilter.init(section);

	section = this->pIni->section(wadseeker.SECTION_NAME);
	wadseeker.init(section);

	section = this->pIni->section(autoUpdates.SECTION_NAME);
	autoUpdates.init(section);

	return true;
}

QList<FileSearchPath> DoomseekerConfig::combinedWadseekPaths() const
{
	QList<FileSearchPath> paths = doomseeker.wadPaths;
	paths << wadseeker.targetDirectory;
	return paths;
}

//////////////////////////////////////////////////////////////////////////////
DClass<DoomseekerConfig::DoomseekerCfg>
{
public:
	IniSection section;
	QuerySpeed querySpeed;

	QString slotStyle() const
	{
		// Slot styles were indexed in older versions of Doomseeker.
		// This here provides compatibility layer that allows to load configuration
		// files from those versions.
		const int NUM_SLOTSTYLES = 2;
		const char *indexedSlotStyles[NUM_SLOTSTYLES] = { "marines", "blocks" };
		bool isInt = false;
		int numeric = section["SlotStyle"].value().toInt(&isInt);
		if (isInt && numeric >= 0 && numeric < NUM_SLOTSTYLES)
			return indexedSlotStyles[numeric];
		else
			return section["SlotStyle"].valueString();
	}
};

DPointered(DoomseekerConfig::DoomseekerCfg)

const QString DoomseekerConfig::DoomseekerCfg::SECTION_NAME = "Doomseeker";

DoomseekerConfig::DoomseekerCfg::DoomseekerCfg()
{
	this->bBotsAreNotPlayers = true;
	this->bCloseToTrayIcon = false;
	this->bColorizeServerConsole = true;
	this->bDrawGridInServerTable = false;
	this->bHidePasswords = false;
	this->bGroupServersWithPlayersAtTheTopOfTheList = true;
	this->bIP2CountryAutoUpdate = true;
	this->bLogCreatedServer = false;
	this->bLookupHosts = true;
	this->bQueryAutoRefreshDontIfActive = true;
	this->bQueryAutoRefreshEnabled = false;
	this->bQueryBeforeLaunch = true;
	this->bQueryOnStartup = true;
	this->bRecordDemo = false;
	this->bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn = true;
	this->bCheckTheIntegrityOfWads = true;
	this->bUseTrayIcon = false;
	this->bMarkServersWithBuddies = true;
	this->buddyServersColor = "#5ecf75";
	this->customServersColor = "#ffaa00";
	this->lanServersColor = "#92ebe5";
	this->localization = LocalizationInfo::SYSTEM_FOLLOW.localeName;
	this->mainWindowState = "";
	this->mainWindowGeometry = "";
	this->queryAutoRefreshEverySeconds = 180;
	setQuerySpeed(QuerySpeed::aggressive());
	this->previousCreateServerConfigDir = "";
	this->previousCreateServerExecDir = "";
	this->previousCreateServerLogDir = "";
	this->previousCreateServerWadDir = "";
	this->slotStyle = 1;
	this->serverListSortIndex = -1;
	this->serverListSortDirection = Qt::DescendingOrder;
	this->wadPaths = FileSearchPath::fromStringList(gDefaultDataPaths->defaultWadPaths());
}

DoomseekerConfig::DoomseekerCfg::~DoomseekerCfg()
{
}

QList<ColumnSort> DoomseekerConfig::DoomseekerCfg::additionalSortColumns() const
{
	QList<ColumnSort> list;
	QVariantList varList = d->section.value("AdditionalSortColumns").toList();
	for (const QVariant &var : varList)
	{
		list << ColumnSort::deserializeQVariant(var);
	}
	return list;
}

void DoomseekerConfig::DoomseekerCfg::setAdditionalSortColumns(const QList<ColumnSort> &val)
{
	QVariantList varList;
	for (const ColumnSort &elem : val)
	{
		varList << elem.serializeQVariant();
	}
	d->section.setValue("AdditionalSortColumns", varList);
}

void DoomseekerConfig::DoomseekerCfg::init(IniSection &section)
{
	// TODO: Make all methods use d->section
	d->section = section;
	section.createSetting("Localization", this->localization);
	section.createSetting("BotsAreNotPlayers", this->bBotsAreNotPlayers);
	section.createSetting("CloseToTrayIcon", this->bCloseToTrayIcon);
	section.createSetting("ColorizeServerConsole", this->bColorizeServerConsole);
	section.createSetting("DrawGridInServerTable", this->bDrawGridInServerTable);
	section.createSetting("HidePasswords", this->bHidePasswords);
	section.createSetting("GroupServersWithPlayersAtTheTopOfTheList", this->bGroupServersWithPlayersAtTheTopOfTheList);
	section.createSetting("IP2CAutoUpdate", this->bIP2CountryAutoUpdate);
	section.createSetting("LogCreatedServers", this->bLogCreatedServer);
	section.createSetting("LookupHosts", this->bLookupHosts);
	section.createSetting("QueryAutoRefreshDontIfActive", this->bQueryAutoRefreshDontIfActive);
	section.createSetting("QueryAutoRefreshEnabled", this->bQueryAutoRefreshEnabled);
	section.createSetting("QueryOnStartup", this->bQueryOnStartup);
	section.createSetting("RecordDemo", this->bRecordDemo);
	section.createSetting("TellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn", this->bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn);
	section.createSetting("CheckTheIntegrityOfWads", this->bCheckTheIntegrityOfWads);
	section.createSetting("UseTrayIcon", this->bUseTrayIcon);
	section.createSetting("MarkServersWithBuddies", this->bMarkServersWithBuddies);
	section.createSetting("BuddyServersColor", this->buddyServersColor);
	section.createSetting("CustomServersColor", this->customServersColor);
	section.createSetting("LanServersColor", this->lanServersColor);
	section.createSetting("QueryAutoRefreshEverySeconds", this->queryAutoRefreshEverySeconds);
	section.createSetting("QueryServerInterval", this->querySpeed().intervalBetweenServers);
	section.createSetting("QueryServerTimeout", this->querySpeed().delayBetweenSingleServerAttempts);
	section.createSetting("QueryAttemptsPerServer", this->querySpeed().attemptsPerServer);
	section.createSetting("SlotStyle", this->slotStyle);
	section.createSetting("ServerListSortIndex", this->serverListSortIndex);
	section.createSetting("ServerListSortDirection", this->serverListSortDirection);
	section.createSetting("WadPaths", FileSearchPath::toVariantList(this->wadPaths));

	initWadAlias();
}

void DoomseekerConfig::DoomseekerCfg::initWadAlias()
{
	if (!d->section.hasSetting("WadAliases"))
		setWadAliases(FileAlias::standardWadAliases());
}

void DoomseekerConfig::DoomseekerCfg::load(IniSection &section)
{
	this->localization = (const QString &)section["Localization"];
	this->bBotsAreNotPlayers = section["BotsAreNotPlayers"];
	this->bCloseToTrayIcon = section["CloseToTrayIcon"];
	this->bColorizeServerConsole = section["ColorizeServerConsole"];
	this->bDrawGridInServerTable = section["DrawGridInServerTable"];
	this->bHidePasswords = section["HidePasswords"];
	this->bGroupServersWithPlayersAtTheTopOfTheList = section["GroupServersWithPlayersAtTheTopOfTheList"];
	this->bIP2CountryAutoUpdate = section["IP2CAutoUpdate"];
	this->bLogCreatedServer = section["LogCreatedServers"];
	this->bLookupHosts = section["LookupHosts"];
	this->bQueryAutoRefreshDontIfActive = section["QueryAutoRefreshDontIfActive"];
	this->bQueryAutoRefreshEnabled = section["QueryAutoRefreshEnabled"];
	this->bQueryBeforeLaunch = section["QueryBeforeLaunch"];
	this->bQueryOnStartup = section["QueryOnStartup"];
	this->bRecordDemo = section["RecordDemo"];
	this->bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn = section["TellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn"];
	this->bCheckTheIntegrityOfWads = section["CheckTheIntegrityOfWads"];
	this->bUseTrayIcon = section["UseTrayIcon"];
	this->bMarkServersWithBuddies = section["MarkServersWithBuddies"];
	this->buddyServersColor = (const QString &)section["BuddyServersColor"];
	this->customServersColor = (const QString &)section["CustomServersColor"];
	this->lanServersColor = (const QString &)section["LanServersColor"];
	this->mainWindowState = (const QString &)section["MainWindowState"];
	this->mainWindowGeometry = section.value("MainWindowGeometry", "").toByteArray();
	this->queryAutoRefreshEverySeconds = section["QueryAutoRefreshEverySeconds"];
	d->querySpeed.intervalBetweenServers = section["QueryServerInterval"];
	d->querySpeed.delayBetweenSingleServerAttempts = section["QueryServerTimeout"];
	d->querySpeed.attemptsPerServer = section["QueryAttemptsPerServer"];
	this->previousCreateServerConfigDir = (const QString &)section["PreviousCreateServerConfigDir"];
	this->previousCreateServerExecDir = (const QString &)section["PreviousCreateServerExecDir"];
	this->previousCreateServerLogDir = (const QString &)section["PreviousCreateServerLogDir"];
	this->previousCreateServerWadDir = (const QString &)section["PreviousCreateServerWadDir"];
	this->serverListColumnState = (const QString &)section["ServerListColumnState"];
	this->serverListSortIndex = section["ServerListSortIndex"];
	this->serverListSortDirection = section["ServerListSortDirection"];
	this->slotStyle = d->slotStyle();

	// Complex data variables.

	// Custom servers
	// CustomServers2 is for Doomseeker >= 1.2.
	// Deserialization of this setting is not backwards-compatible,
	// hence we need an extra compatibility layer. The amount of
	// information that the user could potentially lose if I had
	// been too lazy to code this would be insufferable.
	QList<CustomServerInfo> customServersList;
	CustomServers::decodeConfigEntries(section["CustomServers2"], customServersList);
	QList<CustomServerInfo> backwardCompatibleServers;
	CustomServers::decodeConfigEntries(section["CustomServers"], backwardCompatibleServers);
	for (const CustomServerInfo &backwardCompatibleServer : backwardCompatibleServers)
	{
		bool known = false;
		for (const CustomServerInfo &knownServer : customServersList)
		{
			if (knownServer.isSameServer(backwardCompatibleServer))
			{
				known = true;
				break;
			}
		}
		if (!known)
			customServersList << backwardCompatibleServer;
	}
	this->customServers = customServersList.toVector();

	// WAD paths
	// Backward compatibility, XXX once new stable is released:
	QVariant variantWadPaths = section["WadPaths"].value();
	if (variantWadPaths.isValid() && variantWadPaths.toList().isEmpty())
	{
		// Backward compatibility continued:
		wadPaths.clear();
		QStringList paths = variantWadPaths.toString().split(";");
		for (const QString &path : paths)
		{
			wadPaths << FileSearchPath(path);
		}
	}
	else
	{
		// This is not a part of XXX, this is proper, current behavior:
		wadPaths = FileSearchPath::fromVariantList(section["WadPaths"].value().toList());
	}
	// End of backward compatibility for WAD paths.

	// Buddies list
	if (section.hasSetting("Buddies"))
		this->buddies = PatternList::deserializeQVariant(section.value("Buddies"));
	else if (section.hasSetting("BuddiesList"))
	{
		// Backward compatibility, pre 1.2.
		this->buddies = readPre1Point2BuddiesList(section["BuddiesList"]);
	}
}

void DoomseekerConfig::DoomseekerCfg::save(IniSection &section)
{
	section["Localization"] = this->localization;
	section["BotsAreNotPlayers"] = this->bBotsAreNotPlayers;
	section["CloseToTrayIcon"] = this->bCloseToTrayIcon;
	section["ColorizeServerConsole"] = this->bColorizeServerConsole;
	section["DrawGridInServerTable"] = this->bDrawGridInServerTable;
	section["HidePasswords"] = this->bHidePasswords;
	section["GroupServersWithPlayersAtTheTopOfTheList"] = this->bGroupServersWithPlayersAtTheTopOfTheList;
	section["IP2CAutoUpdate"] = this->bIP2CountryAutoUpdate;
	section["LogCreatedServers"] = this->bLogCreatedServer;
	section["LookupHosts"] = this->bLookupHosts;
	section["QueryAutoRefreshDontIfActive"] = this->bQueryAutoRefreshDontIfActive;
	section["QueryAutoRefreshEnabled"] = this->bQueryAutoRefreshEnabled;
	section["QueryBeforeLaunch"] = this->bQueryBeforeLaunch;
	section["QueryOnStartup"] = this->bQueryOnStartup;
	section["RecordDemo"] = this->bRecordDemo;
	section["TellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn"] = this->bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn;
	section["CheckTheIntegrityOfWads"] = this->bCheckTheIntegrityOfWads;
	section["UseTrayIcon"] = this->bUseTrayIcon;
	section["MarkServersWithBuddies"] = this->bMarkServersWithBuddies;
	section["BuddyServersColor"] = this->buddyServersColor;
	section["CustomServersColor"] = this->customServersColor;
	section["LanServersColor"] = this->lanServersColor;
	section["MainWindowState"] = this->mainWindowState;
	section.setValue("MainWindowGeometry", this->mainWindowGeometry);
	section["QueryAutoRefreshEverySeconds"] = this->queryAutoRefreshEverySeconds;
	section["QueryServerInterval"] = this->querySpeed().intervalBetweenServers;
	section["QueryServerTimeout"] = this->querySpeed().delayBetweenSingleServerAttempts;
	section["QueryAttemptsPerServer"] = this->querySpeed().attemptsPerServer;
	section["PreviousCreateServerConfigDir"] = this->previousCreateServerConfigDir;
	section["PreviousCreateServerExecDir"] = this->previousCreateServerExecDir;
	section["PreviousCreateServerLogDir"] = this->previousCreateServerLogDir;
	section["PreviousCreateServerWadDir"] = this->previousCreateServerWadDir;
	section["ServerListColumnState"] = this->serverListColumnState;
	section["ServerListSortIndex"] = this->serverListSortIndex;
	section["ServerListSortDirection"] = this->serverListSortDirection;
	section["SlotStyle"] = this->slotStyle;

	// Complex data variables.

	// Custom servers
	QStringList allCustomServers; // backward compatibility for Doomseeker <1.2
	QStringList allCustomServers2; // Doomseeker >=1.2
	for (const CustomServerInfo &customServer : this->customServers)
	{
		QString engineName = QUrl::toPercentEncoding(customServer.engine, "", "()");
		QString address = QUrl::toPercentEncoding(customServer.host, "", "()");

		QString customServerStringPrefix = QString("(%1;%2;%3")
			.arg(engineName).arg(address)
			.arg(customServer.port);
		QString customServerString2 = QString("%1;%2)")
			.arg(customServerStringPrefix)
			.arg(customServer.enabled ? 1 : 0);

		allCustomServers << customServerStringPrefix + ")";
		allCustomServers2 << customServerString2;
	}
	section["CustomServers"] = allCustomServers.join(";");
	section["CustomServers2"] = allCustomServers2.join(";");

	section["WadPaths"].setValue(FileSearchPath::toVariantList(this->wadPaths));
	section["Buddies"].setValue(this->buddies.serializeQVariant());
}

const QuerySpeed &DoomseekerConfig::DoomseekerCfg::querySpeed() const
{
	return d->querySpeed;
}

void DoomseekerConfig::DoomseekerCfg::setQuerySpeed(const QuerySpeed &val)
{
	d->querySpeed = val;
}

QList<FileAlias> DoomseekerConfig::DoomseekerCfg::wadAliases() const
{
	QList<FileAlias> list;
	QVariantList varList = d->section.value("WadAliases").toList();
	for (const QVariant &var : varList)
	{
		list << FileAlias::deserializeQVariant(var);
	}
	return FileAliasList::mergeDuplicates(list);
}

void DoomseekerConfig::DoomseekerCfg::setWadAliases(const QList<FileAlias> &val)
{
	QVariantList varList;
	for (const FileAlias &elem : val)
	{
		varList << elem.serializeQVariant();
	}
	d->section.setValue("WadAliases", varList);
}

void DoomseekerConfig::DoomseekerCfg::enableFreedoomInstallation(const QString &dir)
{
	if (!FileUtils::containsPath(wadPathsOnly(), dir))
		wadPaths.prepend(dir);
	QList<FileAlias> aliases = wadAliases();
	aliases << FileAlias::freeDoom1Aliases();
	aliases << FileAlias::freeDoom2Aliases();
	aliases = FileAliasList::mergeDuplicates(aliases);
	setWadAliases(aliases);
}

QStringList DoomseekerConfig::DoomseekerCfg::wadPathsOnly() const
{
	QStringList result;
	for (const FileSearchPath &path : wadPaths)
	{
		result << path.path();
	}
	return result;
}
//////////////////////////////////////////////////////////////////////////////
const QString DoomseekerConfig::AutoUpdates::SECTION_NAME = "Doomseeker_AutoUpdates";

void DoomseekerConfig::AutoUpdates::init(IniSection &section)
{
	section.createSetting("UpdateChannelName", UpdateChannel::mkStable().name());
	section.createSetting("UpdateMode", (int) UM_NotifyOnly);
	section.createSetting("LastKnownUpdateRevisions", QVariant());
	section.createSetting("bPerformUpdateOnNextRun", false);
}

void DoomseekerConfig::AutoUpdates::load(IniSection &section)
{
	updateChannelName = (const QString &)section["UpdateChannelName"];
	updateMode = (UpdateMode)section["UpdateMode"].value().toInt();
	QVariantMap lastKnownUpdateRevisionsVariant = section["LastKnownUpdateRevisions"].value().toMap();
	lastKnownUpdateRevisions.clear();
	for (const QString &package : lastKnownUpdateRevisionsVariant.keys())
	{
		QVariant revisionVariant = lastKnownUpdateRevisionsVariant[package];
		lastKnownUpdateRevisions.insert(package, revisionVariant.toString());
	}
	bPerformUpdateOnNextRun = section["bPerformUpdateOnNextRun"].value().toBool();
}

void DoomseekerConfig::AutoUpdates::save(IniSection &section)
{
	section["UpdateChannelName"] = updateChannelName;
	section["UpdateMode"] = updateMode;
	QVariantMap revisionsVariantMap;
	for (const QString &package : lastKnownUpdateRevisions.keys())
	{
		revisionsVariantMap.insert(package, lastKnownUpdateRevisions[package]);
	}
	section["LastKnownUpdateRevisions"].setValue(revisionsVariantMap);
	section["bPerformUpdateOnNextRun"].setValue(bPerformUpdateOnNextRun);
}
//////////////////////////////////////////////////////////////////////////////
const QString DoomseekerConfig::ServerFilter::SECTION_NAME = "ServerFilter";

void DoomseekerConfig::ServerFilter::init(IniSection &section)
{
	section.createSetting("bEnabled", true);
	section.createSetting("bShowEmpty", true);
	section.createSetting("bShowFull", true);
	section.createSetting("bShowOnlyValid", false);
	section.createSetting("GameModes", QStringList());
	section.createSetting("GameModesExcluded", QStringList());
	section.createSetting("MaxPing", 0);
	section.createSetting("ServerName", "");
	section.createSetting("TestingServers", Doomseeker::Indifferent);
	section.createSetting("WADs", QStringList());
	section.createSetting("WADsExcluded", QStringList());
}

void DoomseekerConfig::ServerFilter::load(IniSection &section)
{
	info.bEnabled = section["bEnabled"];
	info.bShowEmpty = section["bShowEmpty"];
	info.bShowFull = section["bShowFull"];
	info.bShowOnlyValid = section["bShowOnlyValid"];
	info.gameModes = section["GameModes"].value().toStringList();
	info.gameModesExcluded = section["GameModesExcluded"].value().toStringList();
	info.maxPing = section["MaxPing"];
	info.serverName = (const QString &)section["ServerName"];
	info.testingServers = static_cast<Doomseeker::ShowMode>(section.value("TestingServers").toInt());
	info.wads = section["WADs"].value().toStringList();
	info.wadsExcluded = section["WADsExcluded"].value().toStringList();
}

void DoomseekerConfig::ServerFilter::save(IniSection &section)
{
	section["bEnabled"] = info.bEnabled;
	section["bShowEmpty"] = info.bShowEmpty;
	section["bShowFull"] = info.bShowFull;
	section["bShowOnlyValid"] = info.bShowOnlyValid;
	section["GameModes"].setValue(info.gameModes);
	section["GameModesExcluded"].setValue(info.gameModesExcluded);
	section["MaxPing"] = info.maxPing;
	section["ServerName"] = info.serverName;
	section["TestingServers"] = info.testingServers;
	section["WADs"].setValue(info.wads);
	section["WADsExcluded"].setValue(info.wadsExcluded);
}
//////////////////////////////////////////////////////////////////////////////
const QString DoomseekerConfig::WadseekerCfg::SECTION_NAME = "Wadseeker";

DoomseekerConfig::WadseekerCfg::WadseekerCfg()
{
	this->bAlwaysUseDefaultSites = true;
	this->bSearchInIdgames = true;
	this->bSearchInWadArchive = true;
	this->colorMessageCriticalError = "#ff0000";
	this->colorMessageError = "#ff0000";
	this->colorMessageNotice = "#000000";
	this->connectTimeoutSeconds = WADSEEKER_CONNECT_TIMEOUT_SECONDS_DEFAULT;
	this->downloadTimeoutSeconds = WADSEEKER_DOWNLOAD_TIMEOUT_SECONDS_DEFAULT;
	this->idgamesURL = Wadseeker::defaultIdgamesUrl();
	this->maxConcurrentSiteDownloads = 3;
	this->maxConcurrentWadDownloads = 2;
	this->targetDirectory = gDefaultDataPaths->programsDataDirectoryPath();

	// Search URLs remains uninitialized here. It will be initialized
	// by init() and then load() since Doomseeker always calls these
	// methods in this order.
}

void DoomseekerConfig::WadseekerCfg::init(IniSection &section)
{
	section.createSetting("AlwaysUseDefaultSites", this->bAlwaysUseDefaultSites);
	section.createSetting("SearchInIdgames", this->bSearchInIdgames);
	section.createSetting("SearchInWadArchive", this->bSearchInWadArchive);
	section.createSetting("ColorMessageCriticalError", this->colorMessageCriticalError);
	section.createSetting("ColorMessageError", this->colorMessageError);
	section.createSetting("ColorMessageNotice", this->colorMessageNotice);
	section.createSetting("ConnectTimeoutSeconds", this->connectTimeoutSeconds);
	section.createSetting("DownloadTimeoutSeconds", this->downloadTimeoutSeconds);
	section.createSetting("IdgamesApiURL", this->idgamesURL);
	section.createSetting("MaxConcurrentSiteDownloads", this->maxConcurrentSiteDownloads);
	section.createSetting("MaxConcurrentWadDownloads", this->maxConcurrentWadDownloads);
	section.createSetting("SearchURLs", Wadseeker::defaultSitesListEncoded().join(";"));
	section.createSetting("TargetDirectory", this->targetDirectory);
}

void DoomseekerConfig::WadseekerCfg::load(IniSection &section)
{
	this->bAlwaysUseDefaultSites = section["AlwaysUseDefaultSites"];
	this->bSearchInIdgames = section["SearchInIdgames"];
	this->bSearchInWadArchive = section["SearchInWadArchive"];
	this->colorMessageCriticalError = (const QString &)section["ColorMessageCriticalError"];
	this->colorMessageError = (const QString &)section["ColorMessageError"];
	this->colorMessageNotice = (const QString &)section["ColorMessageNotice"];
	this->connectTimeoutSeconds = section["ConnectTimeoutSeconds"];
	this->downloadTimeoutSeconds = section["DownloadTimeoutSeconds"];
	this->idgamesURL = (const QString &)section["IdgamesApiURL"];
	this->maxConcurrentSiteDownloads = section["MaxConcurrentSiteDownloads"];
	this->maxConcurrentWadDownloads = section["MaxConcurrentWadDownloads"];
	this->targetDirectory = (const QString &)section["TargetDirectory"];

	// Complex data values
	this->searchURLs.clear();
	QStringList urlList = section["SearchURLs"].valueString().split(";", QString::SkipEmptyParts);
	for (const QString &url : urlList)
	{
		this->searchURLs << QUrl::fromPercentEncoding(url.toUtf8());
	}
}

void DoomseekerConfig::WadseekerCfg::save(IniSection &section)
{
	section["AlwaysUseDefaultSites"] = this->bAlwaysUseDefaultSites;
	section["SearchInIdgames"] = this->bSearchInIdgames;
	section["SearchInWadArchive"] = this->bSearchInWadArchive;
	section["ColorMessageCriticalError"] = this->colorMessageCriticalError;
	section["ColorMessageError"] = this->colorMessageError;
	section["ColorMessageNotice"] = this->colorMessageNotice;
	section["ConnectTimeoutSeconds"] = this->connectTimeoutSeconds;
	section["DownloadTimeoutSeconds"] = this->downloadTimeoutSeconds;
	section["IdgamesApiURL"] = this->idgamesURL;
	section["MaxConcurrentSiteDownloads"] = this->maxConcurrentSiteDownloads;
	section["MaxConcurrentWadDownloads"] = this->maxConcurrentWadDownloads;
	section["TargetDirectory"] = this->targetDirectory;

	// Complex data values
	QStringList urlEncodedList;
	for (const QString &url : this->searchURLs)
	{
		urlEncodedList << QUrl::toPercentEncoding(url);
	}
	section["SearchURLs"] = urlEncodedList.join(";");
}
