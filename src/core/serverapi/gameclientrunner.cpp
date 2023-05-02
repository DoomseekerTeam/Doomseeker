//------------------------------------------------------------------------------
// gameclientrunner.cpp
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
#include "gameclientrunner.h"

#include "apprunner.h"
#include "commandlinetokenizer.h"
#include "configuration/doomseekerconfig.h"
#include "gui/checkwadsdlg.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "log.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "plugins/engineplugin.h"
#include "serverapi/exefile.h"
#include "serverapi/gameexeretriever.h"
#include "serverapi/message.h"
#include "serverapi/server.h"
#include "templatedpathresolver.h"
#include <QDir>
#include <QScopedPointer>
#include <QStringList>

DClass<ServerConnectParams>
{
public:
	QString connectPassword;
	QString demoName;
	QString inGamePassword;
};

DPointered(ServerConnectParams)

ServerConnectParams::ServerConnectParams()
{
}

ServerConnectParams::ServerConnectParams(const ServerConnectParams &other)
{
	d = other.d;
}

ServerConnectParams &ServerConnectParams::operator=(const ServerConnectParams &other)
{
	d = other.d;
	return *this;
}

ServerConnectParams::~ServerConnectParams()
{
}

const QString &ServerConnectParams::connectPassword() const
{
	return d->connectPassword;
}

const QString &ServerConnectParams::demoName() const
{
	return d->demoName;
}

const QString &ServerConnectParams::inGamePassword() const
{
	return d->inGamePassword;
}

void ServerConnectParams::setConnectPassword(const QString &val)
{
	d->connectPassword = val;
}

void ServerConnectParams::setDemoName(const QString &val)
{
	d->demoName = val;
}

void ServerConnectParams::setInGamePassword(const QString &val)
{
	d->inGamePassword = val;
}
///////////////////////////////////////////////////////////////////////////////
#define BAIL_ON_ERROR(method) \
	{ \
		method; \
		if (isFatalError()) \
		{ \
			return; \
		} \
	}


DClass<GameClientRunner>
{
public:
	QString argBexLoading;
	QString argConnect;
	QString argConnectPassword;
	QString argDehLoading;
	QString argInGamePassword;
	QString argIwadLoading;
	QString argOptionalWadLoading;
	QString argPort;
	QString argPwadLoading;
	QString argDemoRecord;

	QStringList args;
	mutable QString cachedIwadPath;
	ServerConnectParams connectParams;
	CommandLineInfo *cli;
	JoinError joinError;
	QList<PWad> missingPwads;
	QList<PWad> incompatiblePwads;
	PathFinder pathFinder;
	ServerPtr server;

	void (GameClientRunner::*addConnectCommand)();
	void (GameClientRunner::*addExtra)();
	void (GameClientRunner::*addGamePaths)();
	void (GameClientRunner::*addInGamePassword)();
	void (GameClientRunner::*addIwad)();
	void (GameClientRunner::*addModFiles)(const QStringList &);
	void (GameClientRunner::*addPassword)();
	void (GameClientRunner::*createCommandLineArguments)();
};

DPointered(GameClientRunner)

POLYMORPHIC_DEFINE(void, GameClientRunner, addConnectCommand, (), ())
POLYMORPHIC_DEFINE(void, GameClientRunner, addExtra, (), ())
POLYMORPHIC_DEFINE(void, GameClientRunner, addGamePaths, (), ())
POLYMORPHIC_DEFINE(void, GameClientRunner, addInGamePassword, (), ())
POLYMORPHIC_DEFINE(void, GameClientRunner, addIwad, (), ())
POLYMORPHIC_DEFINE(void, GameClientRunner, addModFiles, (const QStringList &files), (files))
POLYMORPHIC_DEFINE(void, GameClientRunner, addPassword, (), ())
POLYMORPHIC_DEFINE(void, GameClientRunner, createCommandLineArguments, (), ())

