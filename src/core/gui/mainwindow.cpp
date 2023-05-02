//------------------------------------------------------------------------------
// mainwindow.cpp
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
#include "application.h"
#include "apprunner.h"
#include "commandline.h"
#include "configuration/doomseekerconfig.h"
#include "configuration/queryspeed.h"
#include "connectionhandler.h"
#include "customservers.h"
#include "datapaths.h"
#include "doomseekerfilepaths.h"
#include "fileutils.h"
#include "gamedemo.h"
#include "gui/aboutdialog.h"
#include "gui/checkwadsdlg.h"
#include "gui/configuration/doomseekerconfigurationdialog.h"
#include "gui/configuration/irc/ircconfigurationdialog.h"
#include "gui/copytextdlg.h"
#include "gui/createserverdialog.h"
#include "gui/demomanager.h"
#include "gui/dockBuddiesList.h"
#include "gui/freedoomdialog.h"
#include "gui/helpers/playersdiagram.h"
#include "gui/helpers/taskbarbutton.h"
#include "gui/helpers/taskbarprogress.h"
#include "gui/ip2cupdatebox.h"
#include "gui/irc/ircdock.h"
#include "gui/irc/ircsounds.h"
#include "gui/logdock.h"
#include "gui/mainwindow.h"
#include "gui/models/serverlistmodel.h"
#include "gui/programargshelpdialog.h"
#include "gui/serverdetailsdock.h"
#include "gui/serverfilterdock.h"
#include "gui/serverlist.h"
#include "gui/wadseekerinterface.h"
#include "gui/wadseekershow.h"
#include "gui/widgets/serversstatuswidget.h"
#include "ip2c/ip2cloader.h"
#include "irc/configuration/chatnetworkscfg.h"
#include "irc/configuration/ircconfig.h"
#include "joincommandlinebuilder.h"
#include "log.h"
#include "main.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "refresher/refresher.h"
#include "serverapi/broadcast.h"
#include "serverapi/broadcastmanager.h"
#include "serverapi/gameclientrunner.h"
#include "serverapi/mastermanager.h"
#include "serverapi/message.h"
#include "serverapi/server.h"
#include "serverapi/serverlistcounttracker.h"
#include "strings.hpp"
#include "ui_mainwindow.h"
#include "updater/autoupdater.h"
#include "updater/updatechannel.h"
#include "updater/updateinstaller.h"
#include "updater/updatepackage.h"
#include "wadseeker/entities/checksum.h"
#include <cassert>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QFileInfo>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QMessageBox>
#include <QPointer>
#include <QProgressBar>
#include <QSizePolicy>
#include <QToolBar>

const QString MainWindow::HELP_SITE_URL = "https://doomseeker.drdteam.org/help";

/**
 *	@brief Menu action for Query Menu
 *
 *	Replaces the original QAction to make toggling of master clients easier.
 *	The constructor automatically connects the passed MasterClient's
 *	setEnabled() slot to this QueryMenuAction toggled() signal.
 */
class QueryMenuAction : public QAction
{
public:
	QueryMenuAction(const EnginePlugin *plugin, ServersStatusWidget *statusWidget, QObject *parent = nullptr)
		: QAction(parent)
	{
		this->pPlugin = plugin;

		if (plugin != nullptr)
		{
			connect(this, SIGNAL(toggled(bool)), plugin->data()->masterClient,
				SLOT(setEnabled(bool)));
			connect(this, SIGNAL(toggled(bool)), statusWidget, SLOT(setMasterEnabledStatus(bool)));
		}
	}

	const EnginePlugin *plugin() const
	{
		return pPlugin;
	}

private:
	const EnginePlugin *pPlugin;
};

DClass<MainWindow> : public Ui::MainWindowWnd
{
public:
	PrivData() : bTotalRefreshInProcess(false), buddiesList(nullptr),
		bWasMaximized(false), bWantToQuit(false), logDock(nullptr),
		masterManager(nullptr), trayIcon(nullptr), trayIconMenu(nullptr)
	{
	}

	QApplication *application;
	QTimer autoRefreshTimer;

	AutoUpdater *autoUpdater;
	QWidget *autoUpdaterStatusBarWidget;
	QPushButton *autoUpdaterAbortButton;
	QLabel *autoUpdaterLabel;
	QProgressBar *autoUpdaterFileProgressBar;
	QProgressBar *autoUpdaterOverallProgressBar;

	/**
	 * Set to true by btnGetServers_click() process and to false
	 * when refreshing thread enters sleep mode.
	 */
	bool bTotalRefreshInProcess;

	DockBuddiesList *buddiesList;
	BroadcastManager *broadcastManager;

	/**
	 * This is required so tray icon knows how to bring the window back.
	 */
	bool bWasMaximized;

	/**
	 * If set to true the closeEvent() method will ignore tray icon
	 * settings and proceed to close the MainWindow. This is set by
	 * quitProgram() slot.
	 */
	bool bWantToQuit;

	IP2CLoader *ip2cLoader;
	QProgressBar *ip2cUpdateProgressBar;
	IRCDock *ircDock;
	LogDock *logDock;
	ServerDetailsDock *detailsDock;
	QPointer<FreedoomDialog> freedoomDialog;
	ServerFilterDock *serverFilterDock;
	ServerList *serverList;

	MasterManager *masterManager;
	QHash<const EnginePlugin *, QueryMenuAction *> queryMenuPorts;
	QHash<const EnginePlugin *, ServersStatusWidget *> serversStatusesWidgets;
	QAction *toolBarGetServers;
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
	/// Update should be discarded if this changes.
	UpdateChannel *updateChannelOnUpdateStart;
	int updaterInstallerErrorCode;

	ConnectionHandler *connectionHandler;
	QDockWidget *mainDock;

	TaskbarProgress *taskbarProgress;
	TaskbarButton *taskbarButton;
};

DPointeredNoCopy(MainWindow)

