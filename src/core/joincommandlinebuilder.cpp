//------------------------------------------------------------------------------
// joincommandlinebuilder.cpp
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
#include "joincommandlinebuilder.h"

#include "application.h"
#include "apprunner.h"
#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "gamedemo.h"
#include "gui/passworddlg.h"
#include "gui/wadseekerinterface.h"
#include "gui/wadseekershow.h"
#include "ini/settingsproviderqt.h"
#include "log.h"
#include "plugins/engineplugin.h"
#include "serverapi/exefile.h"
#include "serverapi/gameclientrunner.h"
#include "serverapi/message.h"
#include "serverapi/server.h"

#include <cassert>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <wadseeker/wadseeker.h>

DClass<JoinCommandLineBuilder>
{
public:
	CommandLineInfo cli;
	bool configurationError;
	QString connectPassword;
	QString error;
	GameDemo demo;
	QString demoName;
	QString inGamePassword;
	ServerPtr server;
	QWidget *parentWidget;
	bool passwordsAlreadySet;

	// For missing wads dialog
	QDialogButtonBox *buttonBox;
	QDialogButtonBox::StandardButton lastButtonClicked;
};

DPointered(JoinCommandLineBuilder)

JoinCommandLineBuilder::JoinCommandLineBuilder(ServerPtr server,
	GameDemo demo, QWidget *parentWidget)
{
	d->configurationError = false;
	d->demo = demo;
	d->demoName = GameDemo::mkDemoFullPath(demo, *server->plugin());
	d->parentWidget = parentWidget;
	d->passwordsAlreadySet = false;
	d->server = server;
}

JoinCommandLineBuilder::~JoinCommandLineBuilder()
{
}

bool JoinCommandLineBuilder::buildServerConnectParams(ServerConnectParams &params)
{
	if (d->server->isLockedAnywhere())
	{
		if (!d->passwordsAlreadySet)
		{
			PasswordDlg password(d->server);
			int ret = password.exec();
			if (ret != QDialog::Accepted)
			{
				return false;
			}
			d->connectPassword = password.connectPassword();
			d->inGamePassword = password.inGamePassword();
			d->passwordsAlreadySet = true;
		}
		params.setConnectPassword(d->connectPassword);
		params.setInGamePassword(d->inGamePassword);
	}

	if (!d->demoName.isEmpty())
	{
		params.setDemoName(d->demoName);
	}
	return true;
}

const CommandLineInfo &JoinCommandLineBuilder::builtCommandLine() const
{
	return d->cli;
}

bool JoinCommandLineBuilder::checkServerStatus()
{
	// Remember to check REFRESHING status first!
	if (d->server->isRefreshing())
	{
		d->error = tr("This server is still refreshing.\nPlease wait until it is finished.");
		gLog << tr("Attempted to obtain a join command line for a \"%1\" "
				"server that is under refresh.").arg(d->server->addressWithPort());
		return false;
	}
	// Fail if Doomseeker couldn't get data on this server.
	else if (!d->server->isKnown())
	{
		d->error = tr("Data for this server is not available.\nOperation failed.");
		gLog << tr("Attempted to obtain a join command line for an unknown server \"%1\"").arg(
				d->server->addressWithPort());
		return false;
	}
	return true;
}

bool JoinCommandLineBuilder::checkWadseekerValidity(QWidget *parent)
{
	Q_UNUSED(parent);
	QString targetDirPath = gDefaultDataPaths->resolveTemplated(gConfig.wadseeker.targetDirectory);
	QDir targetDir(targetDirPath);
	QFileInfo targetDirFileInfo(targetDirPath);

	if (targetDirPath.isEmpty() || !targetDir.exists() || !targetDirFileInfo.isWritable())
	{
		return false;
	}

	return true;
}

const QString &JoinCommandLineBuilder::error() const
{
	return d->error;
}

void JoinCommandLineBuilder::failBuild()
{
	d->cli = CommandLineInfo();
	emit commandLineBuildFinished();
}

void JoinCommandLineBuilder::handleError(const JoinError &error)
{
	if (!error.error().isEmpty())
	{
		d->error = error.error();
	}
	else
	{
		d->error = tr("Unknown error.");
	}
	d->configurationError = (error.type() == JoinError::ConfigurationError);

	gLog << tr("Error when obtaining join parameters for server "
			"\"%1\", game \"%2\": %3").arg(d->server->name()).arg(
			d->server->engineName()).arg(d->error);
}

