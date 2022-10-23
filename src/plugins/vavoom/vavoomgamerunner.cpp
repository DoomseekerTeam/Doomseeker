//------------------------------------------------------------------------------
// vavoomgamerunner.cpp
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
#include "vavoomgamerunner.h"

#include "vavoomengineplugin.h"
#include "vavoomgameinfo.h"
#include "vavoomserver.h"

VavoomGameClientRunner::VavoomGameClientRunner(QSharedPointer<VavoomServer> server)
	: GameClientRunner(server)
{
	this->server = server;
	setArgForConnect("+connect");
	set_addIwad(&VavoomGameClientRunner::addIwad);
}

void VavoomGameClientRunner::addIwad()
{
	// What an odd thing to have to do "-iwaddir /path/to/iwads/ -doom2"
	QString iwad = server->iwad();
	QString iwadLocation = iwadPath();
	QString iwadDir = iwadLocation.left(iwadLocation.length() - iwad.length());
	QString iwadParam = iwadLocation.mid(iwadDir.length());
	iwadParam.truncate(iwadParam.indexOf(QChar('.')));
	args() << "-iwaddir";
	args() << iwadDir;
	args() << ("-" + iwadParam);
}

const EnginePlugin *VavoomGameClientRunner::plugin() const
{
	return VavoomEnginePlugin::staticInstance();
}
