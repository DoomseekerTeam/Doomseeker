//------------------------------------------------------------------------------
// ircconfig.cpp
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
#include "ircconfig.h"

#include "datapaths.h"
#include "ini/settingsproviderqt.h"
#include "irc/configuration/chatnetworkscfg.h"
#include "log.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "version.h"

#include <QMessageBox>

IRCConfig *IRCConfig::instance = nullptr;

IRCConfig::IRCConfig()
{
}

IRCConfig::~IRCConfig()
{
}

IRCConfig &IRCConfig::config()
{
	if (instance == nullptr)
		instance = new IRCConfig();

	return *instance;
}

void IRCConfig::dispose()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

void IRCConfig::loadNetworksFromPlugins()
{
	QList<IRCNetworkEntity> networks = ChatNetworksCfg().networks();
	bool shouldSave = false;
	// Go through the plugins and register their IRC servers.
	for (unsigned int i = 0; i < gPlugins->numPlugins(); i++)
	{
		if (gPlugins->plugin(i)->info()->data()->ircChannels.size() == 0)
			continue;

		// OK so maybe registering only on first run is a good idea after all...
		IniVariable registered = gPlugins->plugin(i)->info()->data()->pConfig->createSetting("IRCRegistered", false);
		if (!registered)
		{
			shouldSave = true;
			registered = true;

			for (const IRCNetworkEntity &entity : gPlugins->plugin(i)->info()->data()->ircChannels)
			{
				// If we have a unique server add it to the list...
				if (!networks.contains(entity))
					networks << entity;
				else
				{
					// otherwise add the channel to the auto join.
					IRCNetworkEntity &existingEntity = networks[networks.indexOf(entity)];
					if (existingEntity.autojoinChannels().contains(entity.autojoinChannels()[0]))
						continue;
					if (existingEntity.isAutojoinNetwork())
					{
						// If we have this set to auto join ask first.
						if (QMessageBox::question(nullptr, QObject::tr("Add plugin's IRC channel?"),
							QObject::tr("Would you like the %1 plugin to add its channel to %2's auto join?")
								.arg(entity.description()).arg(existingEntity.description()),
							QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
							continue;
					}

					existingEntity.autojoinChannels() << entity.autojoinChannels()[0];
				}
			}
		}
	}
	if (shouldSave)
		ChatNetworksCfg().setNetworks(networks);
}

bool IRCConfig::readFromFile()
{
	if (pIni == nullptr)
		return false;

	IniSection section;

	section = pIni->section(AppearanceCfg::SECTION_NAME);
	appearance.load(section);

	section = pIni->section(GeneralCfg::SECTION_NAME);
	general.load(section);

	section = pIni->section(PersonalCfg::SECTION_NAME);
	personal.load(section);

	section = pIni->section(SoundsCfg::SECTION_NAME);
	sounds.load(section);

	loadNetworksFromPlugins();

	return true;
}

bool IRCConfig::saveToFile()
{
	if (pIni == nullptr)
		return false;

	IniSection section;

	section = pIni->section(AppearanceCfg::SECTION_NAME);
	appearance.save(section);

	section = pIni->section(GeneralCfg::SECTION_NAME);
	general.save(section);

	section = pIni->section(PersonalCfg::SECTION_NAME);
	personal.save(section);

	section = pIni->section(SoundsCfg::SECTION_NAME);
	sounds.save(section);

	if (settings->isWritable())
	{
		settings->sync();
		return true;
	}
	return false;
}

bool IRCConfig::setIniFile(const QString &filePath)
{
	pIni.reset();
	settingsProvider.reset();
	settings.reset();

	gLog << QObject::tr("Setting IRC INI file: %1").arg(filePath);
	settings.reset(new QSettings(filePath, QSettings::IniFormat));
	settingsProvider.reset(new SettingsProviderQt(settings.data()));
	pIni.reset(new Ini(settingsProvider.data()));

	IniSection section;

	section = this->pIni->section(AppearanceCfg::SECTION_NAME);
	appearance.init(section);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
const QString IRCConfig::AppearanceCfg::SECTION_NAME = "Appearance";

IRCConfig::AppearanceCfg::AppearanceCfg()
{
	this->backgroundColor = "#000000";
	this->channelActionColor = "#008000";
	this->ctcpColor = "#de5aff";
	this->defaultTextColor = "#b9b9b9";
	this->errorColor = "#ff0000";
	this->mainFont = QFont("Courier");
	this->networkActionColor = "#079CFF";
	this->timestamps = true;
	this->userListFont = QFont("Courier");
	this->userListSelectedTextColor = "#cbcb0f";
	this->userListSelectedBackgroundColor = "#B74600";
	this->urlColor = "#00F6FF";
	windowAlertOnImportantChatEvent = true;
}

void IRCConfig::AppearanceCfg::init(IniSection &section)
{
	section.createSetting("BackgroundColor", this->backgroundColor);
	section.createSetting("ChannelActionColor", this->channelActionColor);
	section.createSetting("CtcpColor", this->ctcpColor);
	section.createSetting("DefaultTextColor", this->defaultTextColor);
	section.createSetting("ErrorColor", this->errorColor);
	section.createSetting("MainFont", this->mainFont.toString());
	section.createSetting("NetworkActionColor", this->networkActionColor);
	section.createSetting("TimeStamps", this->timestamps);
	section.createSetting("UserListFont", this->userListFont.toString());
	section.createSetting("UserListSelectedTextColor", this->userListSelectedTextColor);
	section.createSetting("UserListSelectedBackgroundColor", this->userListSelectedBackgroundColor);
	section.createSetting("UrlColor", this->urlColor);
}

void IRCConfig::AppearanceCfg::load(IniSection &section)
{
	this->backgroundColor = (const QString &)section["BackgroundColor"];
	this->channelActionColor = (const QString &)section["ChannelActionColor"];
	this->ctcpColor = (const QString &)section["CtcpColor"];
	this->defaultTextColor = (const QString &)section["DefaultTextColor"];
	this->errorColor = (const QString &)section["ErrorColor"];
	this->mainFont.fromString(section["MainFont"]);
	this->networkActionColor = (const QString &)section["NetworkActionColor"];
	this->timestamps = section["TimeStamps"];
	this->userListFont.fromString(section["UserListFont"]);
	this->urlColor = (const QString &)section["UrlColor"];
	this->userListSelectedTextColor = (const QString &)section["UserListSelectedTextColor"];
	this->userListSelectedBackgroundColor = (const QString &)section["UserListSelectedBackgroundColor"];
	windowAlertOnImportantChatEvent = section.value("WindowAlertOnImportantChatEvent", true).toBool();
}

void IRCConfig::AppearanceCfg::save(IniSection &section)
{
	section["BackgroundColor"] = this->backgroundColor;
	section["ChannelActionColor"] = this->channelActionColor;
	section["CtcpColor"] = this->ctcpColor;
	section["DefaultTextColor"] = this->defaultTextColor;
	section["ErrorColor"] = this->errorColor;
	section["MainFont"] = this->mainFont.toString();
	section["NetworkActionColor"] = this->networkActionColor;
	section["TimeStamps"] = this->timestamps;
	section["UserListFont"] = this->userListFont.toString();
	section["UserListSelectedTextColor"] = this->userListSelectedTextColor;
	section["UserListSelectedBackgroundColor"] = this->userListSelectedBackgroundColor;
	section["UrlColor"] = this->urlColor;
	section.setValue("WindowAlertOnImportantChatEvent", windowAlertOnImportantChatEvent);
}

//////////////////////////////////////////////////////////////////////////////
const QString IRCConfig::GeneralCfg::SECTION_NAME = "General";

IRCConfig::GeneralCfg::GeneralCfg()
{
}

void IRCConfig::GeneralCfg::load(IniSection &section)
{
	Q_UNUSED(section);
}

void IRCConfig::GeneralCfg::save(IniSection &section)
{
	Q_UNUSED(section);
}
//////////////////////////////////////////////////////////////////////////////
const QString IRCConfig::PersonalCfg::SECTION_NAME = "Personal";

IRCConfig::PersonalCfg::PersonalCfg()
{
}

void IRCConfig::PersonalCfg::load(IniSection &section)
{
	this->alternativeNickname = static_cast<QString>(section["AlternativeNickname"]);
	this->fullName = static_cast<QString>(section["FullName"]);
	this->nickname = static_cast<QString>(section["Nickname"]);
	this->quitMessage = section.value("QuitMessage", "Doomseeker End of Line").toString();
	this->userName = static_cast<QString>(section["UserName"]);
}

void IRCConfig::PersonalCfg::save(IniSection &section)
{
	section["AlternativeNickname"] = this->alternativeNickname;
	section["FullName"] = this->fullName;
	section["Nickname"] = this->nickname;
	section["QuitMessage"] = this->quitMessage;
	section["UserName"] = this->userName;
}
//////////////////////////////////////////////////////////////////////////////
const QString IRCConfig::SoundsCfg::SECTION_NAME = "Sounds";

IRCConfig::SoundsCfg::SoundsCfg()
{
	this->bUseNicknameUsedSound = false;
	this->bUsePrivateMessageReceivedSound = false;
}

void IRCConfig::SoundsCfg::load(IniSection &section)
{
	this->bUseNicknameUsedSound = section["bUseNicknameUsedSound"];
	this->bUsePrivateMessageReceivedSound = section["bUsePrivateMessageReceivedSound"];
	this->nicknameUsedSound = (const QString &)section["NicknameUsedSound"];
	this->privateMessageReceivedSound = (const QString &)section["PrivateMessageReceivedSound"];
}

void IRCConfig::SoundsCfg::save(IniSection &section)
{
	section["bUseNicknameUsedSound"] = this->bUseNicknameUsedSound;
	section["bUsePrivateMessageReceivedSound"] = this->bUsePrivateMessageReceivedSound;
	section["NicknameUsedSound"] = this->nicknameUsedSound;
	section["PrivateMessageReceivedSound"] = this->privateMessageReceivedSound;
}
