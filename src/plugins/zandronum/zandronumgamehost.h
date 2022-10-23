//------------------------------------------------------------------------------
// zandronumgamehost.h
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
#ifndef id7CCF5C41_0664_487D_926B2383C451D847
#define id7CCF5C41_0664_487D_926B2383C451D847

#include "zandronumgameinfo.h"
#include <serverapi/gamehost.h>

class ZandronumGameHost : public GameHost
{
	Q_OBJECT

public:
	ZandronumGameHost();

protected:
	void addExtra() override;

private:
	Q_DISABLE_COPY(ZandronumGameHost)

	void addDMFlags();
	void addHostModeParams();

	QString strArg(const QString &val);
	ZandronumGameInfo::GameVersion gameVersion() const;
};

#endif // header