MainWindow::MainWindow(QApplication *application)
{
	d->autoUpdater = nullptr;
	d->mainDock = nullptr;
	d->connectionHandler = nullptr;
	d->updateChannelOnUpdateStart = new UpdateChannel();
	d->updaterInstallerErrorCode = 0;

	d->application = application;

	this->setAttribute(Qt::WA_DeleteOnClose, true);
	d->setupUi(this);

	d->taskbarButton = new TaskbarButton(this);
	d->taskbarProgress = d->taskbarButton->progress();

	setupIcons();

	initAutoUpdaterWidgets();

	d->updatesConfirmationWidget->hide();
	d->updatesDownloadedWidget->hide();

	// Hide menu options which aren't supported on target platform.
	#ifndef WITH_AUTOUPDATES
	d->menuActionCheckForUpdates->setVisible(false);
	#endif

	initIP2CUpdater();

	// The buddies list must always be available so we can perform certain operations on it
	d->buddiesList = new DockBuddiesList(this);
	d->menuView->addAction(d->buddiesList->toggleViewAction());
	d->buddiesList->toggleViewAction()->setText(MainWindow::tr("&Buddies"));
	d->buddiesList->toggleViewAction()->setShortcut(MainWindow::tr("CTRL+B"));

	connect(d->buddiesList, SIGNAL(joinServer(ServerPtr)), this, SLOT(runGame(ServerPtr)));
	d->buddiesList->hide();
	this->addDockWidget(Qt::LeftDockWidgetArea, d->buddiesList);
	initLogDock();
	initIRCDock();
	initServerFilterDock();
	initMainDock();
	splitDockWidget(d->mainDock, d->serverFilterDock, Qt::Horizontal);

	// Spawn Server Table Handler.
	d->serverList = new ServerList(d->tableServers, this);
	connectEntities();

	d->broadcastManager = new BroadcastManager(this);
	d->serverList->connect(d->broadcastManager,
		SIGNAL(newServerDetected(ServerPtr,int)), SLOT(registerServer(ServerPtr)));
	d->serverList->connect(d->broadcastManager,
		SIGNAL(serverLost(ServerPtr)), SLOT(removeServer(ServerPtr)));

	initServerDetailsDock();
	tabifyDockWidget(d->ircDock, d->detailsDock);

	// Restore checked states.
	d->menuActionRecordDemo->setChecked(gConfig.doomseeker.bRecordDemo);

	// Get the master
	d->masterManager = new MasterManager();
	d->masterManager->setBroadcastManager(d->broadcastManager);
	d->buddiesList->scan(d->masterManager);
	connect(d->masterManager, SIGNAL(masterMessage(MasterClient*,const QString&,const QString&,bool)),
		this, SLOT(masterManagerMessages(MasterClient*,const QString&,const QString&,bool)));
	connect(d->masterManager, SIGNAL(masterMessageImportant(MasterClient*,const Message&)),
		this, SLOT(masterManagerMessagesImportant(MasterClient*,const Message&)));

	// Allow us to enable and disable ports.
	fillQueryMenu(d->masterManager);

	// Init custom servers
	QList<ServerPtr> customServers = d->masterManager->customServs()->readConfig();
	for (ServerPtr server : customServers)
	{
		d->serverList->registerServer(server);
	}

	setWindowIcon(Application::icon());

	// Auto refresh timer
	initAutoRefreshTimer();
	connect(&d->autoRefreshTimer, SIGNAL(timeout()), this, SLOT(autoRefreshTimer_timeout()));

	// Tray icon
	initTrayIcon();

	setupToolBar();

	// Player diagram styles
	PlayersDiagram::loadImages(gConfig.doomseeker.slotStyle);

	// IP2C
	d->menuActionUpdateIP2C->setEnabled(false);
	d->ip2cLoader = new IP2CLoader();
	connectIP2CLoader();
	d->ip2cLoader->load();

	restoreState(QByteArray::fromBase64(gConfig.doomseeker.mainWindowState.toUtf8()));
	restoreGeometry(gConfig.doomseeker.mainWindowGeometry);

	// Start first refresh from a timer. We want the main window fully
	// set up before refresh.
	QTimer::singleShot(1, this, SLOT(postInitAppStartup()));
}

MainWindow::~MainWindow()
{
	// Window geometry settings
	gConfig.doomseeker.mainWindowGeometry = saveGeometry();
	gConfig.doomseeker.mainWindowState = saveState().toBase64();

	if (d->updateChannelOnUpdateStart != nullptr)
		delete d->updateChannelOnUpdateStart;
	if (d->autoUpdater != nullptr)
	{
		d->autoUpdater->disconnect();
		delete d->autoUpdater;
	}
	if (d->connectionHandler)
		delete d->connectionHandler;

	QList<QAction *> menuQueryActions = d->menuQuery->actions();
	QList<QAction *>::iterator it;
	for (QAction *action : menuQueryActions)
	{
		QString pluginName = action->text();

		if (!pluginName.isEmpty())
		{
			IniSection pluginConfig = gConfig.iniSectionForPlugin(pluginName);
			pluginConfig["Query"] = action->isChecked();
		}
	}

	if (d->trayIcon != nullptr)
	{
		d->trayIcon->setVisible(false);
		delete d->trayIcon;
		d->trayIcon = nullptr;
	}

	if (d->trayIconMenu != nullptr)
	{
		delete d->trayIconMenu;
		d->trayIconMenu = nullptr;
	}

	delete d->serverList;

	if (d->masterManager != nullptr)
		delete d->masterManager;

	if (d->ip2cLoader != nullptr)
		delete d->ip2cLoader;
}

void MainWindow::abortAutoUpdater()
{
	if (d->autoUpdater != nullptr)
		d->autoUpdater->abort();
}

void MainWindow::autoRefreshTimer_timeout()
{
	if (gConfig.doomseeker.bQueryAutoRefreshDontIfActive && !isMinimized())
	{
		if (QApplication::activeWindow() != nullptr)
			return;
	}

	getServers();
}

void MainWindow::blockRefreshButtons()
{
	d->toolBarGetServers->setEnabled(false);
}

DockBuddiesList *MainWindow::buddiesList()
{
	return d->buddiesList;
}

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::ActivationChange && isActiveWindow() && !isMinimized() && !isHidden())
	{
		d->serverList->cleanUp();
		event->accept();
	}
	QMainWindow::changeEvent(event);
}

void MainWindow::checkForUpdates(bool bUserTriggered)
{
	if (d->autoUpdater != nullptr)
	{
		if (d->autoUpdater->isRunning())
		{
			QMessageBox::warning(this, tr("Doomseeker - Auto Update"),
				tr("Update is already in progress."));
			return;
		}
		else
		{
			delete d->autoUpdater;
			d->autoUpdater = nullptr;
		}
	}
	gLog << tr("Removing old update packages from local temporary space.");
	QStringList removeFilter(QString("%1*").arg(DataPaths::UPDATE_PACKAGE_FILENAME_PREFIX));
	FileUtils::rmAllFiles(DoomseekerFilePaths::updatePackagesStorageDir(),
		removeFilter);

	showAndLogStatusMessage(tr("Checking for updates..."));
	d->autoUpdater = new AutoUpdater();
	this->connect(d->autoUpdater, SIGNAL(statusMessage(QString)),
		SLOT(showAndLogStatusMessage(QString)));
	this->connect(d->autoUpdater, SIGNAL(finished()),
		SLOT(onAutoUpdaterFinish()));
	this->connect(d->autoUpdater, SIGNAL(downloadAndInstallConfirmationRequested()),
		SLOT(onAutoUpdaterDownloadAndInstallConfirmationRequest()));
	this->connect(d->autoUpdater, SIGNAL(overallProgress(int,int,const QString&)),
		SLOT(onAutoUpdaterOverallProgress(int,int,const QString&)));
	this->connect(d->autoUpdater, SIGNAL(packageDownloadProgress(qint64,qint64)),
		SLOT(onAutoUpdaterFileProgress(qint64,qint64)));

	QMap<QString, QList<QString> > ignoredPackagesRevisions;
	if (!bUserTriggered)
	{
		for (const QString &package : gConfig.autoUpdates.lastKnownUpdateRevisions.keys())
		{
			QString revision = gConfig.autoUpdates.lastKnownUpdateRevisions[package];
			QList<QString> list;
			list << revision;
			ignoredPackagesRevisions.insert(package, list);
		}
	}
	d->autoUpdater->setIgnoreRevisions(ignoredPackagesRevisions);

	UpdateChannel channel = UpdateChannel::fromName(gConfig.autoUpdates.updateChannelName);
	d->autoUpdater->setChannel(channel);
	*d->updateChannelOnUpdateStart = channel;

	bool bRequireConfirmation = true;
	if (!bUserTriggered)
	{
		bRequireConfirmation = (gConfig.autoUpdates.updateMode
			!= DoomseekerConfig::AutoUpdates::UM_FullAuto);
	}
	d->autoUpdater->setRequireDownloadAndInstallConfirmation(bRequireConfirmation);
	d->autoUpdaterStatusBarWidget->show();
	d->autoUpdater->start();
}


