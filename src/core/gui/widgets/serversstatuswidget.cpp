//------------------------------------------------------------------------------
// serversstatuswidget.h
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

#include <cmath>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

#include "gui/serverlist.h"
#include "plugins/engineplugin.h"
#include "serverapi/masterclient.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "serverapi/serverlistcounttracker.h"
#include "serversstatuswidget.h"

ServersStatusWidget::ServersStatusWidget(const EnginePlugin *plugin, const ServerList *serverList)
	: QLabel(), enabled(false), icon(plugin->icon())
{
	this->plugin = plugin;

	this->countTracker = new ServerListCountTracker(this);
	this->countTracker->setPluginFilter(plugin);
	this->serverList = serverList;

	countTracker->connect(serverList, SIGNAL(serverRegistered(ServerPtr)),
		SLOT(registerServer(ServerPtr)));
	countTracker->connect(serverList, SIGNAL(serverDeregistered(ServerPtr)),
		SLOT(deregisterServer(ServerPtr)));
	this->connect(countTracker, SIGNAL(updated()), SLOT(updateDisplay()));
	this->connect(countTracker, SIGNAL(updated()), SIGNAL(counterUpdated()));

	// Transform icon to grayscale format for disabled appearance
	QImage iconImage = icon.toImage();
	int width = iconImage.width();
	int height = iconImage.height();
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			QRgb pixel = iconImage.pixel(x, y);
			int alpha = qAlpha(pixel);
			int gray = qGray(pixel);

			pixel = qRgba(gray, gray, gray, alpha);

			iconImage.setPixel(x, y, pixel);
		}
	}

	iconDisabled = QPixmap::fromImage(iconImage);

	// Have an inset frame unless we're on the Mac
	#ifndef Q_OS_DARWIN
	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Sunken);
	#else
	setFrameShape(QFrame::NoFrame);
	#endif

	setFixedHeight(22);
	setToolTip(tr("Players (Humans + Bots) / Servers Refreshed%"));

	setIndent(22);
	updateDisplay();
}

const ServerListCount &ServersStatusWidget::count() const
{
	return countTracker->count();
}

void ServersStatusWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		emit clicked(plugin);
}

void ServersStatusWidget::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	p.drawPixmap(2, 2, 18, 18, enabled ? icon : iconDisabled);
	p.end();

	QLabel::paintEvent(event);
}

QString ServersStatusWidget::refreshedPercentAsText() const
{
	const ServerListCount &count = countTracker->count();
	if (count.numServers == 0)
		return tr("N/A");
	else
		return tr("%1%").arg(count.refreshedPercent());
}

void ServersStatusWidget::setMasterEnabledStatus(bool bEnabled)
{
	this->enabled = bEnabled;
	updateDisplay();
}

void ServersStatusWidget::updateDisplay()
{
	if (enabled)
	{
		const ServerListCount &count = countTracker->count();
		QString text = tr("%1 (%2+%3) / %4").arg(count.numPlayers).arg(count.numHumanPlayers)
			.arg(count.numBots).arg(count.numServers);
		if (count.numRefreshing > 0)
			text += tr(" %1").arg(refreshedPercentAsText());
		setText(text);
	}
	else
		setText(tr("N/A"));
}
