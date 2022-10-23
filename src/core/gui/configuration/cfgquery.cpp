//------------------------------------------------------------------------------
// cfgquery.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "cfgquery.h"
#include "configuration/doomseekerconfig.h"
#include "configuration/queryspeed.h"
#include "ui_cfgquery.h"

DClass<CFGQuery> : public Ui::CFGQuery
{
public:
	void setQuerySpeed(const QuerySpeed &speed)
	{
		triesBox->setValue(speed.attemptsPerServer);
		timeoutBox->setValue(speed.delayBetweenSingleServerAttempts);
		queryIntervalBox->setValue(speed.intervalBetweenServers);
	}

	QuerySpeed querySpeed() const
	{
		QuerySpeed speed;
		speed.attemptsPerServer = triesBox->value();
		speed.delayBetweenSingleServerAttempts = timeoutBox->value();
		speed.intervalBetweenServers = queryIntervalBox->value();
		return speed;
	}
};

DPointered(CFGQuery)

CFGQuery::CFGQuery(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);

	const QuerySpeed &min = QuerySpeed::MAX_SPEED;
	d->triesBox->setMinimum(min.attemptsPerServer);
	d->timeoutBox->setMinimum(min.delayBetweenSingleServerAttempts);
	d->queryIntervalBox->setMinimum(min.intervalBetweenServers);

	d->setQuerySpeed(QuerySpeed::aggressive());
}

CFGQuery::~CFGQuery()
{
}

void CFGQuery::readSettings()
{
	d->queryBeforeLaunch->setChecked(gConfig.doomseeker.bQueryBeforeLaunch);
	d->queryOnStartup->setChecked(gConfig.doomseeker.bQueryOnStartup);
	d->grbServerAutoRefresh->setChecked(gConfig.doomseeker.bQueryAutoRefreshEnabled);
	d->numAutoRefreshEverySeconds->setValue(gConfig.doomseeker.queryAutoRefreshEverySeconds);
	d->cbDontRefreshIfActive->setChecked(gConfig.doomseeker.bQueryAutoRefreshDontIfActive);
	d->setQuerySpeed(gConfig.doomseeker.querySpeed());
}

void CFGQuery::saveSettings()
{
	gConfig.doomseeker.bQueryBeforeLaunch = d->queryBeforeLaunch->isChecked();
	gConfig.doomseeker.bQueryOnStartup = d->queryOnStartup->isChecked();
	gConfig.doomseeker.bQueryAutoRefreshEnabled = d->grbServerAutoRefresh->isChecked();
	gConfig.doomseeker.queryAutoRefreshEverySeconds = d->numAutoRefreshEverySeconds->value();
	gConfig.doomseeker.bQueryAutoRefreshDontIfActive = d->cbDontRefreshIfActive->isChecked();
	gConfig.doomseeker.setQuerySpeed(d->querySpeed());
}

void CFGQuery::setCautiousQueryPreset()
{
	d->setQuerySpeed(QuerySpeed::cautious());
}

void CFGQuery::setModerateQueryPreset()
{
	d->setQuerySpeed(QuerySpeed::moderate());
}

void CFGQuery::setAggressiveQueryPreset()
{
	d->setQuerySpeed(QuerySpeed::aggressive());
}

void CFGQuery::setVeryAggressiveQueryPreset()
{
	d->setQuerySpeed(QuerySpeed::veryAggressive());
}