void MainWindow::checkForUpdatesAuto()
{
	const bool bUserTriggered = true;
	checkForUpdates(!bUserTriggered);
}

void MainWindow::checkForUpdatesUserTriggered()
{
	const bool bUserTriggered = true;
	checkForUpdates(bUserTriggered);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	// Check if tray icon is available and if we want to minimize to tray icon
	// when 'X' button is pressed. Real quit requests are handled by
	// quitProgram() method. This method sets d->bWantToQuit to true.
	if (d->trayIcon != nullptr && gConfig.doomseeker.bCloseToTrayIcon && !d->bWantToQuit)
	{
		d->bWasMaximized = isMaximized();
		event->ignore();
		hide();
	}
	else
		event->accept();
}

void MainWindow::confirmUpdateInstallation()
{
	assert(d->autoUpdater != nullptr && "MainWindow::confirmUpdateInstallation()");
	d->updatesConfirmationWidget->hide();
	d->autoUpdater->confirmDownloadAndInstall();
}

void MainWindow::connectIP2CLoader()
{
	this->connect(d->ip2cLoader, SIGNAL(finished()), SLOT(ip2cJobsFinished()));
	this->connect(d->ip2cLoader, SIGNAL(downloadProgress(qint64,qint64)),
		SLOT(ip2cDownloadProgress(qint64,qint64)));
}

void MainWindow::discardUpdates()
{
	assert(d->autoUpdater != nullptr && "MainWindow::confirmUpdateInstallation()");
	d->updatesConfirmationWidget->hide();
	// User rejected this update so let's add the packages
	// to the ignore list so user won't be nagged again.
	const QList<UpdatePackage> &pkgList = d->autoUpdater->newUpdatePackages();
	for (const UpdatePackage &pkg : pkgList)
	{
		gConfig.autoUpdates.lastKnownUpdateRevisions.insert(pkg.name, pkg.revision);
	}
	d->autoUpdater->abort();
}

void MainWindow::connectEntities()
{
	// Connect refreshing thread.
	connect(gRefresher, SIGNAL(block()), this, SLOT(blockRefreshButtons()));
	connect(gRefresher, SIGNAL(finishedQueryingMaster(MasterClient*)), this, SLOT(finishedQueryingMaster(MasterClient*)));
	connect(gRefresher, SIGNAL(sleepingModeEnter()), this, SLOT(refreshThreadEndsWork()));
	connect(gRefresher, SIGNAL(sleepingModeEnter()), d->buddiesList, SLOT(scan()));
	connect(gRefresher, SIGNAL(sleepingModeExit()), this, SLOT(refreshThreadBeginsWork()));

	// Controls
	connect(d->menuActionAbout, SIGNAL(triggered()), this, SLOT(menuHelpAbout()));
	connect(d->menuActionAboutQt, SIGNAL(triggered()), d->application, SLOT(aboutQt()));
	connect(d->menuActionBuddies, SIGNAL(triggered()), this, SLOT(menuBuddies()));
	connect(d->menuActionConfigure, SIGNAL(triggered()), this, SLOT(menuOptionsConfigure()));
	connect(d->menuActionCreateServer, SIGNAL(triggered()), this, SLOT(menuCreateServer()));
	connect(d->menuActionHelp, SIGNAL(triggered()), this, SLOT (menuHelpHelp()));
	connect(d->menuActionIRCOptions, SIGNAL(triggered()), this, SLOT(menuIRCOptions()));
	connect(d->menuActionLog, SIGNAL(triggered()), this, SLOT(menuLog()));
	connect(d->menuActionManageDemos, SIGNAL(triggered()), this, SLOT(menuManageDemos()));
	connect(d->menuActionRecordDemo, SIGNAL(triggered()), this, SLOT(menuRecordDemo()));
	connect(d->menuActionUpdateIP2C, SIGNAL(triggered()), this, SLOT(menuUpdateIP2C()));
	connect(d->menuActionQuit, SIGNAL(triggered()), this, SLOT(quitProgram()));
	connect(d->menuActionViewIRC, SIGNAL(triggered()), this, SLOT(menuViewIRC()));
	connect(d->menuActionWadseeker, SIGNAL(triggered()), this, SLOT(menuWadSeeker()));
	connect(d->serverFilterDock, SIGNAL(filterUpdated(const ServerListFilterInfo&)),
		this, SLOT(updateServerFilter(const ServerListFilterInfo&)));
	connect(d->serverFilterDock, SIGNAL(nonEmptyServerGroupingAtTopToggled(bool)),
		d->serverList, SLOT(setGroupServersWithPlayersAtTop(bool)));
	connect(d->serverList, SIGNAL(serverFilterModified(ServerListFilterInfo)),
		d->serverFilterDock, SLOT(setFilterInfo(ServerListFilterInfo)));
	connect(d->serverList, SIGNAL(serverDoubleClicked(ServerPtr)), this, SLOT(runGame(ServerPtr)));
	connect(d->serverList, SIGNAL(displayServerJoinCommandLine(const ServerPtr&)), this, SLOT(showServerJoinCommandLine(const ServerPtr&)));
	connect(d->serverList, SIGNAL(findMissingWADs(const ServerPtr&)), this, SLOT(findMissingWADs(const ServerPtr&)));
	connect(d->serverList, SIGNAL(serverInfoUpdated(ServerPtr)), this, SLOT(serverAddedToList(ServerPtr)));
	connect(d->buddiesList, SIGNAL(scanCompleted()), d->serverList, SLOT(redraw()));
}

