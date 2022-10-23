//------------------------------------------------------------------------------
// zandronumqserverdmflagsparser.h
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
#ifndef id86E9EA04_C69C_43C1_877B45D653F448FA
#define id86E9EA04_C69C_43C1_877B45D653F448FA

#include <QDataStream>
#include <QList>

class DMFlagsSection;
class ZandronumqServer;

/**
 * @brief SQF_ALL_DMFLAGS parser family.
 */
class ZandronumqServerDmflagsParser
{
public:
	static ZandronumqServerDmflagsParser *mkParser(ZandronumqServer * server, QDataStream *in);

	ZandronumqServerDmflagsParser(ZandronumqServer *server, QDataStream *in);
	virtual ~ZandronumqServerDmflagsParser()
	{}

	virtual QList<DMFlagsSection> parse() = 0;

protected:
	QDataStream *inStream;
	ZandronumqServer *server;

	QList<DMFlagsSection> sequential32Parse(const QList<DMFlagsSection> &knownFlags);
};

class ZandronumqServerNullParser : public ZandronumqServerDmflagsParser
{
public:
	ZandronumqServerNullParser(ZandronumqServer *server, QDataStream *in)
		: ZandronumqServerDmflagsParser(server, in)
	{}

	QList<DMFlagsSection> parse() override;
};

class ZandronumqServer1DmflagsParser : public ZandronumqServerDmflagsParser
{
public:
	ZandronumqServer1DmflagsParser(ZandronumqServer *server, QDataStream *in)
		: ZandronumqServerDmflagsParser(server, in)
	{}

	QList<DMFlagsSection> parse() override;
};

#endif
