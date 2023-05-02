//------------------------------------------------------------------------------
// gamehost.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "gamehost.h"

#include "apprunner.h"
#include "commandlinetokenizer.h"
#include "configuration/doomseekerconfig.h"
#include "gamedemo.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/message.h"
#include "serverapi/serverstructs.h"
#include <cassert>
#include <QFileInfo>
#include <QStringList>

#define BAIL_ON_ERROR(method) \
	{ \
		method; \
		if (d->message.isError()) \
		{ \
			return; \
		} \
	}

DClass<GameHost>
{
public:
	QString argBexLoading;
	QString argDehLoading;
	QString argIwadLoading;
	QString argOptionalWadLoading;
	QString argPort;
	QString argPwadLoading;
	QString argDemoPlayback;
	QString argDemoRecord;
	QString argServerLaunch;

	CommandLineInfo *currentCmdLine;
	Message message;
	GameCreateParams params;
	EnginePlugin *plugin;

	void (GameHost::*addIwad)();
	void (GameHost::*addPwads)();
	void (GameHost::*addDMFlags)();
	void (GameHost::*addGlobalGameCustomParameters)();
};

DPointered(GameHost)

GameHost::GameHost(EnginePlugin *plugin)
{
	d->argBexLoading = "-deh";
	d->argDehLoading = "-deh";
	d->argIwadLoading = "-iwad";
	d->argOptionalWadLoading = "-file";
	d->argPort = "-port";
	d->argPwadLoading = "-file";
	d->argDemoPlayback = "-playdemo";
	d->argDemoRecord = "-record";
	d->currentCmdLine = nullptr;
	d->plugin = plugin;

	set_addIwad(&GameHost::addIwad_default);
	set_addPwads(&GameHost::addPwads_default);
	set_addDMFlags(&GameHost::addDMFlags_default);
	set_addGlobalGameCustomParameters(&GameHost::addGlobalGameCustomParameters_default);
}

GameHost::~GameHost()
{
}

POLYMORPHIC_DEFINE(void, GameHost, addIwad, (), ())
POLYMORPHIC_DEFINE(void, GameHost, addPwads, (), ())
POLYMORPHIC_DEFINE(void, GameHost, addDMFlags, (), ())
POLYMORPHIC_DEFINE(void, GameHost, addGlobalGameCustomParameters, (), ())

void GameHost::addCustomParameters()
{
	args().append(params().customParameters());
}

void GameHost::addDemoPlaybackIfApplicable()
{
	if (params().hostMode() == GameCreateParams::Demo)
	{
		args() << argForDemoPlayback();
		args() << params().demoPath();
	}
}

void GameHost::addDemoRecordIfApplicable()
{
	if (params().hostMode() == GameCreateParams::Offline
		&& params().demoRecord() != GameDemo::NoDemo)
	{
		args() << argForDemoRecord();
		args() << params().demoPath();
	}
}

void GameHost::addDMFlags_default()
{
}

void GameHost::addExtra()
{
}

void GameHost::addGlobalGameCustomParameters_default()
{
	IniSection config = gConfig.iniSectionForPlugin(plugin());
	QString customParameters = config["CustomParameters"];
	CommandLineTokenizer tokenizer;
	args() << tokenizer.tokenize(customParameters);
}

void GameHost::addIwad_default()
{
	const QString &iwadPath = params().iwadPath();

	if (iwadPath.isEmpty())
	{
		setMessage(Message::customError(tr("IWAD is not set")));
		return;
	}

	QFileInfo fi(iwadPath);

	if (!fi.isFile())
	{
		QString error = tr("IWAD Path error:\n\"%1\" doesn't exist or is a directory!").arg(iwadPath);
		setMessage(Message::customError(error));
	}

	args() << argForIwadLoading() << iwadPath;
}

void GameHost::addPwads_default()
{
	verifyPwadPaths();
	for (int i = 0; i < params().pwadsPaths().size(); ++i)
	{
		const QString &pwad = params().pwadsPaths()[i];
		args() << fileLoadingPrefix(i) << pwad;
	}
}

