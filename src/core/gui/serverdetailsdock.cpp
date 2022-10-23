//------------------------------------------------------------------------------
// serverdetailsdock.cpp
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
// Copyright (C) 2014 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#include <QAction>
#include <QBoxLayout>

#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "serverapi/serverptr.h"
#include "serverapi/serverstructs.h"
#include "serverapi/tooltips/tooltipgenerator.h"
#include "serverdetailsdock.h"
#include "ui_serverdetailsdock.h"

DClass<ServerDetailsDock> : public Ui::ServerDetailsDock
{
};

DPointered(ServerDetailsDock)

ServerDetailsDock::ServerDetailsDock(QWidget *parent) : QDockWidget(parent)
{
	d->setupUi(this);
	this->toggleViewAction()->setIcon(QIcon(":/icons/server_details.png"));

	clear();
}

ServerDetailsDock::~ServerDetailsDock()
{
}

void ServerDetailsDock::clear()
{
	d->lblServer->setText("");
	d->sbArea->setText("");
	d->detailsArea->setText("");
}

void ServerDetailsDock::displaySelection(QList<ServerPtr> &selectedServers)
{
	ServerPtr server = selectServer(selectedServers);
	if (server == nullptr)
	{
		clear();
		return;
	}

	d->lblServer->setText(server->name());
	TooltipGenerator *tooltipGenerator = server->tooltipGenerator();
	if (server->players().numClients() != 0)
		d->sbArea->setText(tooltipGenerator->playerTableHTML());
	else
		d->sbArea->setText(QString());
	d->detailsArea->setText(QString("<div>%1</div>%2").arg(server->customDetails()).arg(tooltipGenerator->dmflagsHTML()));
	delete tooltipGenerator;
}

void ServerDetailsDock::reorientContentsBasingOnDimensions()
{
	const int TOPTOBOTTOM_PREFERENCE_THRESHOLD = 100;
	if (height() + TOPTOBOTTOM_PREFERENCE_THRESHOLD > width())
		static_cast<QBoxLayout *>(d->dataLayout)->setDirection(QBoxLayout::TopToBottom);
	else
		static_cast<QBoxLayout *>(d->dataLayout)->setDirection(QBoxLayout::LeftToRight);
}

void ServerDetailsDock::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	reorientContentsBasingOnDimensions();
}

ServerPtr ServerDetailsDock::selectServer(QList<ServerPtr> &selectedServers)
{
	if (selectedServers.count() == 0)
		return ServerPtr();
	if (!selectedServers[0]->isKnown())
		return ServerPtr();
	return selectedServers[0];
}