GameClientRunner::GameClientRunner(ServerPtr server)
{
	set_addConnectCommand(&GameClientRunner::addConnectCommand_default);
	set_addGamePaths(&GameClientRunner::addGamePaths_default);
	set_addExtra(&GameClientRunner::addExtra_default);
	set_addInGamePassword(&GameClientRunner::addInGamePassword_default);
	set_addIwad(&GameClientRunner::addIwad_default);
	set_addModFiles(&GameClientRunner::addModFiles_default);
	set_addPassword(&GameClientRunner::addPassword_default);
	set_createCommandLineArguments(&GameClientRunner::createCommandLineArguments_default);
	d->argBexLoading = "-deh";
	d->argConnect = "-connect";
	d->argDehLoading = "-deh";
	d->argIwadLoading = "-iwad";
	d->argOptionalWadLoading = "-file"; // Assume one does not have this feature.
	d->argPort = "-port";
	d->argPwadLoading = "-file";
	d->argDemoRecord = "-record";
	d->cli = nullptr;
	d->server = server;
}

GameClientRunner::~GameClientRunner()
{
}

void GameClientRunner::addConnectCommand_default()
{
	QString address = QString("%1:%2").arg(d->server->address().toString()).arg(d->server->port());
	args() << argForConnect() << address;
}

void GameClientRunner::addCustomParameters()
{
	IniSection config = gConfig.iniSectionForPlugin(d->server->plugin());
	QString customParameters = config["CustomParameters"];
	CommandLineTokenizer tokenizer;
	args() << tokenizer.tokenize(customParameters);
}

void GameClientRunner::addDemoRecordCommand()
{
	args() << argForDemoRecord() << demoName();
}

void GameClientRunner::addGamePaths_default()
{
	GamePaths paths = gamePaths();
	if (!paths.isValid())
	{
		// gamePaths() sets JoinError.
		return;
	}

	QDir applicationDir = paths.workingDir;
	if (paths.workingDir.isEmpty())
	{
		d->joinError.setType(JoinError::ConfigurationError);
		d->joinError.setError(tr("Path to working directory for game \"%1\" is empty.\n\n"
			"Make sure the configuration for the client executable is set properly.")
				.arg(pluginName()));
		return;
	}
	else if (!applicationDir.exists())
	{
		d->joinError.setType(JoinError::ConfigurationError);
		d->joinError.setError(tr("%1\n\nThis directory cannot be used as working "
			"directory for game: %2\n\nExecutable: %3")
				.arg(paths.workingDir, pluginName(), paths.clientExe));
		return;
	}

	setExecutable(paths.clientExe);
	setWorkingDir(applicationDir.path());
}

void GameClientRunner::addInGamePassword_default()
{
	if (!argForInGamePassword().isNull())
	{
		args() << argForInGamePassword() << inGamePassword();
	}
	else
	{
		gLog << tr("BUG: Plugin doesn't specify argument for in-game "
				"password, but the server requires such password.");
	}
}

void GameClientRunner::addExtra_default()
{
}

void GameClientRunner::addIwad_default()
{
	args() << argForIwadLoading() << iwadPath();
}

void GameClientRunner::addWads()
{
	addIwad();
	addPwads();

	if (!d->incompatiblePwads.isEmpty())
	{
		d->joinError.setIncompatibleWads(d->incompatiblePwads);
		d->joinError.setType(JoinError::MissingWads);
	}
	if (!isIwadFound() || !d->missingPwads.isEmpty())
	{
		if (!isIwadFound())
		{
			d->joinError.setMissingIwad(d->server->iwad());
			d->joinError.setType(JoinError::MissingWads);
		}
		d->joinError.setMissingWads(d->missingPwads);
		d->joinError.setType(JoinError::NoError);
		for (const PWad &wad : d->missingPwads)
		{
			// Only error if there are required missing wads
			if (!wad.isOptional())
			{
				d->joinError.setType(JoinError::MissingWads);
				break;
			}
		}
	}
}

void GameClientRunner::addPassword_default()
{
	if (!argForConnectPassword().isNull())
	{
		args() << argForConnectPassword() << connectPassword();
	}
	else
	{
		gLog << tr("BUG: Plugin doesn't specify argument for connect "
				"password, but the server is passworded.");
	}
}

void GameClientRunner::addPwads()
{
	auto checkWadsDlg = new CheckWadsDlg(&d->pathFinder);
	checkWadsDlg->addWads(d->server->wads());
	const CheckResult checkResults = checkWadsDlg->checkWads();
	for (const PWad &wad : checkResults.missingWads)
	{
		markPwadAsMissing(wad);
	}
	for (const PWad &wad : checkResults.incompatibleWads)
	{
		markPwadAsIncompatible(wad);
	}
	QStringList paths;
	for (const PWad &wad : checkResults.foundWads)
	{
		paths << findWad(wad.name());
	}
	addModFiles(paths);
}

