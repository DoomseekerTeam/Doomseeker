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
#include "serverdetailsdock.h"
#include "ui_serverdetailsdock.h"

#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "serverapi/serverptr.h"
#include "serverapi/serverstructs.h"
#include "serverapi/tooltips/servertooltip.h"
#include "serverapi/tooltips/tooltipgenerator.h"

#include <QAction>
#include <QScopedPointer>

DClass<ServerDetailsDock> : public Ui::ServerDetailsDock
{
public:
	QString bold(const QString &inner) const
	{
		return QString("<b>%1</b>").arg(inner);
	}

	QString div(const QString &inner) const
	{
		return QString(R"(<div style="white-space: pre;">%1</div>)")
			.arg(inner);
	}

	QString italic(const QString &inner) const
	{
		return QString("<i>%1</i>").arg(inner);
	}
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
	d->lblServer->setText(d->italic(tr("No server selected.")));
	d->generalArea->setText("");
	d->playersArea->setText("");
	d->dmflagsArea->setText("");
}

void ServerDetailsDock::displaySelection(QList<ServerPtr> &selectedServers)
{
	ServerPtr server = selectServer(selectedServers);
	if (server == nullptr)
	{
		clear();
		return;
	}

	d->lblServer->setText(server->name().toHtmlEscaped());
	QScopedPointer<TooltipGenerator> tooltipGenerator(server->tooltipGenerator());

	QString generalHtml;
	generalHtml += tooltipGenerator->generalInfoHTML();
	generalHtml += tr("<b>Address:</b> %1\n").arg(server->addressWithPort());
	const QString portInfo = ServerTooltip::createPortToolTip(server);
	if (!portInfo.isEmpty())
		generalHtml += "\n" + portInfo + "\n";
	generalHtml = d->div(generalHtml);
	generalHtml += d->div(server->customDetails());
	d->generalArea->setText(generalHtml);

	QString playersHtml;
	playersHtml += d->div(tooltipGenerator->gameInfoTableHTML());
	if (server->players().numClients() != 0)
		playersHtml += d->div(tooltipGenerator->playerTableHTML());
	else
		playersHtml += d->bold(tr("No players on this server."));

	d->playersArea->setText(playersHtml);

	QString dmflagsHtml = tooltipGenerator->dmflagsHTML();
	if (dmflagsHtml.isEmpty())
		dmflagsHtml = d->bold(tr("DMFlags unknown or no DMFlags set."));
	d->dmflagsArea->setText(dmflagsHtml);
}

ServerPtr ServerDetailsDock::selectServer(QList<ServerPtr> &selectedServers)
{
	if (selectedServers.count() == 0)
		return ServerPtr();
	if (!selectedServers[0]->isKnown())
		return ServerPtr();
	return selectedServers[0];
}
