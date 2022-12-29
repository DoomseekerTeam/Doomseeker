//------------------------------------------------------------------------------
// respondercfg.cpp
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
#include "respondercfg.h"

unsigned ResponderCfg::masterBannedEvery()
{
	return 0;
}

unsigned short ResponderCfg::masterServerPort()
{
	return 32111;
}

unsigned short ResponderCfg::maxPacketSize()
{
	return 1024;
}

unsigned short ResponderCfg::serverPortBase()
{
	return 32112;
}

unsigned short ResponderCfg::numBannedServers()
{
	return 2;
}

unsigned short ResponderCfg::numNotRespondingServers()
{
	return 5;
}

unsigned short ResponderCfg::numTooSoonServers()
{
	return 2;
}

unsigned short ResponderCfg::numValidServers()
{
	return 100;
}
