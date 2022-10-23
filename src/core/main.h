//------------------------------------------------------------------------------
// main.h
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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <QApplication>
#include <QObject>
#include <QStringList>
#include <QUrl>

class LocalizationInfo;

/**
 * This class holds some global information.
 */
class Main : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief If true then program will install updates and restart
	 *        instead of quitting if quit is requested.
	 *
	 * This flag is ignored if the program configuration states that
	 * there are no updates to install.
	 *
	 * Default: false.
	 */
	static bool bInstallUpdatesAndRestart;
	static QString argDataDir;

	Main(int argc, char *argv[]);
	~Main() override;

	/**
	 * @brief Called by main() after run() returns.
	 *
	 * finalize() cleans up after Main object.
	 */
	void finalize();

	/**
	 * @brief Replaces main().
	 */
	int run();

private:
	enum LogVerbosity
	{
		LV_Default,
		LV_Verbose,
		LV_Quiet
	};

	void applyLogVerbosity();
	int connectToServerByURL();
	void convertOldIniToQSettingsIni();

	void createMainWindow();

	int runTestMode();
	int runVersionDump();

	void initCaCerts();

	/**
	 * @b Creates required directories and sets up the application.
	 *
	 * @return False if it was impossible to setup the directories.
	 */
	bool initDataDirectories();

	/**
	 * If updateip2c == true, application should quit after this returns.
	 * @return If updateip2c == true it returns the exit code. Otherwise
	 *     always returns zero.
	 */
	int initIP2C();
	void initIRCConfig();
	void initLocalizationsDefinitions();
	void initMainConfig();
	void initPasswordsConfig();
	void initPluginConfig();
	int installPendingUpdates();

	/**
	 * @return If false - terminate the application after this method
	 *     returns.
	 */
	bool interpretCommandLineParameters();
	void setupRefreshingThread();
	bool shouldLogToStderr() const;

	char **arguments;
	int argumentsCount;
	bool bIsFirstRun;
	bool bTestMode;
	bool bVersionDump;
	QString versionDumpFile;
	QStringList dataDirectories;
	LogVerbosity logVerbosity;
	QString rconPluginName; /// If not empty assume we want to launch an rcon client.
	QString rconAddress;
	unsigned short rconPort;
	bool startCreateGame;
	bool startRcon;
	int updateFailedCode;
	QUrl connectUrl;
	bool bPortableMode;

private slots:
	void runCreateGame();
	void runRemoteConsole();
};

#endif /* __MAIN_H__ */
