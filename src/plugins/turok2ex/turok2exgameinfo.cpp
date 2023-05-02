//------------------------------------------------------------------------------
// Turok2Exgameinfo.cpp
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
#include "turok2exgameinfo.h"

QList<GameMode> Turok2ExGameInfo::gameModes()
{
	QList<GameMode> gameModesList;
	gameModesList << GameMode::ffaGame(MODE_ROKMATCH, tr("Rok Match"));
	gameModesList << GameMode::teamGame(MODE_TEAMROKMATCH, tr("Team Rok Match"));
	gameModesList << GameMode::ffaGame(MODE_FRAGTAG, tr("Frag Tag"));
	gameModesList << GameMode::ffaGame(MODE_LASTTUROKSTANDING, tr("Last Turok Standing"));
	gameModesList << GameMode::ffaGame(MODE_ROKMIX, tr("Rok Mix"));
	gameModesList << GameMode::ffaGame(MODE_INSTAROK, tr("Insta Rok"));
	gameModesList << GameMode::ffaGame(MODE_RAPTORFEST, tr("Raptor Fest"));
	return gameModesList;
}

QList<GameCVar> Turok2ExGameInfo::limits(const GameMode &gameMode)
{
	Q_UNUSED(gameMode);
	QList<GameCVar> limits;
	limits << GameCVar(tr("Frag limit"), "+sv_fraglimit", 20);
	limits << GameCVar(tr("Time limit (in seconds)"), "+sv_timelimit", 600);
	limits << GameCVar(tr("Point limit"), "+sv_pointlimit", 3);
	limits << GameCVar(tr("Force team balance"), "+sv_forcebalanceteams", 1);
	limits << GameCVar(tr("PFM overlap threshold"), "+sv_pfm_maxoverlap", 4);
	return limits;
}

QString Turok2AboutProvider::provide()
{
	QString aboutDescription = tr("Turok 2 EX - Doomseeker plugin\n\n");
	aboutDescription += tr(
		"This plugin is distributed under the terms of the LGPL v2.1 or later.\n\n");
	aboutDescription += tr(
		"CRC code on BSD License:\n");
	aboutDescription += tr(
		"Copyright (c) 2004-2006 Intel Corporation. "
		"All rights reserved.\n\n"

		"Redistribution and use in source and binary forms, with or without "
		"modification, are permitted provided that the following conditions are met:\n\n"

		"1. Redistributions of source code must retain the above copyright notice, "
		"this list of conditions and the following disclaimer.\n"
		"2. Redistributions in binary form must reproduce the above copyright "
		"notice, this list of conditions and the following disclaimer in the "
		"documentation and/or other materials provided with the distribution.\n\n"

		"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" "
		"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE "
		"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE "
		"ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE "
		"LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR "
		"CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF "
		"SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS "
		"INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN "
		"CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) "
		"ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE "
		"POSSIBILITY OF SUCH DAMAGE.");
	return aboutDescription;
}
