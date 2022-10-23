//------------------------------------------------------------------------------
// dmflagshtmlgenerator.cpp
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
#include "dmflagshtmlgenerator.h"

#include "serverapi/server.h"
#include "serverapi/serverstructs.h"

DClass<DmflagsHtmlGenerator>
{
public:
	ServerCPtr server;

	QString mkSectionContents(const DMFlagsSection &section)
	{
		QString result;
		for (int i = 0; i < section.count(); ++i)
		{
			result += "<li>" + section[i].name() + "</li>";
		}
		return result;
	}
};

DPointered(DmflagsHtmlGenerator)

DmflagsHtmlGenerator::DmflagsHtmlGenerator(const ServerCPtr &server)
{
	d->server = server;
}

DmflagsHtmlGenerator::~DmflagsHtmlGenerator()
{
}

QString DmflagsHtmlGenerator::generate()
{
	QString result;
	const QList<DMFlagsSection> sections = d->server->dmFlags();
	for (const DMFlagsSection &section : sections)
	{
		if (!section.isEmpty())
		{
			// Hack (#3944): the dmflag bits are stored in Doomseeker in the
			// 32-bit unsigned format, but ZDoom-based ports use signed. If port
			// uses the most significant bit for a dmflag the combined values
			// will go into negative numbers.
			const int32_t numerical = static_cast<int32_t>(section.combineValues());

			result += QString("<li><b>%1 (%2):</b></li>").arg(section.name()).arg(numerical);
			result += "<ul>";
			result += d->mkSectionContents(section);
			result += "</ul>";
		}
	}
	if (!result.isEmpty())
	{
		result = "<ul>" + result + "</ul>";
	}
	return result;
}
