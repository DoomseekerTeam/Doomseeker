//------------------------------------------------------------------------------
// gameexefactory.cpp
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
#include "gameexefactory.h"

#include "plugins/engineplugin.h"
#include "serverapi/exefile.h"
#include "serverapi/gamefile.h"

DClass<GameExeFactory>
{
public:
	EnginePlugin *plugin;

	QList<ExeFilePath>(GameExeFactory::*additionalExecutables)(int) const;
	GameFileList (GameExeFactory::*gameFiles)() const;
};

DPointered(GameExeFactory)

GameExeFactory::GameExeFactory(EnginePlugin *plugin)
{
	d->plugin = plugin;

	set_additionalExecutables(&GameExeFactory::additionalExecutables_default);
	set_gameFiles(&GameExeFactory::gameFiles_default);
}

GameExeFactory::~GameExeFactory()
{
}

POLYMORPHIC_DEFINE_CONST(QList<ExeFilePath>, GameExeFactory, additionalExecutables, (int execType), (execType))
POLYMORPHIC_DEFINE_CONST(GameFileList, GameExeFactory, gameFiles, (), ())

EnginePlugin *GameExeFactory::plugin() const
{
	return d->plugin;
}

QList<ExeFilePath> GameExeFactory::additionalExecutables_default(int execType) const
{
	Q_UNUSED(execType);
	return QList<ExeFilePath>();
}

GameFileList GameExeFactory::gameFiles_default() const
{
	GameFile tmplate = GameFile().setSearchSuffixes(d->plugin->data()->gameFileSearchSuffixes);
	GameFileList list;
	if (d->plugin->data()->clientOnly)
	{
		list << GameFile(tmplate).setConfigName("BinaryPath").setNiceName(tr("game"))
			.setFileName(d->plugin->data()->clientExeName)
			.setExecutable(GameFile::Cso);
	}
	else
	{
		list << GameFile(tmplate).setConfigName("BinaryPath").setNiceName(tr("client"))
			.setFileName(d->plugin->data()->clientExeName)
			.setExecutable(GameFile::Offline | GameFile::Client);
		list << GameFile(tmplate).setConfigName("ServerBinaryPath").setNiceName(tr("server"))
			.setFileName(d->plugin->data()->serverExeName).setExecutable(GameFile::Server);
	}
	return list;
}
