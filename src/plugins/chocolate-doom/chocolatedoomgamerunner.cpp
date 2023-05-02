//------------------------------------------------------------------------------
// chocolatedoomgamerunner.cpp
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
#include "chocolatedoomgamerunner.h"

#include "chocolatedoombinaries.h"
#include "chocolatedoomgameexefactory.h"
#include "chocolatedoomgameinfo.h"
#include "chocolatedoomserver.h"

#include <gui/createserver/iwadandwadspickerdialog.h>
#include <ini/inisection.h>
#include <serverapi/createserverdialogapi.h>
#include <serverapi/gamefile.h>
#include <serverapi/message.h>
#include <serverapi/playerslist.h>
#include <QFileInfo>
#include <QRegularExpression>

ChocolateDoomGameClientRunner::ChocolateDoomGameClientRunner(QSharedPointer<ChocolateDoomServer> server)
	: GameClientRunner(server)
{
	this->server = server;
	set_addGamePaths(&ChocolateDoomGameClientRunner::addGamePaths);
	set_createCommandLineArguments(&ChocolateDoomGameClientRunner::createCommandLineArguments);
}

void ChocolateDoomGameClientRunner::createCommandLineArguments()
{
	addWads();
	if (joinError().type() == JoinError::NoError)
	{
		retrieveExecutableExeFile();
		if (!exeFilePath.isEmpty())
		{
			if (server->players().size() == 0)
			{
				configureEmptyServer();
			}
			else
			{
				joinPopulatedServer();
			}
			if (joinError().type() == JoinError::NoError)
			{
				addGamePaths();
				addConnectCommand();
				addCustomParameters();
			}
		}
	}
}

void ChocolateDoomGameClientRunner::addGamePaths()
{
	if (!overwriteExecutable.isEmpty())
	{
		setExecutable(overwriteExecutable);
		setWorkingDir(QFileInfo(overwriteExecutable).path());
	}
	else
		addGamePaths_default();
}

void ChocolateDoomGameClientRunner::configureEmptyServer()
{
	CreateServerDialogApi *csd = CreateServerDialogApi::createNew(nullptr);
	csd->dialog()->setAttribute(Qt::WA_DeleteOnClose, false);
	csd->makeRemoteGameSetup(plugin());
	csd->setIwadByName(QFileInfo(iwadPath()).fileName());
	csd->setRemoteExecutableByPath(exeFilePath);
	if (csd->dialog()->exec() == QDialog::Accepted)
		csd->fillInCommandLineArguments(overwriteExecutable, args());
	else
		setJoinError(JoinError(JoinError::Terminate));
	delete csd;
}

QStringList ChocolateDoomGameClientRunner::executables() const
{
	QStringList paths;
	paths << exeFilePath;
	QList<ExeFilePath> listExeFilePath(plugin()->gameExe()->additionalExecutables(GameFile::CreateGame));
	foreach (ExeFilePath exeFile, listExeFilePath)
	{
		paths << exeFile.path();
	}
	GameFile executableForIwad = ChocolateDoomGameExeFactory::executableForIwad(server->iwad());
	paths << plugin()->data()->pConfig->value(executableForIwad.configName()).toString();
	for (const GameFile &file : ChocolateDoomGameExeFactory::gameFiles().asQList())
	{
		paths << plugin()->data()->pConfig->value(file.configName()).toString();
	}
	paths = paths.filter(QRegularExpression("\\S+"));
	paths.removeDuplicates();
	return paths;
}

void ChocolateDoomGameClientRunner::joinPopulatedServer()
{
	auto dialog = new IwadAndWadsPickerDialog(nullptr);
	dialog->setWindowTitle(tr("Join Chocolate Doom game"));
	dialog->setExecutables(executables());
	dialog->setIwadByName(server->iwad());
	if (dialog->exec() == QDialog::Accepted)
	{
		overwriteExecutable = dialog->executable();
		args() << "-iwad" << dialog->iwadPath();
		for (const QString &file : dialog->filePaths())
		{
			if (file.toLower().endsWith(".deh"))
				args() << "-deh" << file;
			else
				args() << "-file" << file;
		}
	}
	else
		setJoinError(JoinError(JoinError::Terminate));
	delete dialog;
}

void ChocolateDoomGameClientRunner::retrieveExecutableExeFile()
{
	QScopedPointer<ExeFile> exeFile(server->clientExe());
	Message msg = Message();
	exeFilePath = exeFile->pathToExe(msg, ExeFile::InternalChecking);
	if (msg.type() == Message::Type::GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED)
	{
		setJoinError(JoinError::CanAutomaticallyInstallGame);
	}
}
