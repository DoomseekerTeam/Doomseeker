//------------------------------------------------------------------------------
// tooltipgenerator.cpp
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
#include "tooltipgenerator.h"

#include "serverapi/server.h"
#include "serverapi/tooltips/dmflagshtmlgenerator.h"
#include "serverapi/tooltips/gameinfotip.h"
#include "serverapi/tooltips/generalinfotip.h"
#include "serverapi/tooltips/playertable.h"

DClass<TooltipGenerator>
{
public:
	ServerCPtr server;
};

DPointered(TooltipGenerator)

TooltipGenerator::TooltipGenerator(const ServerCPtr &server)
{
	d->server = server;
}

TooltipGenerator::~TooltipGenerator()
{
}

QString TooltipGenerator::dmflagsHTML()
{
	DmflagsHtmlGenerator generator(server());
	return generator.generate();
}

QString TooltipGenerator::gameInfoTableHTML()
{
	GameInfoTip tip(server());
	return tip.generateHTML();
}

QString TooltipGenerator::generalInfoHTML()
{
	GeneralInfoTip tip(server());
	return tip.generateHTML();
}

QString TooltipGenerator::playerTableHTML()
{
	PlayerTable table(server());
	return table.generateHTML();
}

ServerCPtr TooltipGenerator::server() const
{
	return d->server;
}
