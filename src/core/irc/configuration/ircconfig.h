//------------------------------------------------------------------------------
// ircconfig.h
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
#ifndef __IRCCONFIG_H__
#define __IRCCONFIG_H__

#define gIRCConfig IRCConfig::config()

#include "ini/ini.h"
#include "irc/entities/ircnetworkentity.h"
#include <QFont>
#include <QScopedPointer>
#include <QSettings>

class SettingsProviderQt;

/**
 * @brief This Singleton holds most of Doomseeker IRC configuration in memory.
 *
 * The first time it is instantiated the cfg variables are set to default
 * values. Then they can be either read from or saved to a .ini file.
 * The save is handled internally and external access to the .ini file
 * should remain minimal.
 */
class IRCConfig
{
public:
	class AppearanceCfg
	{
	public:
		static const QString SECTION_NAME;

		QString backgroundColor;
		QString channelActionColor;
		QString ctcpColor;
		QString defaultTextColor;
		QString errorColor;
		QFont mainFont;
		QString networkActionColor;
		bool timestamps;
		QString urlColor;
		QFont userListFont;
		QString userListSelectedTextColor;
		QString userListSelectedBackgroundColor;
		bool windowAlertOnImportantChatEvent;

		AppearanceCfg();

		void init(IniSection &section);
		void load(IniSection &section);
		void save(IniSection &section);
	};

	class GeneralCfg
	{
	public:
		static const QString SECTION_NAME;

		GeneralCfg();

		void load(IniSection &section);
		void save(IniSection &section);
	};

	class PersonalCfg
	{
	public:
		static const QString SECTION_NAME;

		QString alternativeNickname;
		QString fullName;
		QString nickname;
		QString quitMessage;
		QString userName;

		PersonalCfg();

		// This does not require a init since all values are meant
		// to be empty by default.

		void load(IniSection &section);
		void save(IniSection &section);
	};

	class SoundsCfg
	{
	public:
		static const QString SECTION_NAME;

		bool bUseNicknameUsedSound;
		bool bUsePrivateMessageReceivedSound;
		QString nicknameUsedSound;
		QString privateMessageReceivedSound;

		SoundsCfg();

		void load(IniSection &section);
		void save(IniSection &section);
	};

	/**
	 * @brief Returns the Singleton.
	 */
	static IRCConfig &config();

	/**
	 * @brief Disposes of the Singleton.
	 *
	 * This will @b NOT save the current config to a file.
	 * It is safe to call even if the config was not initialized
	 * first.
	 */
	static void dispose();

	AppearanceCfg appearance;
	GeneralCfg general;
	PersonalCfg personal;
	SoundsCfg sounds;

	Ini *ini()
	{
		return this->pIni.data();
	}

	/**
	 * @brief Reads settings from ini file. This file must be
	 * previously set by setIniFile() method.
	 */
	bool readFromFile();

	/**
	 * @brief Saves current settings to ini file. This file must
	 * be previously set by setIniFile() method.
	 */
	bool saveToFile();

	/**
	 * @brief Initializes the Ini class instance to point
	 * to a file.
	 *
	 * This method will properly dispose of the previous
	 * ini file. However it will neither save the old ini file
	 * or read the new one.
	 */
	bool setIniFile(const QString &filePath);


private:
	static IRCConfig *instance;

	QScopedPointer<QSettings> settings;
	QScopedPointer<SettingsProviderQt> settingsProvider;
	QScopedPointer<Ini> pIni;

	IRCConfig();
	~IRCConfig();

	void loadNetworksFromPlugins();
};

#endif
