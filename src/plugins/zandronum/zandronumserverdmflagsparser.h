//------------------------------------------------------------------------------
// zandronumserverdmflagsparser.h
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
class ZandronumServer;

/**
 * @brief SQF_ALL_DMFLAGS parser family.
 */
class ZandronumServerDmflagsParser
{
public:
	static ZandronumServerDmflagsParser *mkParser(ZandronumServer * server, QDataStream *in);

	ZandronumServerDmflagsParser(ZandronumServer *server, QDataStream *in);
	virtual ~ZandronumServerDmflagsParser()
	{}

	virtual QList<DMFlagsSection> parse() = 0;

protected:
	QDataStream *inStream;
	ZandronumServer *server;

	QList<DMFlagsSection> sequential32Parse(const QList<DMFlagsSection> &knownFlags);
};

class ZandronumServerNullParser : public ZandronumServerDmflagsParser
{
public:
	ZandronumServerNullParser(ZandronumServer *server, QDataStream *in)
		: ZandronumServerDmflagsParser(server, in)
	{}

	QList<DMFlagsSection> parse() override;
};

class ZandronumServer2DmflagsParser : public ZandronumServerDmflagsParser
{
public:
	ZandronumServer2DmflagsParser(ZandronumServer *server, QDataStream *in)
		: ZandronumServerDmflagsParser(server, in)
	{}

	QList<DMFlagsSection> parse() override;
};

class ZandronumServer3DmflagsParser : public ZandronumServerDmflagsParser
{
public:
	ZandronumServer3DmflagsParser(ZandronumServer *server, QDataStream *in)
		: ZandronumServerDmflagsParser(server, in)
	{}

	QList<DMFlagsSection> parse() override;
};

#endif
