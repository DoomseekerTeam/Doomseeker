//------------------------------------------------------------------------------
// gameexeretriever.cpp
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
#include "gameexeretriever.h"

#include "ini/inisection.h"
#include "plugins/engineplugin.h"
#include "serverapi/gameexefactory.h"
#include "serverapi/gamefile.h"
#include "serverapi/message.h"

GameExeRetriever::GameExeRetriever(GameExeFactory &factory)
	: factory(factory)
{
}

QString GameExeRetriever::pathToOfflineExe(Message &message)
{
	GameFile file = GameFiles::preferredOfflineExecutable(factory.gameFiles());
	if (!file.isValid())
	{
		message = Message::customError(tr("Game doesn't define offline executable."));
		return QString();
	}

	QString path = factory.plugin()->data()->pConfig->value(file.configName()).toString();
	if (path.trimmed().isEmpty())
	{
		message = Message::customError(tr("Game offline executable is not configured."));
		return QString();
	}
	return path;
}
