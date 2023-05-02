//------------------------------------------------------------------------------
// zandronumqserverdmflagsparser.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "zandronumqserverdmflagsparser.h"

#include "zandronumq1dmflags.h"
#include "zandronumqgameinfo.h"
#include "zandronumqserver.h"
#include <datastreamoperatorwrapper.h>
#include <serverapi/serverstructs.h>

ZandronumqServerDmflagsParser *ZandronumqServerDmflagsParser::mkParser(
	ZandronumqServer *server, QDataStream *in)
{
	return new ZandronumqServer1DmflagsParser(server, in);
}

ZandronumqServerDmflagsParser::ZandronumqServerDmflagsParser(
	ZandronumqServer *server, QDataStream *in)
{
	this->server = server;
	this->inStream = in;
}

QList<DMFlagsSection> ZandronumqServerDmflagsParser::sequential32Parse(const QList<DMFlagsSection> &knownFlags)
{
	DataStreamOperatorWrapper in(inStream);
	int numSections = in.readQInt8();

	QList<DMFlagsSection> result;
	// Read each dmflags section separately.
	for (int i = 0; i < knownFlags.count() && i < numSections; ++i)
	{
		quint32 dmflags = in.readQUInt32();

		const DMFlagsSection &knownSection = knownFlags[i];
		DMFlagsSection detectedSection = knownSection.copyEmpty();

		// Iterate through every known flag to check whether it should be
		// inserted into the structure of this server.
		for (int j = 0; j < knownSection.count(); ++j)
		{
			if ((dmflags & knownSection[j].value()) != 0)
			{
				detectedSection << knownSection[j];
			}
		}

		result << detectedSection;
	}
	if (knownFlags.count() < numSections)
	{
		// Skip all sections we didn't read.
		int remaining = numSections - knownFlags.count();
		in.skipRawData(remaining * 4);
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////

QList<DMFlagsSection> ZandronumqServerNullParser::parse()
{
	DataStreamOperatorWrapper in(inStream);
	int numSections = in.readQInt8();
	in.skipRawData(numSections * 4);
	return QList<DMFlagsSection>();
}

///////////////////////////////////////////////////////////////////////////////

QList<DMFlagsSection> ZandronumqServer1DmflagsParser::parse()
{
	return sequential32Parse(zandronumq1::Dmflags().flags());
}