void GameHost::addPwads_prefixOnce()
{
	verifyPwadPaths();
	QMap<QString, QStringList> groups;
	for (int i = 0; i < params().pwadsPaths().size(); ++i)
	{
		const QString &pwad = params().pwadsPaths()[i];
		QString prefix = fileLoadingPrefix(i);
		groups[prefix] << pwad;
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

QString GameHost::fileLoadingPrefix(int index) const
{
	const QString &pwad = params().pwadsPaths()[index];
	bool optional = false;
	if (params().pwadsOptional().size() > index)
	{
		optional = params().pwadsOptional()[index];
	}

	if (pwad.toLower().endsWith(".deh"))
	{
		return argForDehLoading();
	}
	else if (pwad.toLower().endsWith(".bex"))
	{
		return argForBexLoading();
	}

	if (optional)
		return argForOptionalWadLoading();
	return argForPwadLoading();
}

const QString &GameHost::argForBexLoading() const
{
	return d->argBexLoading;
}

const QString &GameHost::argForDehLoading() const
{
	return d->argDehLoading;
}

const QString &GameHost::argForIwadLoading() const
{
	return d->argIwadLoading;
}

const QString &GameHost::argForOptionalWadLoading() const
{
	return d->argOptionalWadLoading;
}

const QString &GameHost::argForPort() const
{
	return d->argPort;
}

const QString &GameHost::argForPwadLoading() const
{
	return d->argPwadLoading;
}

const QString &GameHost::argForDemoPlayback() const
{
	return d->argDemoPlayback;
}

const QString &GameHost::argForDemoRecord() const
{
	return d->argDemoRecord;
}

const QString &GameHost::argForServerLaunch() const
{
	return d->argServerLaunch;
}

QStringList &GameHost::args()
{
	return d->currentCmdLine->args;
}

void GameHost::createCommandLineArguments()
{
	BAIL_ON_ERROR(addGlobalGameCustomParameters());
	BAIL_ON_ERROR(addIwad());
	BAIL_ON_ERROR(addPwads());

	// Port
	if (params().hostMode() == GameCreateParams::Host && params().port() > 0)
	{
		args() << argForPort() << QString::number(params().port());
	}

	// CVars
	const QList<GameCVar> &cvars = params().cvars();
	for (const GameCVar &c : cvars)
	{
		args() << QString(c.command()) << c.valueString();
	}

	if (params().hostMode() == GameCreateParams::Host)
	{
		// Some games may not offer such argument.
		if (!argForServerLaunch().isEmpty())
		{
			args() << argForServerLaunch();
		}
	}

	BAIL_ON_ERROR(addDMFlags());
	BAIL_ON_ERROR(addExtra());
	BAIL_ON_ERROR(addCustomParameters());

	addDemoPlaybackIfApplicable();
	addDemoRecordIfApplicable();
	saveDemoMetaData();
}

Message GameHost::createHostCommandLine(const GameCreateParams &params, CommandLineInfo &cmdLine)
{
	d->message = Message();
	d->currentCmdLine = &cmdLine;
	d->params = params;

	args().clear();

	setupGamePaths();
	if (d->message.isError())
	{
		return d->message;
	}

	createCommandLineArguments();
	return d->message;
}

Message GameHost::host(const GameCreateParams &params)
{
	CommandLineInfo cmdLine;

	Message message = createHostCommandLine(params, cmdLine);
	if (!message.isIgnore())
	{
		return message;
	}

	#ifdef Q_OS_WIN32
	const bool WRAP_IN_SSS_CONSOLE = false;
	#else
	const bool WRAP_IN_SSS_CONSOLE = params.hostMode() == GameCreateParams::Host;
	#endif

	if (WRAP_IN_SSS_CONSOLE)
	{
		QIcon icon;
		if (plugin() != nullptr)
		{
			icon = plugin()->icon();
		}
		AppRunner::runExecutableWrappedInStandardServerConsole(icon, cmdLine);
		return Message();
	}
	else
	{
		return AppRunner::runExecutable(cmdLine);
	}
}

const GameCreateParams &GameHost::params() const
{
	return d->params;
}

EnginePlugin *GameHost::plugin() const
{
	return d->plugin;
}

void GameHost::saveDemoMetaData()
{
	if (params().demoRecord() == GameDemo::Managed)
	{
		GameDemo::saveDemoMetaData(params().demoPath(), *plugin(),
			params().iwadName(), params().pwads());
	}
}

void GameHost::setArgForBexLoading(const QString &arg)
{
	d->argBexLoading = arg;
}

void GameHost::setArgForDehLoading(const QString &arg)
{
	d->argDehLoading = arg;
}

void GameHost::setArgForIwadLoading(const QString &arg)
{
	d->argIwadLoading = arg;
}

void GameHost::setArgForOptionalWadLoading(const QString &arg)
{
	d->argOptionalWadLoading = arg;
}

void GameHost::setArgForPort(const QString &arg)
{
	d->argPort = arg;
}

void GameHost::setArgForPwadLoading(const QString &arg)
{
	d->argPwadLoading = arg;
}

void GameHost::setArgForDemoPlayback(const QString &arg)
{
	d->argDemoPlayback = arg;
}

void GameHost::setArgForDemoRecord(const QString &arg)
{
	d->argDemoRecord = arg;
}

void GameHost::setArgForServerLaunch(const QString &arg)
{
	d->argServerLaunch = arg;
}

void GameHost::setMessage(const Message &message)
{
	d->message = message;
}

void GameHost::setupGamePaths()
{
	QFileInfo fileInfo(params().executablePath());
	if (!fileInfo.isFile() && !fileInfo.isBundle())
	{
		QString error = tr("%1\n doesn't exist or is not a file.").arg(fileInfo.filePath());
		setMessage(Message::customError(error));
		return;
	}
	d->currentCmdLine->executable = params().executablePath();
	d->currentCmdLine->applicationDir = fileInfo.dir();
}

bool GameHost::verifyPwadPaths()
{
	for (const QString &pwad : params().pwadsPaths())
	{
		QFileInfo fi(pwad);
		if (!fi.isFile())
		{
			QString error = tr("PWAD path error:\n\"%1\" doesn't exist or is a directory!").arg(pwad);
			setMessage(Message::customError(error));
			return false;
		}
	}
	return true;
}
