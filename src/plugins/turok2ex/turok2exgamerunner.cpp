//------------------------------------------------------------------------------
// Turok2Exgamerunner.cpp
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
// Copyright (C) 2017 "Edward Richardson" <Edward850@crantime.org>
//------------------------------------------------------------------------------
#include "turok2exgamerunner.h"

#include "turok2exgameinfo.h"
#include "turok2exserver.h"
#include <datapaths.h>

Turok2ExGameClientRunner::Turok2ExGameClientRunner(QSharedPointer<Turok2ExServer> server)
	: GameClientRunner(server)
{
	this->server = server;
	set_addConnectCommand(&Turok2ExGameClientRunner::addConnectCommand);
	set_addModFiles(&Turok2ExGameClientRunner::addModFiles_prefixOnce);
	set_addIwad(&Turok2ExGameClientRunner::addIwad);
}

void Turok2ExGameClientRunner::addConnectCommand()
{
	GameClientRunner::addConnectCommand_default();
}

void Turok2ExGameClientRunner::addFiles(const QStringList &files)
{
	Q_UNUSED(files);
	QString waddir = "";
	for (int i = 0; i < server->numWads(); ++i)
	{
		QString pwad = findWad(server->wad(i).name());
		if (pwad.length() > 0)
			waddir += " \"" + pwad + '"';
	}
	if (waddir.length() > 0)
		args() << "-file" << waddir;
}

void Turok2ExGameClientRunner::addIwad()
{
	iwadPath(); //claim we found it
}
