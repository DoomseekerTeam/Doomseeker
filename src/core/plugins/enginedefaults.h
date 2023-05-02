//------------------------------------------------------------------------------
// enginedefaults.h
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef ide7dfca98_374f_4972_bff9_0b872f8133c6
#define ide7dfca98_374f_4972_bff9_0b872f8133c6

#include "serverapi/serverstructs.h"

class DefaultDifficultyProvider : public GameCVarProvider
{
	Q_OBJECT

public:
	QList<GameCVar> get(const QVariant &context) override
	{
		Q_UNUSED(context);
		QList<GameCVar> list;
		list << GameCVar(tr("1 - I'm too young to die"), "", 0);
		list << GameCVar(tr("2 - Hey, not too rough"), "", 1);
		list << GameCVar(tr("3 - Hurt me plenty"), "", 2);
		list << GameCVar(tr("4 - Ultra-violence"), "", 3);
		list << GameCVar(tr("5 - NIGHTMARE!"), "", 4);
		return list;
	}
};

#endif
