//------------------------------------------------------------------------------
// Turok2Exgamerunner.h
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
#ifndef __TUROK2EX_GAME_RUNNER_H_
#define __TUROK2EX_GAME_RUNNER_H_

#include "serverapi/gameclientrunner.h"

class Turok2ExServer;

class Turok2ExGameClientRunner : public GameClientRunner
{
public:
	Turok2ExGameClientRunner(QSharedPointer<Turok2ExServer> server);

private:
	QSharedPointer<Turok2ExServer> server;
	void addFiles(const QStringList &files);
	void addIwad();
	void addConnectCommand();
};

#endif
