//------------------------------------------------------------------------------
// mastermanager.h
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
#ifndef __MASTERMANAGER_H__
#define __MASTERMANAGER_H__

#include "serverapi/masterclient.h"
#include "serverapi/message.h"
#include <QSharedPointer>

class BroadcastManager;
class CustomServers;
class MasterClientSignalProxy;

/**
 * @brief Manager class for a number of MasterClient instances.
 *
 * MasterManager will emit listUpdated() signal once the last MasterClient
 * that is on the mastersBeingRefreshed list emits its own listUpdate() signal.
 */
class MasterManager : public MasterClient
{
	Q_OBJECT

public:
	MasterManager();
	~MasterManager();

	void addMaster(MasterClient *master);
	QList<ServerPtr> allServers() const;
	CustomServers *customServs() { return customServers; }

	int numMasters() const { return masters.size(); }
	void setBroadcastManager(BroadcastManager *broadcastManagerPtr);
	const EnginePlugin *plugin() const { return nullptr; }

	MasterClient *operator[](int index) { return masters[index]; }

public slots:
	void refreshStarts();

signals:
	/**
	 * @brief Emitted for every MasterClient that emits listUpdated()
	 * signal.
	 */
	void listUpdatedForMaster(MasterClient *pSender);
	void masterMessage(MasterClient *pSender, const QString &title, const QString &content, bool isError);
	void masterMessageImportant(MasterClient *pSender, const Message &objMessage);

private:
	CustomServers *customServers;
	QList<MasterClient *> masters;
	QSet<MasterClient *> mastersBeingRefreshed;

	BroadcastManager *broadcastManager;

	QByteArray createServerListRequest() { return QByteArray(); }
	Response readMasterResponse(const QByteArray &data);
	void timeoutRefreshEx();

private slots:
	void masterListUpdated();

	void forwardMasterMessage(const QString &title, const QString &content, bool isError)
	{
		MasterClient *master = static_cast<MasterClient *>(sender());
		emit masterMessage(master, title, content, isError);
	}

	void forwardMasterMessageImportant(const Message &message)
	{
		MasterClient *master = static_cast<MasterClient *>(sender());
		emit masterMessageImportant(master, message);
	}
};

#endif