void GameClientRunner::addModFiles_default(const QStringList &files)
{
	for (const QString &file : files)
	{
		args() << fileLoadingPrefix(file) << file;
	}
}

void GameClientRunner::addModFiles_prefixOnce(const QStringList &files)
{
	QMap<QString, QStringList> groups;
	for (const QString &file : files)
	{
		QString prefix = fileLoadingPrefix(file);
		groups[prefix] << file;
	}
	for (const QString &prefix : groups.keys())
	{
		args() << prefix;
		for (const QString &file : groups[prefix])
		{
			args() << file;
		}
	}
}

QString GameClientRunner::fileLoadingPrefix(const QString &file) const
{
	if (file.toLower().endsWith(".deh"))
	{
		return argForDehLoading();
	}
	else if (file.toLower().endsWith(".bex"))
	{
		return argForBexLoading();
	}
	return argForPwadLoading();
}

QStringList &GameClientRunner::args()
{
	return d->cli->args;
}

const QString &GameClientRunner::argForBexLoading() const
{
	return d->argBexLoading;
}

const QString &GameClientRunner::argForConnect() const
{
	return d->argConnect;
}

const QString &GameClientRunner::argForConnectPassword() const
{
	return d->argConnectPassword;
}

const QString &GameClientRunner::argForDehLoading() const
{
	return d->argDehLoading;
}

const QString &GameClientRunner::argForInGamePassword() const
{
	return d->argInGamePassword;
}

const QString &GameClientRunner::argForIwadLoading() const
{
	return d->argIwadLoading;
}

const QString &GameClientRunner::argForOptionalWadLoading() const
{
	return d->argOptionalWadLoading;
}

const QString &GameClientRunner::argForPort() const
{
	return d->argPort;
}

const QString &GameClientRunner::argForPwadLoading() const
{
	return d->argPwadLoading;
}

const QString &GameClientRunner::argForDemoRecord() const
{
	return d->argDemoRecord;
}

bool GameClientRunner::canDownloadWadsInGame() const
{
	return d->server->plugin()->data()->inGameFileDownloads;
}

const QString &GameClientRunner::connectPassword() const
{
	return d->connectParams.connectPassword();
}

void GameClientRunner::createCommandLineArguments_default()
{
	BAIL_ON_ERROR(addConnectCommand());
	if (d->server->isLocked())
	{
		BAIL_ON_ERROR(addPassword());
	}
	if (d->server->isLockedInGame())
	{
		BAIL_ON_ERROR(addInGamePassword());
	}
	if (!demoName().isEmpty())
	{
		BAIL_ON_ERROR(addDemoRecordCommand());
	}
	BAIL_ON_ERROR(addWads());
	BAIL_ON_ERROR(addExtra());
	BAIL_ON_ERROR(addCustomParameters());
	BAIL_ON_ERROR(addGamePaths());
}

JoinError GameClientRunner::createJoinCommandLine(CommandLineInfo &cli,
	const ServerConnectParams &params)
{
	d->cli = &cli;
	d->cli->args.clear();
	d->connectParams = params;
	d->joinError = JoinError();

	setupPathFinder();
	if (d->joinError.isError())
	{
		return d->joinError;
	}
	createCommandLineArguments();

	return d->joinError;
}

const QString &GameClientRunner::demoName() const
{
	return d->connectParams.demoName();
}

QString GameClientRunner::findIwad() const
{
	return findWad(d->server->iwad().toLower());
}

QString GameClientRunner::findWad(const QString &wad) const
{
	return WadPathFinder(d->pathFinder).find(wad).path();
}

