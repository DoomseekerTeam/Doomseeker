//------------------------------------------------------------------------------
// engineplugin.cpp
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

#include "gui/configuration/engineconfigpage.h"
#include "ini/ini.h"
#include "irc/entities/ircnetworkentity.h"
#include "log.h"
#include "pathfinder/pathfind.h"
#include "plugins/enginedefaults.h"
#include "plugins/engineplugin.h"
#include "serverapi/gameexefactory.h"
#include "serverapi/gamefile.h"
#include "serverapi/gamehost.h"
#include "serverapi/server.h"
#include "strings.hpp"

#include <cstdarg>
#include <QPixmap>

EnginePlugin::Data::Data()
{
	// Init the defaults.
	allowsConnectPassword = false;
	allowsClientSlots = true;
	allowsPlayerSlots = true;
	allowsEmail = false;
	allowsInternalClients = false;
	allowsJoinPassword = false;
	allowsMOTD = false;
	allowsRConPassword = false;
	allowsURL = false;
	allowsUpnp = false;
	allowsUpnpPort = false;
	allowsLogging = false;
	broadcast = nullptr;
	clientOnly = false;
	createDMFlagsPagesAutomatic = true;
	defaultServerPort = 10666;
	demoExtensionAutomatic = true;
	demoExtension = "lmp";
	hasIwad = true;
	hasMapList = true;
	icon = nullptr;
	inGameFileDownloads = false;
	internalAndSystemClientsDisjoint = false;
	masterClient = nullptr;
	pConfig = nullptr;
	refreshThreshold = 2;
	supportsRandomMapRotation = false;
	valid = true;
	version = 0;
	aboutProvider.reset();
}

////////////////////////////////////////////////////////////////////////////////

EnginePlugin::EnginePlugin()
{
	d = new Data;

	d->gameExeFactory = QSharedPointer<GameExeFactory>(new GameExeFactory(this));
	d->difficulty = QSharedPointer<DefaultDifficultyProvider>(new DefaultDifficultyProvider());

	// At the moment I can't think of how we would support any ABI other than
	// the current, but I suppose we might as well keep track of it?
	d->abiVersion = DOOMSEEKER_ABI_VERSION;
}

EnginePlugin::~EnginePlugin()
{
	delete d->icon;
	delete d->pConfig;
	delete d;
}

ConfigPage *EnginePlugin::configuration(QWidget *parent)
{
	return new EngineConfigPage(this, *d->pConfig, parent);
}

QList<DMFlagsSection> EnginePlugin::dmFlags() const
{
	return QList<DMFlagsSection>();
}

GameExeFactory *EnginePlugin::gameExe()
{
	return data()->gameExeFactory.data();
}

GameHost *EnginePlugin::gameHost()
{
	return new GameHost(this);
}

QList<GameMode> EnginePlugin::gameModes() const
{
	return QList<GameMode>();
}

QList<GameCVar> EnginePlugin::gameModifiers() const
{
	return QList<GameCVar>();
}

