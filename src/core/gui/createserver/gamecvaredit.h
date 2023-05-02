//------------------------------------------------------------------------------
// gamecvaredit.h
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
// Copyright (C) 2021 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_GUI_CREATESERVER_GAMECVAREDIT_H
#define DOOMSEEKER_GUI_CREATESERVER_GAMECVAREDIT_H

#include "dptr.h"

#include <QWidget>
#include <QVariant>

class GameCVar;

class GameCVarEdit : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(GameCVarEdit);

public:
	GameCVarEdit(GameCVar cvar, QWidget *parent);

	/// The set cvar with the current value().
	GameCVar cvar() const;
	/// Return true if the cvar needs to be labelled by a separate label.
	bool externalLabel() const;

	void setValue(const QVariant &value);
	QVariant value() const;

private:
	DPtr<GameCVarEdit> d;
};

#endif
