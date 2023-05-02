//------------------------------------------------------------------------------
// serverfilterbuildermenu.cpp
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
#include "serverfilterbuildermenu.h"

#include "gui/entity/serverlistfilterinfo.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"

DClass<ServerFilterBuilderMenu>
{
public:
	ServerListFilterInfo filter;
	QString gameMode;
	unsigned maxPing;

	static void addIfNotContains(QStringList &target, const QString &candidate)
	{
		if (!target.contains(candidate, Qt::CaseInsensitive))
			target << candidate;
	}
};

DPointered(ServerFilterBuilderMenu)

ServerFilterBuilderMenu::ServerFilterBuilderMenu(const Server &server,
	const ServerListFilterInfo &filter, QWidget *parent)
	: QMenu(tr("Build server filter ..."), parent)
{
	d->filter = filter;
	d->gameMode = server.gameMode().name();
	d->maxPing = server.ping();

	addAction(this, tr("Show only servers with ping lower than %1").arg(d->maxPing),
		SLOT(applyPingFilter()));
	if (!d->filter.gameModes.contains(d->gameMode, Qt::CaseInsensitive))
	{
		addAction(this, tr("Filter by game mode \"%1\"").arg(d->gameMode),
			SLOT(applyGameModeFilter()));
	}
	if (!d->filter.gameModesExcluded.contains(d->gameMode, Qt::CaseInsensitive))
	{
		addAction(this, tr("Hide game mode \"%1\"").arg(d->gameMode),
			SLOT(applyGameModeExcludedFilter()));
	}

	QMenu *includeWads = new QMenu(tr("Include WAD ..."), this);
	QMenu *excludeWads = new QMenu(tr("Exclude WAD ..."), this);

	QStringList wads = server.allWadNames();
	for (const QString &wad : wads)
	{
		if (!d->filter.wadsExcluded.contains(wad, Qt::CaseInsensitive))
			mkExcludeWadAction(excludeWads, wad);
		if (!d->filter.wads.contains(wad, Qt::CaseInsensitive))
			mkIncludeWadAction(includeWads, wad);
	}

	if (!excludeWads->isEmpty())
		addMenu(excludeWads);
	if (!includeWads->isEmpty())
		addMenu(includeWads);
}

ServerFilterBuilderMenu::~ServerFilterBuilderMenu()
{
}

QAction *ServerFilterBuilderMenu::addAction(QMenu *menu, const QString &text, const char *slot)
{
	auto action = new QAction(menu);
	action->setText(text);
	this->connect(action, SIGNAL(triggered()), slot);
	menu->addAction(action);
	return action;
}

void ServerFilterBuilderMenu::applyGameModeExcludedFilter()
{
	if (!d->filter.gameModesExcluded.contains(d->gameMode, Qt::CaseInsensitive))
		d->filter.gameModesExcluded << d->gameMode;
}

void ServerFilterBuilderMenu::applyGameModeFilter()
{
	if (!d->filter.gameModes.contains(d->gameMode, Qt::CaseInsensitive))
		d->filter.gameModes << d->gameMode;
}

void ServerFilterBuilderMenu::applyPingFilter()
{
	d->filter.maxPing = d->maxPing;
}

void ServerFilterBuilderMenu::excludeWadFromAction()
{
	auto action = static_cast<QAction *>(sender());
	PrivData<ServerFilterBuilderMenu>::addIfNotContains(d->filter.wadsExcluded, action->text());
}

const ServerListFilterInfo &ServerFilterBuilderMenu::filter() const
{
	return d->filter;
}

void ServerFilterBuilderMenu::includeWadFromAction()
{
	auto action = static_cast<QAction *>(sender());
	PrivData<ServerFilterBuilderMenu>::addIfNotContains(d->filter.wads, action->text());
}

QAction *ServerFilterBuilderMenu::mkExcludeWadAction(QMenu *menu, const QString &wadName)
{
	return addAction(menu, wadName, SLOT(excludeWadFromAction()));
}

QAction *ServerFilterBuilderMenu::mkIncludeWadAction(QMenu *menu, const QString &wadName)
{
	return addAction(menu, wadName, SLOT(includeWadFromAction()));
}