void MainWindow::fillQueryMenu(MasterManager *masterManager)
{
	// This is called only once from the constructor. No clears to
	// d->queryMenuPorts are ever performed. Not even in the destructor.
	for (unsigned i = 0; i < gPlugins->numPlugins(); ++i)
	{
		const EnginePlugin *plugin = gPlugins->info(i);
		if (!plugin->data()->hasMasterClient() && !plugin->data()->hasBroadcast())
			continue;

		if (plugin->data()->hasMasterClient())
		{
			MasterClient *pMasterClient = plugin->data()->masterClient;
			pMasterClient->updateAddress();
			masterManager->addMaster(pMasterClient);
		}

		if (plugin->data()->hasBroadcast())
			d->broadcastManager->registerPlugin(plugin);

		// Now is a good time to also populate the status bar widgets
		auto statusWidget = new ServersStatusWidget(plugin, d->serverList);
		d->serversStatusesWidgets.insert(plugin, statusWidget);

		this->connect(statusWidget, SIGNAL(clicked(const EnginePlugin*)),
			SLOT(togglePluginQueryEnabled(const EnginePlugin*)));
		this->connect(statusWidget, SIGNAL(counterUpdated()),
			SLOT(updateRefreshProgress()));

		statusBar()->addPermanentWidget(statusWidget);

		QString name = gPlugins->info(i)->data()->name;
		auto query = new QueryMenuAction(plugin, statusWidget, d->menuQuery);
		d->queryMenuPorts.insert(plugin, query);

		d->menuQuery->addAction(query);

		query->setCheckable(true);
		query->setIcon(plugin->icon());
		query->setText(name);

		IniSection pluginConfig = gConfig.iniSectionForPlugin(name);

		if (!pluginConfig.retrieveSetting("Query").value().isNull())
		{
			bool enabled = pluginConfig["Query"];
			setQueryPluginEnabled(plugin, enabled);
		}
		else
		{
			// if no setting is found for this engine
			// set default to true:
			setQueryPluginEnabled(plugin, true);
		}
	}
}

void MainWindow::findMissingWADs(const ServerPtr &server)
{
	// Display a message if all WADs are present.
	QList<PWad> wads = server->wads();
	PathFinder pathFinder = server->wadPathFinder();
	QList<PWad> missingWads;
	QList<PWad> incompatibleWads;

	auto checkWadsDlg = new CheckWadsDlg(&pathFinder);
	checkWadsDlg->addWads(wads);
	const CheckResult checkResults = checkWadsDlg->checkWads();

	for (const PWad &wad : checkResults.missingWads)
	{
		missingWads << PWad(wad.name(), true, wad.checksums());
	}
	incompatibleWads << checkResults.incompatibleWads;

	if (missingWads.isEmpty() && incompatibleWads.isEmpty())
	{
		QMessageBox::information(this, tr("All WADs found"), tr("All of the WADs used by this server are present."));
		return;
	}

	MissingWadsDialog dialog(missingWads, incompatibleWads, this);
	dialog.setAllowIgnore(false);
	if (dialog.exec() == QDialog::Accepted && dialog.decision() == MissingWadsDialog::Install)
	{
		if (!gWadseekerShow->checkWadseekerValidity(this))
			return;
		WadseekerInterface *wadseeker = WadseekerInterface::createAutoNoGame();
		wadseeker->setCustomSite(server->webSite());
		wadseeker->setWads(dialog.filesToDownload());
		wadseeker->setAttribute(Qt::WA_DeleteOnClose);
		wadseeker->show();
	}
}

void MainWindow::finishConfiguration(DoomseekerConfigurationDialog &configDialog, bool lookupHostsChanged)
{
	// In case the master server addresses changed, notify the master clients.
	updateMasterAddresses();
	gRefresher->setDelayBetweenResends(gConfig.doomseeker.querySpeed().delayBetweenSingleServerAttempts);

	// If appearance changed - update the widgets.
	if (configDialog.wasAppearanceChanged())
	{
		updateDynamicAppearance();
		initTrayIcon();
	}

	// If changes require a restart, tell the user
	if (configDialog.isRestartNeeded())
	{
		QString warningRestartNeeded = tr("Doomseeker needs to be restarted for some changes to be applied.");
		d->importantMessagesWidget->addMessage(warningRestartNeeded);
	}

	// Do the following only if setting changed from false to true.
	if (lookupHostsChanged)
		d->serverList->lookupHosts();

	// Refresh custom servers list:
	if (configDialog.customServersChanged())
	{
		d->serverList->removeCustomServers();
		QList<ServerPtr> servers = d->masterManager->customServs()->readConfig();
		for (ServerPtr server : servers)
		{
			d->serverList->registerServer(server);
		}
		refreshCustomServers();
	}
}

void MainWindow::finishedQueryingMaster(MasterClient *master)
{
	if (master == nullptr)
		return;

	for (int i = 0; i < master->numServers(); i++)
		d->serverList->registerServer((*master)[i]);
}

void MainWindow::fixIconsDpi()
{
	// http://blog.qt.io/blog/2013/04/25/retina-display-support-for-mac-os-ios-and-x11/
	QIcon icon(":/icons/exclamation_16.png");
	d->lblExclamation1->setPixmap(icon.pixmap(16));
	d->lblExclamation2->setPixmap(icon.pixmap(16));
}

void MainWindow::getServers()
{
	// Check if this operation has any sense.
	if (!isAnythingToRefresh())
	{
		QString message = tr("Doomseeker is unable to proceed with the refresh"
			" operation because the following problem has occurred:\n\n");

		if (gPlugins->numPlugins() == 0)
			message += tr("Plugins are missing from the \"engines/\" directory.");
		else if (!isAnyMasterEnabled())
			message += tr("No master servers are enabled in the \"Query\" menu.");
		else
			message += tr("Unknown error occurred.");

		gLog << message;
		QMessageBox::warning(this, tr("Doomseeker - refresh problem"), message);
		return;
	}

	d->bTotalRefreshInProcess = true;
	d->autoRefreshTimer.stop();
	gLog << tr("Total refresh initialized!");

	// Remove all non special servers. Master servers will
	// "re-provide" them if they are still alive. Whatever
	// remains on the list should be refreshed now.
	d->serverList->removeNonSpecialServers();
	refreshServersOnList();

	if (!isAnyMasterEnabled() && !d->serverList->hasAtLeastOneServer())
	{
		gLog << tr("Warning: No master servers were enabled for this refresh. "
				"Check your Query menu or \"engines/\" directory.");
	}

	d->masterManager->clearServers();
	for (int i = 0; i < d->masterManager->numMasters(); ++i)
	{
		MasterClient *pMaster = (*d->masterManager)[i];

		if (pMaster->isEnabled())
			gRefresher->registerMaster(pMaster);
	}
}

bool MainWindow::hasCustomServers() const
{
	CustomServers *customServers = d->masterManager->customServs();
	return customServers->numServers() > 0;
}

void MainWindow::initAutoRefreshTimer()
{
	const unsigned MIN_DELAY = 30;
	const unsigned MAX_DELAY = 3600;

	bool bEnabled = gConfig.doomseeker.bQueryAutoRefreshEnabled;

	if (!bEnabled)
		d->autoRefreshTimer.stop();
	else
	{
		// If delay value is out of bounds we should adjust
		// config value as well.
		unsigned &delay = gConfig.doomseeker.queryAutoRefreshEverySeconds;

		// Make sure delay is in given limit.
		if (delay < MIN_DELAY)
			delay = MIN_DELAY;
		else if (delay > MAX_DELAY)
			delay = MAX_DELAY;

		unsigned delayMs = delay * 1000;

		d->autoRefreshTimer.setSingleShot(false);
		d->autoRefreshTimer.start(delayMs);
	}
}