void EnginePlugin::init(const char *name, const char *const icon[], ...)
{
	d->name = name;
	d->icon = new QPixmap(icon);
	d->scheme = QString(d->name).replace(' ', "");

	va_list va;
	va_start(va, icon);

	int feature;
	while ((feature = va_arg(va, int)) != EP_Done)
	{
		switch (feature)
		{
		default:
			// Since we don't know if the feature has arguments we must abort.
			gLog << QString("%1 plugin attempted to use unknown feature.").arg(name);
			d->valid = false;
			return;

		case EP_Author:
			d->author = va_arg(va, const char *);
			break;
		case EP_Version:
			d->version = va_arg(va, unsigned int);
			break;
		case EP_AboutProvider:
			d->aboutProvider.reset(va_arg(va, TextProvider *));
			break;

		case EP_AllowsConnectPassword:
			d->allowsConnectPassword = true;
			break;
		case EP_AllowsEmail:
			d->allowsEmail = true;
			break;
		case EP_AllowsInternalClients:
			d->allowsInternalClients = true;
			break;
		case EP_WarningInternalClientsProvider:
			d->warningInternalClientsProvider.reset(va_arg(va, TextProvider *));
			break;
		case EP_InternalAndSystemClientsDisjoint:
			d->internalAndSystemClientsDisjoint = true;
			break;
		case EP_AllowsURL:
			d->allowsURL = true;
			break;
		case EP_AllowsJoinPassword:
			d->allowsJoinPassword = true;
			break;
		case EP_AllowsLogging:
			d->allowsLogging = true;
			break;
		case EP_AllowsRConPassword:
			d->allowsRConPassword = true;
			break;
		case EP_AllowsMOTD:
			d->allowsMOTD = true;
			break;
		case EP_AllowsUpnp:
			d->allowsUpnp = true;
			break;
		case EP_AllowsUpnpPort:
			d->allowsUpnpPort = true;
			break;
		case EP_Broadcast:
			d->broadcast = va_arg(va, Broadcast *);
			break;
		case EP_CanonicalName:
			d->canonicalName = va_arg(va, const char *);
			break;
		case EP_ClientOnly:
			d->clientOnly = true;
			break;
		case EP_DefaultMaster:
			d->defaultMaster = va_arg(va, const char *);
			break;
		case EP_DefaultServerPort:
			d->defaultServerPort = va_arg(va, unsigned int);
			break;
		case EP_DemoExtension:
			d->demoExtensionAutomatic = va_arg(va, unsigned int);
			d->demoExtension = va_arg(va, const char *);
			break;
		case EP_DifficultyProvider:
			d->difficulty = QSharedPointer<GameCVarProvider>(va_arg(va, GameCVarProvider *));
			break;
		case EP_DontCreateDMFlagsPagesAutomatic:
			d->createDMFlagsPagesAutomatic = false;
			break;
		case EP_InGameFileDownloads:
			d->inGameFileDownloads = true;
			break;
		case EP_IRCChannel:
		{
			// Either create an entity or put the channel in an existing one.
			IRCNetworkEntity entity;
			entity.setDescription(va_arg(va, const char *));
			entity.setAddress(va_arg(va, const char *));
			entity.autojoinChannels() << va_arg(va, const char *);

			if (d->ircChannels.contains(entity))
			{
				IRCNetworkEntity &existingEntity = d->ircChannels[d->ircChannels.indexOf(entity)];
				existingEntity.autojoinChannels() << entity.autojoinChannels()[0];
			}
			else
				d->ircChannels << entity;
			break;
		}
		case EP_MasterClient:
			d->masterClient = va_arg(va, MasterClient *);
			break;
		case EP_NoClientSlots:
			d->allowsClientSlots = false;
			break;
		case EP_NoPlayerSlots:
			d->allowsPlayerSlots = false;
			break;
		case EP_NoIwad:
			d->hasIwad = false;
			break;
		case EP_NoMapList:
			d->hasMapList = false;
			break;
		case EP_SupportsRandomMapRotation:
			d->supportsRandomMapRotation = true;
			break;
		case EP_URLScheme:
			d->scheme = va_arg(va, const char *);
			break;
		case EP_RefreshThreshold:
			d->refreshThreshold = va_arg(va, unsigned int);
			break;
		case EP_ClientExeName:
			d->clientExeName = va_arg(va, const char *);
			break;
		case EP_ServerExeName:
			d->serverExeName = va_arg(va, const char *);
			break;
		case EP_GameFileSearchSuffixes:
		{
			QString suffixes = va_arg(va, const char *);
			d->gameFileSearchSuffixes = suffixes.split(";", QString::SkipEmptyParts);
			break;
		}
		}
	}

	va_end(va);
}

void EnginePlugin::masterHost(QString &host, unsigned short &port) const
{
	QString str = d->pConfig->setting("Masterserver");
	Strings::translateServerAddress(str, host, port, d->defaultMaster);
}

QString EnginePlugin::nameCanonical() const
{
	if (!d->canonicalName.isEmpty())
	{
		return d->canonicalName;
	}
	else
	{
		// Derive.
		QString name = data()->name;
		name = name.toLower();
		name = name.replace(QRegExp("\\s"), "_");
		return name;
	}
}

ServerPtr EnginePlugin::server(const QHostAddress &address, unsigned short port) const
{
	ServerPtr server = mkServer(address, port);
	if (server != nullptr)
		server->setSelf(server.toWeakRef());
	return server;
}

void EnginePlugin::setConfig(IniSection &ini)
{
	d->pConfig = new IniSection(ini);

	ini.createSetting("Masterserver", data()->defaultMaster);
	findGameFiles(ini);

	setupConfig(ini);
}

void EnginePlugin::setupConfig(IniSection &config)
{
}

void EnginePlugin::setGameExeFactory(QSharedPointer<GameExeFactory> factory)
{
	d->gameExeFactory = factory;
}

void EnginePlugin::findGameFiles(IniSection &ini)
{
	for (const GameFile &file : gameExe()->gameFiles().asQList())
	{
		if (!ini.hasSetting(file.configName()))
		{
			QString path = PathFind::findGameFile(collectKnownPaths(ini), file);
			ini[file.configName()] = path;
		}
	}
}

QStringList EnginePlugin::collectKnownPaths(const IniSection &ini) const
{
	QStringList paths;
	for (const GameFile &file : data()->gameExeFactory->gameFiles().asQList())
	{
		QString path = ini.retrieveSetting(file.configName()).valueString();
		if (!path.isEmpty())
		{
			paths << path;
		}
	}
	return paths;
}

void EnginePlugin::start()
{
}
