//------------------------------------------------------------------------------
// votingsetupwidget.h
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
#ifndef id3E880CB7_0FF0_4187_BAD90AD0EA16A263
#define id3E880CB7_0FF0_4187_BAD90AD0EA16A263

#include "ui_votingsetupwidget.h"
#include <QStringList>
#include <QWidget>

class Ini;

class VotingSetupWidget : public QWidget, private Ui::VotingSetupWidget
{
	Q_OBJECT

public:
	VotingSetupWidget(QWidget *parent);

	QStringList generateGameRunParameters();
	bool loadConfig(Ini &ini);
	bool saveConfig(Ini &ini);

private:
	/**
	 * @brief Internal enums cast directly to game's CCMD values.
	 */
	enum WhoCanVote
	{
		AllCanVote = 0,
		NoneCanVote = 1,
		SpectatorsCantVote = 2
	};

	QStringList gameParametersList() const;

	void setWhoCanVote(WhoCanVote who);
	WhoCanVote whoCanVote() const;
};

#endif