MissingWadsDialog::MissingWadsProceed JoinCommandLineBuilder::handleMissingWads(const JoinError &error)
{
	QList<PWad> missingWads;
	if (!error.missingIwad().isEmpty())
	{
		missingWads << error.missingIwad();
	}
	if (!error.missingWads().isEmpty())
	{
		missingWads << error.missingWads();
	}

	MissingWadsDialog dialog(missingWads, error.incompatibleWads(), d->server->plugin(), d->parentWidget);
	if (dialog.exec() == QDialog::Accepted)
	{
		if (dialog.decision() == MissingWadsDialog::Install)
		{
			if (!gWadseekerShow->checkWadseekerValidity(d->parentWidget))
			{
				return MissingWadsDialog::Cancel;
			}
			WadseekerInterface *wadseeker = WadseekerInterface::create(d->server);
			this->connect(wadseeker, SIGNAL(finished(int)), SLOT(onWadseekerDone(int)));
			wadseeker->setWads(dialog.filesToDownload());
			wadseeker->setAttribute(Qt::WA_DeleteOnClose);
			wadseeker->show();
		}
	}
	return dialog.decision();
}

bool JoinCommandLineBuilder::isConfigurationError() const
{
	return d->configurationError;
}

void JoinCommandLineBuilder::missingWadsClicked(QAbstractButton *button)
{
	d->lastButtonClicked = d->buttonBox->standardButton(button);
}

void JoinCommandLineBuilder::obtainJoinCommandLine()
{
	assert(d->server != nullptr);
	d->cli = CommandLineInfo();

	if (!checkServerStatus())
	{
		failBuild();
		return;
	}

	ServerConnectParams params;
	if (!buildServerConnectParams(params))
	{
		failBuild();
		return;
	}
	GameClientRunner *gameRunner = d->server->gameRunner();
	JoinError joinError = gameRunner->createJoinCommandLine(d->cli, params);
	delete gameRunner;

	switch (joinError.type())
	{
	case JoinError::Terminate:
		failBuild();
		return;
	case JoinError::ConfigurationError:
	case JoinError::Critical:
	{
		handleError(joinError);
		failBuild();
		return;
	}

	case JoinError::CanAutomaticallyInstallGame:
	{
		if (tryToInstallGame())
		{
			obtainJoinCommandLine();
		}
		else
		{
			failBuild();
		}
		return;
	}

	case JoinError::MissingWads:
	{
		MissingWadsDialog::MissingWadsProceed proceed =
			handleMissingWads(joinError);
		switch (proceed)
		{
		case MissingWadsDialog::Cancel:
			failBuild();
			return;
		case MissingWadsDialog::Ignore:
			break;
		case MissingWadsDialog::Install:
			// async process; will call slot
			return;
		default:
			gLog << "Bug: not sure how to proceed after \"MissingWads\".";
			failBuild();
			return;
		}
		[[gnu::fallthrough]];
	}

	case JoinError::NoError:
		if (d->demo == GameDemo::Managed)
		{
			GameDemo::saveDemoMetaData(d->demoName, *d->server->plugin(),
				d->server->iwad(), d->server->wads());
		}
		break;

	default:
		gLog << "JoinCommandLineBuilder - unhandled JoinError type!";
		break;
	}

	emit commandLineBuildFinished();
}

void JoinCommandLineBuilder::onWadseekerDone(int result)
{
	if (result == QDialog::Accepted)
	{
		obtainJoinCommandLine();
	}
	else
	{
		failBuild();
	}
}

ServerPtr JoinCommandLineBuilder::server() const
{
	return d->server;
}

void JoinCommandLineBuilder::setPasswords(const QString &connectPassword, const QString &inGamePassword)
{
	d->passwordsAlreadySet = !(connectPassword.isNull() && inGamePassword.isNull());
	if (!connectPassword.isNull())
		d->connectPassword = connectPassword;
	if (!inGamePassword.isNull())
		d->inGamePassword = inGamePassword;
}

bool JoinCommandLineBuilder::tryToInstallGame()
{
	Message message = d->server->clientExe()->install(gApp->mainWindowAsQWidget());
	if (message.isError())
	{
		QMessageBox::critical(gApp->mainWindowAsQWidget(), tr("Game installation failure"),
			message.contents(), QMessageBox::Ok);
	}
	return message.type() == Message::Type::SUCCESSFUL;
}
