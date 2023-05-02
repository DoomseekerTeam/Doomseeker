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

#ifndef __SERVERSSTATUSWIDGET_H__
#define __SERVERSSTATUSWIDGET_H__

#include "serverapi/serverptr.h"
#include <QLabel>

class EnginePlugin;
class MasterClient;
class Server;
class ServerList;
class ServerListCountTracker;
struct ServerListCount;

class ServersStatusWidget : public QLabel
{
	Q_OBJECT

public:
	ServersStatusWidget(const EnginePlugin *plugin, const ServerList *serverList);

	const ServerListCount &count() const;

public slots:
	/**
	 * @brief Changes the appearance of the widget basing on the boolean
	 * value.
	 *
	 * If set to true, appearance will be clear. If set to false, appearance
	 * will be "grayed out".
	 */
	void setMasterEnabledStatus(bool bEnabled);
	void updateDisplay();

signals:
	void clicked(const EnginePlugin *plugin);
	void counterUpdated();

private:
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	void registerServer(ServerPtr server);
	QString refreshedPercentAsText() const;

	bool enabled;
	QPixmap icon;
	QPixmap iconDisabled;
	ServerListCountTracker *countTracker;
	const EnginePlugin *plugin;
	const ServerList *serverList;
};

#endif /* __SERVERSSTATUSWIDGET_H__ */
