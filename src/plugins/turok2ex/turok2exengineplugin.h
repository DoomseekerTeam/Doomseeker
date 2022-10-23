//------------------------------------------------------------------------------
// Turok2Exengineplugin.h
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
#ifndef __TUROK2EX_ENGINEPLUGIN_H_
#define __TUROK2EX_ENGINEPLUGIN_H_

#include "plugins/engineplugin.h"

class Turok2ExEnginePlugin : public EnginePlugin
{
	DECLARE_PLUGIN(Turok2ExEnginePlugin)
public:
	Turok2ExEnginePlugin();

	GameHost *gameHost() override;
	QList<GameMode> gameModes() const override;
	QList<GameCVar> limits(const GameMode &mode) const override;
	ServerPtr mkServer(const QHostAddress &address, unsigned short port) const override;
};

#endif
