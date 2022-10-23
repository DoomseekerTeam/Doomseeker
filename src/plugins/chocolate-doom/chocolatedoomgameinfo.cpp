//------------------------------------------------------------------------------
// chocolatedoomgameinfo.h
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
// Copyright (C) 2013 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#include "chocolatedoomgameinfo.h"
#include <QObject>

QList<GameMode> ChocolateDoomGameInfo::gameModes()
{
	QList<GameMode> gameModesList;
	gameModesList << GameMode::mkCooperative();
	gameModesList << GameMode::mkDeathmatch();
	gameModesList << GameMode::ffaGame(MODE_ALTDEATH, QObject::tr("Altdeath (Deathmatch 2.0)"));
	return gameModesList;
}

QString ChocolateDoomAboutProvider::provide()
{
	QString aboutDescription = "Chocolate Doom (https://www.chocolate-doom.org/)\n\n";
	aboutDescription += tr("This plugin is distributed under the terms of the LGPL v2.1 or later.\n\n");
	return aboutDescription;
}