void MainWindow::initAutoUpdaterWidgets()
{
	static const int FILE_BAR_WIDTH = 50;
	static const int OVERALL_BAR_WIDTH = 180;

	d->autoUpdaterStatusBarWidget = new QWidget(statusBar());
	d->autoUpdaterStatusBarWidget->setLayout(new QHBoxLayout(d->autoUpdaterStatusBarWidget));
	d->autoUpdaterStatusBarWidget->layout()->setContentsMargins(QMargins(0, 0, 0, 0));
	statusBar()->addPermanentWidget(d->autoUpdaterStatusBarWidget);
	d->autoUpdaterStatusBarWidget->hide();

	d->autoUpdaterLabel = new QLabel(d->autoUpdaterStatusBarWidget);
	d->autoUpdaterLabel->setText(tr("Auto Updater:"));
	d->autoUpdaterStatusBarWidget->layout()->addWidget(d->autoUpdaterLabel);

	d->autoUpdaterFileProgressBar = mkStdProgressBarForStatusBar();
	d->autoUpdaterFileProgressBar->setFormat("%p%");
	d->autoUpdaterFileProgressBar->setMaximumWidth(FILE_BAR_WIDTH);
	d->autoUpdaterFileProgressBar->setMinimumWidth(FILE_BAR_WIDTH);
	d->autoUpdaterStatusBarWidget->layout()->addWidget(d->autoUpdaterFileProgressBar);

	d->autoUpdaterOverallProgressBar = mkStdProgressBarForStatusBar();
	d->autoUpdaterOverallProgressBar->setMaximumWidth(OVERALL_BAR_WIDTH);
	d->autoUpdaterOverallProgressBar->setMinimumWidth(OVERALL_BAR_WIDTH);
	d->autoUpdaterStatusBarWidget->layout()->addWidget(d->autoUpdaterOverallProgressBar);

	d->autoUpdaterAbortButton = new QPushButton(statusBar());
	d->autoUpdaterAbortButton->setToolTip(tr("Abort update."));
	d->autoUpdaterAbortButton->setIcon(QIcon(":/icons/x.png"));
	this->connect(d->autoUpdaterAbortButton, SIGNAL(clicked()),
		SLOT(abortAutoUpdater()));
	d->autoUpdaterStatusBarWidget->layout()->addWidget(d->autoUpdaterAbortButton);
}

void MainWindow::initIP2CUpdater()
{
	static const int PROGRESSBAR_WIDTH = 220;

	d->ip2cUpdateProgressBar = mkStdProgressBarForStatusBar();
	d->ip2cUpdateProgressBar->setFormat(tr("IP2C Update"));
	d->ip2cUpdateProgressBar->hide();
	d->ip2cUpdateProgressBar->setMaximumWidth(PROGRESSBAR_WIDTH);
	d->ip2cUpdateProgressBar->setMinimumWidth(PROGRESSBAR_WIDTH);
	statusBar()->addPermanentWidget(d->ip2cUpdateProgressBar);
}

void MainWindow::initIRCDock()
{
	d->ircDock = new IRCDock(this);
	d->menuView->addAction(d->ircDock->toggleViewAction());
	d->ircDock->toggleViewAction()->setText(tr("&IRC"));
	d->ircDock->toggleViewAction()->setShortcut(tr("CTRL+I"));
	d->ircDock->hide();
	this->addDockWidget(Qt::BottomDockWidgetArea, d->ircDock);

	if (ChatNetworksCfg().isAnyNetworkOnAutoJoin())
	{
		this->d->ircDock->setVisible(true);
		this->d->ircDock->performNetworkAutojoins();
	}
}

void MainWindow::initLogDock()
{
	d->logDock = new LogDock(this);
	d->menuView->addAction(d->logDock->toggleViewAction());
	d->logDock->toggleViewAction()->setText(tr("&Log"));
	d->logDock->toggleViewAction()->setShortcut(tr("CTRL+L"));
	d->logDock->hide();
	this->addDockWidget(Qt::BottomDockWidgetArea, d->logDock);

	connect(&gLog, SIGNAL(newEntry(const QString&)), d->logDock, SLOT(appendLogEntry(const QString&)));

	// Also add anything that already might be in the log to the box.
	d->logDock->appendLogEntry(gLog.content());
}

void MainWindow::initMainDock()
{
	setDockNestingEnabled(true); // This line allows us to essentially treat a dock as a central widget.

	// Make a dock out of the central MainWindow widget and drop that widget
	// from the MainWindow itself.
	d->mainDock = new QDockWidget(tr("Servers"));
	d->mainDock->setTitleBarWidget(new QWidget(this));
	d->mainDock->setObjectName("ServerList");
	d->mainDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	d->mainDock->setWidget(centralWidget());
	setCentralWidget(nullptr);
	addDockWidget(Qt::RightDockWidgetArea, d->mainDock);
}

void MainWindow::initServerDetailsDock()
{
	d->detailsDock = new ServerDetailsDock(this);
	d->menuView->addAction(d->detailsDock->toggleViewAction());
	d->detailsDock->toggleViewAction()->setText(tr("Server &Details"));
	d->detailsDock->toggleViewAction()->setShortcut(tr("CTRL+D"));
	d->detailsDock->hide();
	addDockWidget(Qt::BottomDockWidgetArea, d->detailsDock);

	d->detailsDock->connect(d->serverList, SIGNAL(serversSelected(QList<ServerPtr>&)), SLOT(displaySelection(QList<ServerPtr> &)));
}

void MainWindow::initServerFilterDock()
{
	d->serverFilterDock = new ServerFilterDock(this);

	d->menuView->addAction(d->serverFilterDock->toggleViewAction());
	d->serverFilterDock->hide();
	this->addDockWidget(Qt::RightDockWidgetArea, d->serverFilterDock);
}

void MainWindow::initTrayIcon()
{
	bool isEnabled = gConfig.doomseeker.bUseTrayIcon;
	if (!isEnabled || !QSystemTrayIcon::isSystemTrayAvailable())
	{
		if (d->trayIcon != nullptr)
		{
			delete d->trayIcon;
			d->trayIcon = nullptr;
		}

		if (d->trayIconMenu != nullptr)
		{
			delete d->trayIconMenu;
			d->trayIconMenu = nullptr;
		}
	}
	else if (d->trayIcon == nullptr)
	{
		QAction *trayAction;
		d->trayIconMenu = new QMenu(this);
		trayAction = d->trayIconMenu->addAction("Exit");
		connect(trayAction, SIGNAL(triggered()), this, SLOT(quitProgram()));

		// This should be automatically deleted when main window closes
		d->trayIcon = new QSystemTrayIcon(this);
		connect(d->trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIcon_activated(QSystemTrayIcon::ActivationReason)));

		updateTrayIconTooltipAndLogTotalRefresh();

		d->trayIcon->setContextMenu(d->trayIconMenu);
		d->trayIcon->setIcon(QIcon(":/icon.png"));
		d->trayIcon->setVisible(true);
	}
}

