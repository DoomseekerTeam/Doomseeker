//------------------------------------------------------------------------------
// mainwindow.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------

#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include "dptr.h"
#include "serverapi/serverptr.h"

#include <QMainWindow>
#include <QSystemTrayIcon>

class DockBuddiesList;
class DoomseekerConfigurationDialog;
class EnginePlugin;
class IP2CLoader;
class MasterClient;
class MasterManager;
class Message;
class QProgressBar;
class QueryMenuAction;
class ServerListFilterInfo;
struct ServerListCount;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QApplication *application);
	~MainWindow() override;

	DockBuddiesList *buddiesList();

	/**
	 * This will either enable or disable the auto refresh timer
	 * depending on the settings. This method also takes care of every
	 * checks. It will make sure the delay between the refreshes is
	 * inside gives boundaries (30 - 3600 seconds).
	 */
	void initAutoRefreshTimer();

	bool isEffectivelyActiveWindow() const;

	void notifyFirstRun();

	/**
	 * @brief This should be set if program was started with
	 *        "--update-failed" argument.
	 *
	 * Display is done in a way that allows the full initialization process
	 * to complete first before displaying the actual window.
	 *
	 * @b NOTE: This is different than setDisplayUpdateInstallerError()
	 * and mutually exclusive.
	 */
	void setDisplayUpdaterProcessFailure(int errorCode);
	/**
	 * @brief If set then MainWindow will display the reason for
	 *        updater failure.
	 *
	 * Display is done in a way that allows the full initialization process
	 * to complete first before displaying the actual window.
	 *
	 * @b NOTE: This is different than setDisplayUpdaterProcessFailure()
	 * and mutually exclusive.
	 */
	void setDisplayUpdateInstallerError(int errorCode);
	/**
	 * @brief Sets query for selected MasterClient object to enabled
	 * or disabled.
	 */
	void setQueryPluginEnabled(const EnginePlugin *pClient, bool bEnabled);

	void stopAutoRefreshTimer();

	void updateMasterAddresses();

public slots:
	void runGame(const ServerPtr &);
	void showInstallFreedoomDialog();
	void updateDynamicAppearance();

protected:
	friend class DoomseekerConfigurationDialog;

	static const QString HELP_SITE_URL;

	void changeEvent(QEvent *event) override;
	void checkForUpdates(bool bUserTriggered);

	/**
	 * Connects signals from objects and controls of the main window
	 * to their slots.
	 */
	void connectEntities();

	void closeEvent(QCloseEvent *event) override;

	/**
	 * Fills query menu with engines that have master server.
	 * @param masterManager - instantiated MainWindow::masterManager
	 *     is passed here.
	 *     Since this method is called from the constructor it's important
	 *     to pay attention to not call it before masterManager is
	 *     instantiated.
	 *     In other words: this argument exists solely to avoid "random"
	 *     crashes.
	 */
	void fillQueryMenu(MasterManager *masterManager);

	/**
	 * Called by the configuration dialog to sync config changes.
	 */
	void finishConfiguration(DoomseekerConfigurationDialog &, bool);

	bool hasCustomServers() const;

	QProgressBar *mkStdProgressBarForStatusBar();

	void initAutoUpdaterWidgets();
	void initIP2CUpdater();
	void initIRCDock();
	void initLogDock();
	void initMainDock();
	void initServerDetailsDock();
	void initServerFilterDock();

	/**
	 * Checks whether the program will use the tray icon and
	 * deletes or instantiates a QSystemTrayIcon object.
	 */
	void initTrayIcon();

	/**
	 * @brief Will check if refresh operation has any sense.
	 *
	 * @return True if there is anything to refresh (be it master or
	 *     even a single custom server). False if pressing "Get Servers"
	 *     button will produce no results whatsoever.
	 */
	bool isAnythingToRefresh() const;
	bool isAnyMasterEnabled() const;

	QueryMenuAction *queryMenuActionForPlugin(const EnginePlugin *plugin);

	void refreshCustomServers();
	void refreshServersOnList();
	void showEvent(QShowEvent *event) override;
	void setupIcons();
	void setupToolBar();
	ServerListCount sumServerListCount() const;

	void updateTrayIconTooltip(const ServerListCount &count);
	void updateTrayIconTooltipAndLogTotalRefresh();

protected slots:
	void abortAutoUpdater();
	void autoRefreshTimer_timeout();
	void blockRefreshButtons();
	/**
	 * @brief Auto triggered updates will display
	 *        install confirmation only if configured to.
	 */
	void checkForUpdatesAuto();
	/**
	 * @brief User triggered updates will always display
	 *        install confirmation.
	 */
	void checkForUpdatesUserTriggered();
	void confirmUpdateInstallation();
	void discardUpdates();
	void findMissingWADs(const ServerPtr &);
	void finishedQueryingMaster(MasterClient *master);
	void getServers();
	void masterManagerMessages(MasterClient *pSender, const QString &title, const QString &content, bool isError);
	void masterManagerMessagesImportant(MasterClient *pSender, const Message &objMessage);
	void menuBuddies();
	void menuCreateServer();
	void menuHelpAbout();
	void menuHelpHelp();
	void menuIRCOptions();
	void menuLog();
	void menuManageDemos();
	void menuOptionsConfigure();
	void menuRecordDemo();
	void menuUpdateIP2C();
	void menuViewIRC();
	void menuWadSeeker();

	void onAutoUpdaterDownloadAndInstallConfirmationRequest();
	void onAutoUpdaterFileProgress(qint64 bytesReceived, qint64 bytesTotal);
	void onAutoUpdaterFinish();
	void onAutoUpdaterOverallProgress(int current, int total, const QString &msg);

	void quitProgram();
	void postInitAppStartup();
	void refreshThreadBeginsWork();
	void refreshThreadEndsWork();
	void restartAndInstallUpdatesNow();
	void serverAddedToList(const ServerPtr &);
	void showServerJoinCommandLine(const ServerPtr &);
	void showUpdaterProcessErrorDialog();
	void showUpdateInstallErrorDialog();

	/**
	 * This affects the query menu, servers status widgets, master
	 * client and broadcasts.
	 */
	void togglePluginQueryEnabled(const EnginePlugin *plugin);
	void toolBarAction(QAction *pAction);
	void trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
	void updateServerFilter(const ServerListFilterInfo &filterInfo);
	void updateRefreshProgress();

private:
	DPtr<MainWindow> d;

	void connectIP2CLoader();
	void fixIconsDpi();

private slots:
	void ip2cDownloadProgress(qint64 current, qint64 max);
	void ip2cJobsFinished();
	void ip2cStartUpdate();
	void showAndLogStatusMessage(const QString &message);
	void showServerJoinCommandLineOnBuilderFinished();
	void showProgramArgsHelp();
};

#endif