GameClientRunner::GamePaths GameClientRunner::gamePaths()
{
	Message msg;
	GamePaths result;

	QScopedPointer<ExeFile> exeFile(d->server->clientExe());
	result.clientExe = exeFile->pathToExe(msg, ExeFile::InternalChecking);
	if (result.clientExe.isEmpty())
	{
		if (msg.type() == Message::Type::GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED)
		{
			d->joinError.setType(JoinError::CanAutomaticallyInstallGame);
			if (msg.contents().isEmpty())
			{
				d->joinError.setError(msg.contents());
			}
			else
			{
				d->joinError.setError(tr("Game can be installed by Doomseeker"));
			}
		}
		else
		{
			d->joinError.setType(JoinError::ConfigurationError);
			QString error = tr("Game %1 cannot be found.").arg(pluginName());
			if (!msg.isIgnore())
			{
				error += "\n\n" + msg.contents();
			}
			d->joinError.setError(error);
		}
		return GamePaths();
	}
	result.workingDir = exeFile->workingDirectory(msg);
	if (result.workingDir.isEmpty() && d->server->plugin()->data()->allowsInternalClients &&
		result.clientExe.endsWith(ExeFile::scriptFileExtension()))
	{
		const QString internalPath = exeFile->versionInstallPath();
		if (result.clientExe.left(result.clientExe.lastIndexOf(ExeFile::scriptFileExtension())) == internalPath)
			result.workingDir = internalPath;
	}
	return result;
}

const QString &GameClientRunner::inGamePassword() const
{
	return d->connectParams.inGamePassword();
}

bool GameClientRunner::isFatalError() const
{
	if (d->joinError.isError())
	{
		if (d->joinError.isMissingWadsError() && canDownloadWadsInGame())
		{
			return false;
		}
		return true;
	}
	return false;
}

bool GameClientRunner::isIwadFound() const
{
	return !d->cachedIwadPath.isEmpty();
}

const QString &GameClientRunner::iwadPath() const
{
	if (!isIwadFound())
	{
		d->cachedIwadPath = findIwad();
	}
	return d->cachedIwadPath;
}

void GameClientRunner::markPwadAsMissing(const PWad &pwadName)
{
	d->missingPwads << pwadName;
}

void GameClientRunner::markPwadAsIncompatible(const PWad &pwadName)
{
	d->incompatiblePwads << pwadName;
}

PathFinder &GameClientRunner::pathFinder()
{
	return d->pathFinder;
}

const QString &GameClientRunner::pluginName() const
{
	return d->server->plugin()->data()->name;
}

ServerConnectParams &GameClientRunner::serverConnectParams()
{
	return d->connectParams;
}

void GameClientRunner::setArgForBexLoading(const QString &arg)
{
	d->argBexLoading = arg;
}

void GameClientRunner::setArgForConnect(const QString &arg)
{
	d->argConnect = arg;
}

void GameClientRunner::setArgForConnectPassword(const QString &arg)
{
	d->argConnectPassword = arg;
}

void GameClientRunner::setArgForDehLoading(const QString &arg)
{
	d->argDehLoading = arg;
}

void GameClientRunner::setArgForInGamePassword(const QString &arg)
{
	d->argInGamePassword = arg;
}

void GameClientRunner::setArgForIwadLoading(const QString &arg)
{
	d->argIwadLoading = arg;
}

void GameClientRunner::setArgForOptionalWadLoading(const QString &arg)
{
	d->argOptionalWadLoading = arg;
}

void GameClientRunner::setArgForPort(const QString &arg)
{
	d->argPort = arg;
}

void GameClientRunner::setArgForPwadLoading(const QString &arg)
{
	d->argPwadLoading = arg;
}

void GameClientRunner::setArgForDemoRecord(const QString &arg)
{
	d->argDemoRecord = arg;
}

void GameClientRunner::setExecutable(const QString &path)
{
	d->cli->executable = QFileInfo(gDoomseekerTemplatedPathResolver().resolve(path));
}

void GameClientRunner::setWorkingDir(const QString &path)
{
	d->cli->applicationDir.setPath(gDoomseekerTemplatedPathResolver().resolve(path));
}

JoinError GameClientRunner::joinError() const
{
	return d->joinError;
}

void GameClientRunner::setJoinError(const JoinError &e)
{
	d->joinError = e;
}

void GameClientRunner::setupPathFinder()
{
	d->pathFinder = d->server->wadPathFinder();
}

QString GameClientRunner::wadTargetDirectory() const
{
	return gDoomseekerTemplatedPathResolver().resolve(
		DoomseekerConfig::config().wadseeker.targetDirectory);
}