void MainWindow::ip2cDownloadProgress(qint64 current, qint64 max)
{
	d->ip2cUpdateProgressBar->setMaximum(max);
	d->ip2cUpdateProgressBar->setValue(current);
}

void MainWindow::ip2cJobsFinished()
{
	d->menuActionUpdateIP2C->setEnabled(true);
	d->serverList->updateCountryFlags();
	d->ip2cUpdateProgressBar->hide();

	if (d->ip2cLoader != nullptr)
	{
		delete d->ip2cLoader;
		d->ip2cLoader = nullptr;
	}
}

void MainWindow::ip2cStartUpdate()
{
	if (d->ip2cLoader != nullptr)
	{
		// If update is currently in progress then prevent re-starting.
		return;
	}

	d->menuActionUpdateIP2C->setEnabled(false);
	d->ip2cUpdateProgressBar->show();

	d->ip2cLoader = new IP2CLoader();
	connectIP2CLoader();
	d->ip2cLoader->update();
}

bool MainWindow::isAnythingToRefresh() const
{
	return hasCustomServers() || isAnyMasterEnabled();
}

bool MainWindow::isAnyMasterEnabled() const
{
	for (int i = 0; i < d->masterManager->numMasters(); ++i)
	{
		MasterClient *pMaster = (*d->masterManager)[i];

		if (pMaster->isEnabled())
			return true;
	}

	return false;
}

bool MainWindow::isEffectivelyActiveWindow() const
{
	return this->isActiveWindow() || DoomseekerConfigurationDialog::isOpen();
}

void MainWindow::masterManagerMessages(MasterClient *pSender, const QString &title, const QString &content, bool isError)
{
	Q_UNUSED(title);
	QString message = tr("Master server for %1: %2").arg(pSender->plugin()->data()->name).arg(content);

	if (isError)
	{
		message = tr("Error: %1").arg(message);
		statusBar()->showMessage(message);
	}

	gLog << message;
}

void MainWindow::masterManagerMessagesImportant(MasterClient *pSender, const Message &objMessage)
{
	QString strFullMessage = tr("%1: %2")
		.arg(pSender->plugin()->data()->name)
		.arg(objMessage.contents());
	d->importantMessagesWidget->addMessage(strFullMessage, objMessage.timestamp());
}

void MainWindow::menuBuddies()
{
	d->buddiesList->setVisible(!d->buddiesList->isVisible());
}

void MainWindow::menuCreateServer()
{
	// This object will auto-delete on close.
	auto dialog = new CreateServerDialog(GameCreateParams::Host, nullptr);
	dialog->setWindowIcon(this->windowIcon());
	dialog->show();
}

void MainWindow::menuHelpAbout()
{
	AboutDialog dlg(this);
	d->autoRefreshTimer.stop();
	dlg.exec();
	initAutoRefreshTimer();
}

