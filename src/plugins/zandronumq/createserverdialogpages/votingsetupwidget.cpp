//------------------------------------------------------------------------------
// votingsetupwidget.cpp
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
#include "votingsetupwidget.h"

#include <ini/ini.h>
#include <ini/inisection.h>

VotingSetupWidget::VotingSetupWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	mainPanel->setEnabled(false);
}

QStringList VotingSetupWidget::generateGameRunParameters()
{
	if (cbUseThisPage->isChecked())
	{
		return gameParametersList();
	}
	else
	{
		return QStringList();
	}
}

QStringList VotingSetupWidget::gameParametersList() const
{
	QStringList params;

	params << "+sv_limitnumvotes" << (cbVoteFloodingProtectionEnabled->isChecked() ? "1" : "0");
	params << "+sv_minvoters" << QString::number(spinMinimumNumberOfPlayersRequiredToCallAVote->value());
	params << "+sv_nocallvote" << QString::number(static_cast<int>(whoCanVote()));

	params << "+sv_nochangemapvote" << (!cbChangeMapVote->isChecked() ? "1" : "0");
	params << "+sv_nonextmapvote" << (!cbNextMapVote->isChecked() ? "1" : "0");
	params << "+sv_noduellimitvote" << (!cbDuelLimitVote->isChecked() ? "1" : "0");
	params << "+sv_nofraglimitvote" << (!cbFragLimitVote->isChecked() ? "1" : "0");
	params << "+sv_nokickvote" << (!cbKickVote->isChecked() ? "1" : "0");
	params << "+sv_nomapvote" << (!cbMapVote->isChecked() ? "1" : "0");
	params << "+sv_nopointlimitvote" << (!cbPointLimitVote->isChecked() ? "1" : "0");
	params << "+sv_notimelimitvote" << (!cbTimeLimitVote->isChecked() ? "1" : "0");
	params << "+sv_nowinlimitvote" << (!cbWinLimitVote->isChecked() ? "1" : "0");
	params << "+sv_noforcespecvote" << (!cbForceSpectatorVote->isChecked() ? "1" : "0");

	return params;
}

bool VotingSetupWidget::loadConfig(Ini &ini)
{
	IniSection section = ini.section("voting");

	cbUseThisPage->setChecked(section.value("UseThisPage").toBool());

	setWhoCanVote(static_cast<WhoCanVote>(section.value("WhoCanVote").toInt()));
	spinMinimumNumberOfPlayersRequiredToCallAVote->setValue(section.value("MinimumPlayersRequiredToVote", 1).toInt());
	cbVoteFloodingProtectionEnabled->setChecked(section.value("VoteFloodingProtection", true).toBool());

	cbKickVote->setChecked(section.value("KickVote", true).toBool());
	cbChangeMapVote->setChecked(section.value("ChangeMapVote", true).toBool());
	cbNextMapVote->setChecked(section.value("NextMapVote", true).toBool());
	cbMapVote->setChecked(section.value("MapVote", true).toBool());
	cbTimeLimitVote->setChecked(section.value("TimeLimitVote", true).toBool());
	cbFragLimitVote->setChecked(section.value("FragLimitVote", true).toBool());
	cbDuelLimitVote->setChecked(section.value("DuelLimitVote", true).toBool());
	cbPointLimitVote->setChecked(section.value("PointLimitVote", true).toBool());
	cbWinLimitVote->setChecked(section.value("WinLimitVote", true).toBool());
	cbForceSpectatorVote->setChecked(section.value("ForceSpectatorVote", true).toBool());

	return true;
}

bool VotingSetupWidget::saveConfig(Ini &ini)
{
	IniSection section = ini.section("voting");

	section["UseThisPage"] = cbUseThisPage->isChecked();

	section["WhoCanVote"] = whoCanVote();
	section["MinimumPlayersRequiredToVote"] = spinMinimumNumberOfPlayersRequiredToCallAVote->value();
	section["VoteFloodingProtection"] = cbVoteFloodingProtectionEnabled->isChecked();

	section["KickVote"] = cbKickVote->isChecked();
	section["NextMapVote"] = cbNextMapVote->isChecked();
	section["MapVote"] = cbMapVote->isChecked();
	section["TimeLimitVote"] = cbTimeLimitVote->isChecked();
	section["FragLimitVote"] = cbFragLimitVote->isChecked();
	section["DuelLimitVote"] = cbDuelLimitVote->isChecked();
	section["PointLimitVote"] = cbPointLimitVote->isChecked();
	section["WinLimitVote"] = cbWinLimitVote->isChecked();
	section["ForceSpectatorVote"] = cbForceSpectatorVote->isChecked();

	return true;
}

void VotingSetupWidget::setWhoCanVote(WhoCanVote who)
{
	switch (who)
	{
	case NoneCanVote:
		rbNoOneCanVote->setChecked(true);
		break;
	case SpectatorsCantVote:
		rbSpectatorsCantVote->setChecked(true);
		break;
	case AllCanVote:
	default:
		rbAllCanVote->setChecked(true);
		break;
	}
}

VotingSetupWidget::WhoCanVote VotingSetupWidget::whoCanVote() const
{
	if (rbNoOneCanVote->isChecked())
	{
		return NoneCanVote;
	}
	else if (rbSpectatorsCantVote->isChecked())
	{
		return SpectatorsCantVote;
	}
	else
	{
		return AllCanVote;
	}
}