void MainWindow::menuHelpHelp()
{
	if (HELP_SITE_URL.isEmpty() || !Strings::isUrlSafe(HELP_SITE_URL))
	{
		QMessageBox::critical(this, tr("Help error"), tr("No help found."), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	bool bSuccess = QDesktopServices::openUrl(HELP_SITE_URL);

	if (!bSuccess)
	{
		QMessageBox::critical(this, tr("Help error"), tr("Failed to open URL:\n%1").arg(HELP_SITE_URL), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
}

void MainWindow::menuIRCOptions()
{
	IRCConfigurationDialog dialog(this);
	dialog.initOptionsList();
	dialog.exec();

	if (d->ircDock != nullptr)
	{
		d->ircDock->applyAppearanceSettings();

		// This could probably be optimized to not re-read files from drive
		// if audio options didn't change but currently there are only two
		// files, so no harm should be done.
		d->ircDock->sounds().loadFromConfig();
	}
}

void MainWindow::menuLog()
{
	d->logDock->setVisible(!d->logDock->isVisible());
}

void MainWindow::menuManageDemos()
{
	DemoManagerDlg dm;
	dm.setWindowIcon(this->windowIcon());
	dm.exec();
}

void MainWindow::menuOptionsConfigure()
{
	DoomseekerConfigurationDialog::openConfiguration(this);
}

void MainWindow::menuRecordDemo()
{
	gConfig.doomseeker.bRecordDemo = d->menuActionRecordDemo->isChecked();
}

void MainWindow::menuUpdateIP2C()
{
	IP2CUpdateBox updateBox(this);

	connect(&updateBox, SIGNAL(accepted()), this, SLOT(ip2cStartUpdate()));

	updateBox.exec();
}

void MainWindow::menuViewIRC()
{
	d->ircDock->setVisible(!d->ircDock->isVisible());
}

void MainWindow::menuWadSeeker()
{
	if (gWadseekerShow->checkWadseekerValidity(this))
	{
		WadseekerInterface *wadseeker = WadseekerInterface::create(nullptr);
		wadseeker->setAttribute(Qt::WA_DeleteOnClose);
		wadseeker->show();
	}
}

QProgressBar *MainWindow::mkStdProgressBarForStatusBar()
{
	auto pBar = new QProgressBar(statusBar());
	pBar->setAlignment(Qt::AlignCenter);
	pBar->setTextVisible(true);
	pBar->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	return pBar;
}

void MainWindow::notifyFirstRun()
{
	// On first run prompt configuration box.
	QMessageBox::information(nullptr, tr("Welcome to Doomseeker"),
		tr("Before you start browsing for servers, please ensure that Doomseeker is properly configured."));
	d->menuActionConfigure->trigger();
}

void MainWindow::onAutoUpdaterDownloadAndInstallConfirmationRequest()
{
	d->updatesConfirmationWidget->show();
}

void MainWindow::onAutoUpdaterFileProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	d->autoUpdaterFileProgressBar->setValue(bytesReceived);
	d->autoUpdaterFileProgressBar->setMaximum(bytesTotal);
}

void MainWindow::onAutoUpdaterFinish()
{
	showAndLogStatusMessage(tr("Program update detection & download finished with status: [%1] %2")
		.arg((int)d->autoUpdater->errorCode()).arg(d->autoUpdater->errorString()));
	gConfig.autoUpdates.bPerformUpdateOnNextRun = false;
	if (d->autoUpdater->errorCode() == AutoUpdater::EC_Ok)
	{
		UpdateChannel channel = UpdateChannel::fromName(gConfig.autoUpdates.updateChannelName);
		if (channel == *d->updateChannelOnUpdateStart)
		{
			if (!d->autoUpdater->downloadedPackagesFilenames().isEmpty())
			{
				gLog << tr("Updates will be installed on next program start.");
				d->updatesDownloadedWidget->show();
				gConfig.autoUpdates.bPerformUpdateOnNextRun = true;
			}
		}
		else
			gLog << tr("Update channel was changed during update process. Discarding update.");
	}
	gConfig.saveToFile();
	d->autoUpdaterStatusBarWidget->hide();
	d->updatesConfirmationWidget->hide();
	d->autoUpdater->deleteLater();
	d->autoUpdater = nullptr;
}

void MainWindow::onAutoUpdaterOverallProgress(int current, int total,
	const QString &msg)
{
	d->autoUpdaterOverallProgressBar->setValue(current);
	d->autoUpdaterOverallProgressBar->setMaximum(total);
	d->autoUpdaterOverallProgressBar->setFormat(msg);
}

void MainWindow::postInitAppStartup()
{
	// Load server filter from config.
	d->serverFilterDock->setFilterInfo(gConfig.serverFilter.info);

	fixIconsDpi();

	// Check query on statup
	// Let's see if we have any plugins first. If not, display error.
	if (gPlugins->numPlugins() > 0)
	{
		bool bGettingServers = false;
		bool queryOnStartup = gConfig.doomseeker.bQueryOnStartup;
		if (queryOnStartup)
		{
			// If "Query on startup" option is enabled we should
			// attempt to refresh any masters that are enabled
			// in the Query menu.

			if (isAnyMasterEnabled())
			{
				bGettingServers = true;
				getServers();
			}
			else
				gLog << tr("Query on startup warning: No master servers are enabled in the Query menu.");
		}

		// If we already successfully called the getServers() method
		// there is no need to call refreshCustomServers().
		if (!bGettingServers && hasCustomServers())
		{
			// Custom servers should be refreshed no matter what.
			// They will not block the app in any way, there is no reason
			// not to refresh them.
			refreshCustomServers();
		}
	}
	else
	{
		// There are no plugins so something is really bad.
		// Display error message.
		QString error = tr("Doomseeker was unable to find any plugin libraries.\n"
			"Although the application will still work it will not be possible "
			"to fetch any server info or launch any game.\n\n"
			"Please check if there are any files in \"engines/\" directory.\n"
			"To fix this problem you may try downloading Doomseeker "
			"again from the site specified in the Help|About box and reinstalling "
			"Doomseeker.");
		QMessageBox::critical(nullptr, tr("Doomseeker critical error"), error);
	}
}

QueryMenuAction *MainWindow::queryMenuActionForPlugin(const EnginePlugin *plugin)
{
	if (plugin == nullptr)
		return nullptr;

	if (d->queryMenuPorts.contains(plugin))
		return d->queryMenuPorts[plugin];

	return nullptr;
}

void MainWindow::quitProgram()
{
	d->bWantToQuit = true;
	QApplication::closeAllWindows();
}

void MainWindow::refreshCustomServers()
{
	for (const ServerPtr &server : d->serverList->servers())
	{
		if (server->isCustom())
			gRefresher->registerServer(server.data());
	}
}

void MainWindow::refreshServersOnList()
{
	for (const ServerPtr &server : d->serverList->servers())
	{
		gRefresher->registerServer(server.data());
	}
}

void MainWindow::refreshThreadBeginsWork()
{
	statusBar()->showMessage(tr("Querying..."));
	d->taskbarProgress->show();
}

void MainWindow::refreshThreadEndsWork()
{
	d->toolBarGetServers->setEnabled(true);

	d->serverList->cleanUpRightNow();
	statusBar()->showMessage(tr("Done"));
	updateTrayIconTooltipAndLogTotalRefresh();
	d->taskbarProgress->hide();

	if (d->bTotalRefreshInProcess)
		initAutoRefreshTimer();

	d->bTotalRefreshInProcess = false;
	QList<ServerPtr> selectedServers = d->serverList->selectedServers();
	d->detailsDock->displaySelection(selectedServers);
}

void MainWindow::restartAndInstallUpdatesNow()
{
	Main::bInstallUpdatesAndRestart = true;
	quitProgram();
}

void MainWindow::runGame(const ServerPtr &server)
{
	if (d->connectionHandler)
		delete d->connectionHandler;

	d->connectionHandler = new ConnectionHandler(server, this);
	d->connectionHandler->run();
}

void MainWindow::setQueryPluginEnabled(const EnginePlugin *plugin, bool bEnabled)
{
	assert(plugin != nullptr);

	QueryMenuAction *pAction = queryMenuActionForPlugin(plugin);
	if (pAction != nullptr)
	{
		pAction->setChecked(bEnabled);
		if (plugin->data()->hasMasterClient())
			plugin->data()->masterClient->setEnabled(bEnabled);
		if (plugin->data()->hasBroadcast())
			plugin->data()->broadcast->setEnabled(bEnabled);
		d->serversStatusesWidgets[plugin]->setMasterEnabledStatus(bEnabled);
	}
}

void MainWindow::serverAddedToList(const ServerPtr &pServer)
{
	if (pServer->isKnown())
	{
		const QString &gameMode = pServer->gameMode().name();
		d->serverFilterDock->addGameModeToComboBox(gameMode);
	}
}

void MainWindow::setDisplayUpdaterProcessFailure(int errorCode)
{
	assert(this->d->updaterInstallerErrorCode == 0 &&
		"MainWindow::setDisplayUpdaterProcessFailure()");
	this->d->updaterInstallerErrorCode = errorCode;
	QTimer::singleShot(0, this, SLOT(showUpdaterProcessErrorDialog()));
}

void MainWindow::setDisplayUpdateInstallerError(int errorCode)
{
	assert(this->d->updaterInstallerErrorCode == 0 &&
		"MainWindow::setDisplayUpdateInstallerError()");
	this->d->updaterInstallerErrorCode = errorCode;
	QTimer::singleShot(0, this, SLOT(showUpdateInstallErrorDialog()));
}

void MainWindow::setupIcons()
{
	QStyle &style = *QApplication::style();

	// File menu.
	d->menuActionQuit->setIcon(style.standardIcon(QStyle::SP_TitleBarCloseButton));

	// Help menu.
	d->menuActionHelp->setIcon(style.standardIcon(QStyle::SP_MessageBoxQuestion));
	d->menuActionAbout->setIcon(style.standardIcon(QStyle::SP_MessageBoxInformation));
}

void MainWindow::setupToolBar()
{
	QToolBar *pToolBar = new QToolBar(tr("Main Toolbar"), this);
	pToolBar->setMovable(false);
	pToolBar->setObjectName("Toolbar");

	// Refresh buttons
	d->toolBarGetServers = new QAction(QIcon(":/icons/refresh.png"), tr("Get Servers"), pToolBar);

	// Setup menu
	// Refresh buttons
	pToolBar->addAction(d->toolBarGetServers);

	// File menu buttons.
	pToolBar->addSeparator();
	pToolBar->addAction(d->menuActionCreateServer);
	pToolBar->addAction(d->menuActionWadseeker);

	// Demo buttons
	pToolBar->addSeparator();
	pToolBar->addAction(d->menuActionManageDemos);
	pToolBar->addAction(d->menuActionRecordDemo);

	pToolBar->addSeparator();

	// Dockable windows buttons.
	pToolBar->addAction(d->buddiesList->toggleViewAction());
	pToolBar->addAction(d->logDock->toggleViewAction());
	pToolBar->addAction(d->ircDock->toggleViewAction());
	pToolBar->addAction(d->serverFilterDock->toggleViewAction());
	pToolBar->addAction(d->detailsDock->toggleViewAction());

	// Quick Search
	QLineEdit *qs = d->serverFilterDock->createQuickSearch();
	qs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	qs->setMinimumWidth(175);
	qs->setMaximumWidth(175);

	QWidget *searchSeparator = new QWidget();
	searchSeparator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
	pToolBar->addWidget(searchSeparator);
	pToolBar->addWidget(new QLabel(tr("Search:"), pToolBar));
	pToolBar->addWidget(qs);

	this->addToolBar(Qt::TopToolBarArea, pToolBar);
	setUnifiedTitleAndToolBarOnMac(true);
	connect(pToolBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(toolBarAction(QAction*)));
}

void MainWindow::showAndLogStatusMessage(const QString &message)
{
	gLog << message;
	statusBar()->showMessage(message);
}

void MainWindow::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	// http://stackoverflow.com/a/26910648/1089357
	d->taskbarButton->setWindow(windowHandle());
}

void MainWindow::showInstallFreedoomDialog()
{
	if (!d->freedoomDialog.isNull())
	{
		d->freedoomDialog->activateWindow();
		return;
	}
	auto dialog = new FreedoomDialog(nullptr);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
	d->freedoomDialog = dialog;
}

void MainWindow::showProgramArgsHelp()
{
	auto dialog = new ProgramArgsHelpDialog(this);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void MainWindow::showServerJoinCommandLine(const ServerPtr &server)
{
	CommandLineInfo cli;
	ConnectionHandler connectionHandler(server, this);
	GameDemo demo = gConfig.doomseeker.bRecordDemo ? GameDemo::Unmanaged : GameDemo::NoDemo;
	JoinCommandLineBuilder *builder = new JoinCommandLineBuilder(server, demo, this);
	this->connect(builder, SIGNAL(commandLineBuildFinished()), SLOT(showServerJoinCommandLineOnBuilderFinished()));
	builder->obtainJoinCommandLine();
}

void MainWindow::showServerJoinCommandLineOnBuilderFinished()
{
	auto builder = static_cast<JoinCommandLineBuilder *>(sender());
	CommandLineInfo cli = builder->builtCommandLine();
	if (cli.isValid())
	{
		QString execPath = cli.executable.absoluteFilePath();
		QStringList args = cli.args;

		CommandLine::escapeExecutable(execPath);
		CommandLine::escapeArgs(args);

		CopyTextDlg ctd(execPath + " " + args.join(" "), builder->server()->name(), this);
		ctd.exec();
	}
	else
	{
		if (!builder->error().isEmpty())
		{
			QMessageBox::critical(this, tr("Doomseeker - show join command line"),
				tr("Command line cannot be built:\n%1").arg(builder->error()));
		}
	}
	builder->deleteLater();
}

void MainWindow::showUpdaterProcessErrorDialog()
{
	QString explanation;
	if (this->d->updaterInstallerErrorCode != UpdateInstaller::PEC_GeneralFailure)
	{
		QString errorCodeExplanation = UpdateInstaller::processErrorCodeToStr(
			(UpdateInstaller::ProcessErrorCode) this->d->updaterInstallerErrorCode);
		explanation = tr("Update installation problem:\n%1").arg(errorCodeExplanation);
	}
	else
		explanation = tr("Update installation failed.");
	QMessageBox::critical(this, tr("Doomseeker - Auto Update problem"),
		tr("%1\n\nRemaining updates have been discarded.").arg(explanation));
}

void MainWindow::showUpdateInstallErrorDialog()
{
	QString error = UpdateInstaller::errorCodeToStr(
		(UpdateInstaller::ErrorCode)this->d->updaterInstallerErrorCode);
	QString msg = tr("Update install problem:\n%1\n\nRemaining updates have been discarded.").arg(error);
	QMessageBox::critical(this, tr("Doomseeker - Auto Update problem"), msg);
}

void MainWindow::stopAutoRefreshTimer()
{
	d->autoRefreshTimer.stop();
}

void MainWindow::togglePluginQueryEnabled(const EnginePlugin *plugin)
{
	QueryMenuAction *pAction = queryMenuActionForPlugin(plugin);
	assert(pAction != nullptr);

	setQueryPluginEnabled(plugin, !pAction->isChecked());
}

void MainWindow::toolBarAction(QAction *pAction)
{
	if (pAction == d->toolBarGetServers)
		getServers();
}

void MainWindow::trayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
	{
		if (isMinimized() || !isVisible())
		{
			d->bWasMaximized == true ? showMaximized() : showNormal();
			activateWindow();
		}
		else if (gConfig.doomseeker.bCloseToTrayIcon)
			close();
		else
			showMinimized();
	}
}

void MainWindow::updateDynamicAppearance()
{
	d->tableServers->setShowGrid(gConfig.doomseeker.bDrawGridInServerTable);
	d->serverList->redraw();
	d->serverList->cleanUpForce();
}

// NOTE: Probably would be better if the master manager wasn't tied to the
//       MainWindow class?
void MainWindow::updateMasterAddresses()
{
	for (int i = 0; i < d->masterManager->numMasters(); i++)
		(*d->masterManager)[i]->updateAddress();
}

void MainWindow::updateServerFilter(const ServerListFilterInfo &filterInfo)
{
	d->serverList->applyFilter(filterInfo);
	d->lblServerFilterApplied->setVisible(filterInfo.isFilteringAnything());
}

ServerListCount MainWindow::sumServerListCount() const
{
	ServerListCount count;
	for (const ServersStatusWidget *status : d->serversStatusesWidgets.values())
	{
		count += status->count();
	}
	return count;
}

void MainWindow::updateRefreshProgress()
{
	ServerListCount count = sumServerListCount();
	d->taskbarProgress->setMaximum(count.numServers);
	d->taskbarProgress->setValue(count.numServers - count.numRefreshing);
	updateTrayIconTooltip(count);
}

void MainWindow::updateTrayIconTooltip(const ServerListCount &count)
{
	if (d->trayIcon != nullptr)
	{
		QString tip;
		tip += tr("Generic servers: %1\n").arg(count.numGenericServers);
		tip += tr("Custom servers: %1\n").arg(count.numCustomServers);
		tip += tr("LAN servers: %1\n").arg(count.numLanServers);
		tip += tr("Human players: %1").arg(count.numHumanPlayers);
		d->trayIcon->setToolTip(tip);
	}
}

void MainWindow::updateTrayIconTooltipAndLogTotalRefresh()
{
	ServerListCount count = sumServerListCount();
	updateTrayIconTooltip(count);

	if (d->bTotalRefreshInProcess)
	{
		gLog << tr("Finished refreshing. Servers on the list: %1 "
				"(+%2 custom, +%3 LAN). Players: %4.")
			.arg(count.numGenericServers).arg(count.numCustomServers)
			.arg(count.numLanServers).arg(count.numHumanPlayers);
	}
}
