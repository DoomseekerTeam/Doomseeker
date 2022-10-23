//------------------------------------------------------------------------------
// masterclient.cpp
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
#include "masterclient.h"

#include "datapaths.h"
#include "log.h"
#include "plugins/engineplugin.h"
#include "serverapi/message.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"

#include <QDataStream>
#include <QErrorMessage>
#include <QFile>
#include <QHostInfo>
#include <QMessageBox>
#include <QUdpSocket>

DClass<MasterClient>
{
public:
	QHostAddress address;

	bool timeouted;
	bool enabled;
	unsigned short port;
	QList<ServerPtr> servers;

	QFile *cache;

	bool isCacheOpenForReading() const
	{
		return cache != nullptr && cache->isReadable();
	}

	bool isCacheOpenForWriting() const
	{
		return cache != nullptr && cache->isWritable();
	}

	QString (MasterClient::*masterBanHelp)() const;
};

DPointered(MasterClient)

POLYMORPHIC_DEFINE_CONST(QString, MasterClient, masterBanHelp, (), ())

MasterClient::MasterClient()
{
	d->cache = nullptr;
	d->timeouted = false;
	d->enabled = true;
	d->port = 0;
	set_masterBanHelp(&MasterClient::masterBanHelp_default);
}

MasterClient::~MasterClient()
{
	emptyServerList();
	resetPacketCaching();
	if (d->cache != nullptr)
	{
		delete d->cache;
	}
}

QHostAddress MasterClient::address() const
{
	return d->address;
}

void MasterClient::clearServers()
{
	d->servers.clear();
}

bool MasterClient::isAddressSame(const QHostAddress &address, unsigned short port) const
{
	return d->address.toIPv4Address() == address.toIPv4Address() && d->port == port;
}

void MasterClient::emitBannedMessage()
{
	Message msg = Message(Message::Type::BANNED_FROM_MASTERSERVER);
	QString helpMsg = masterBanHelp();
	if (!helpMsg.trimmed().isEmpty())
	{
		msg = Message(Message::Type::CUSTOM_ERROR, tr("%1 %2").arg(
			msg.contents(), helpMsg.trimmed()));
	}
	emit message(engineName(), msg.contents(), true);
	emit messageImportant(msg);
}

void MasterClient::emptyServerList()
{
	clearServers();
}

QString MasterClient::engineName() const
{
	if (plugin() == nullptr)
	{
		return "";
	}
	return plugin()->data()->name;
}

bool MasterClient::isEnabled() const
{
	return d->enabled;
}

bool MasterClient::isTimeouted() const
{
	return d->timeouted;
}

QString MasterClient::masterBanHelp_default() const
{
	return QString();
}

void MasterClient::notifyResponse(Response response)
{
	switch (response)
	{
	default:
		break;
	case RESPONSE_BANNED:
	{
		emitBannedMessage();
		break;
	}
	case RESPONSE_WAIT:
		emit message(engineName(), tr("Could not fetch a new server list from the "
			"master because not enough time has passed."), true);
		readPacketCache();
		break;
	case RESPONSE_BAD:
		emit message(engineName(), tr("Bad response from master server."), true);
		readPacketCache();
		break;
	case RESPONSE_OLD:
		emit message(engineName(),
			tr("Could not fetch a new server list. The protocol you are using is too old. "
			"An update may be available."), true);
		break;
	}
}

int MasterClient::numServers() const
{
	return d->servers.size();
}

ServerPtr MasterClient::operator[](int index) const
{
	return d->servers[index];
}

unsigned short MasterClient::port() const
{
	return d->port;
}

bool MasterClient::preparePacketCache(bool write)
{
	if (write ? !d->isCacheOpenForWriting() : !d->isCacheOpenForReading())
	{
		if (plugin() == nullptr)
		{
			return false;
		}

		if (d->cache == nullptr)
		{
			QString cacheFile = gDefaultDataPaths->cacheLocationPath() + "/"
				+ QString(plugin()->data()->name).replace(' ', "");
			d->cache = new QFile(cacheFile);
		}
		else
		{
			d->cache->close();
		}

		if (!d->cache->open(write ? QIODevice::WriteOnly | QIODevice::Truncate : QIODevice::ReadOnly))
		{
			resetPacketCaching();
			return false;
		}
	}
	else if (!write && d->isCacheOpenForReading())
	{
		// If we prepare cache for reading we want to start
		// reading from the beginning.
		d->cache->seek(0);
	}
	return d->cache != nullptr;
}

void MasterClient::pushPacketToCache(const QByteArray &data)
{
	if (!preparePacketCache(true))
	{
		return;
	}

	QDataStream strm(d->cache);
	strm << static_cast<quint16>(data.size());
	strm << data;
}

MasterClient::Response MasterClient::readResponse(const QByteArray &data)
{
	Response response = readMasterResponse(data);
	if (response == RESPONSE_GOOD || response == RESPONSE_PENDING)
	{
		pushPacketToCache(data);
	}
	return response;
}

void MasterClient::readPacketCache()
{
	if (!preparePacketCache(false))
	{
		// Cache didn't open? Guess we just emit the signal.
		emit listUpdated();
		return;
	}

	QDataStream strm(d->cache);
	if (strm.atEnd())
	{
		// Can't read anything from cache. Either cache is empty
		// or for some reason the file cursor is set to the
		// end of the file.
		emit listUpdated();
		return;
	}

	gLog << tr("Reloading master server results from cache for %1!").arg(plugin()->data()->name);
	bool hasGood = false;
	while (!strm.atEnd())
	{
		quint16 size;
		strm >> size;

		QByteArray data(size, '\0');
		strm >> data;

		Response response = readMasterResponse(data);
		if (response == RESPONSE_GOOD)
		{
			hasGood = true;
		}
		if (response != RESPONSE_GOOD && response != RESPONSE_PENDING)
		{
			// Cache was not read properly. We need to emit the signal
			// to notify the program that this master client finished
			// updating.
			emit listUpdated();
			return;
		}
	}
	if (!hasGood)
	{
		// Plugins are ought to emit listUpdated() only when RESPONSE_GOOD
		// is achieved. If that's not the case, we shall emit it here
		// to notify refreshing process that the server has completed
		// updating.
		emit listUpdated();
	}
}

void MasterClient::registerNewServer(ServerPtr server)
{
	server->setSelf(server.toWeakRef());
	d->servers << server;
}

void MasterClient::resetPacketCaching()
{
	if (d->cache != nullptr)
	{
		delete d->cache;
		d->cache = nullptr;
	}
}

void MasterClient::refreshStarts()
{
	setTimeouted(false);
	emptyServerList();
	resetPacketCaching();
}

bool MasterClient::sendRequest(QUdpSocket *socket)
{
	if (d->address.isNull())
		return false;

	// Make request
	QByteArray request = createServerListRequest();
	if (request.isEmpty())
		return false;
	socket->writeDatagram(request, d->address, d->port);
	return true;
}

const QList<ServerPtr> &MasterClient::servers() const
{
	return d->servers;
}

void MasterClient::setEnabled(bool b)
{
	d->enabled = b;
}

void MasterClient::setTimeouted(bool b)
{
	d->timeouted = b;
}

void MasterClient::timeoutRefresh()
{
	// Avoid timeouting more than once. This would cause errors.
	if (!isTimeouted())
	{
		setTimeouted(true);

		emit message(tr("Master server timeout"), tr("Connection timeout (%1:%2).")
			.arg(d->address.toString()).arg(d->port), true);
		readPacketCache();

		timeoutRefreshEx();
	}
}

void MasterClient::timeoutRefreshEx()
{
}

void MasterClient::updateAddress()
{
	QString host;
	unsigned short port;
	plugin()->masterHost(host, port);

	QHostInfo info = QHostInfo::fromName(host);
	if (info.addresses().size() == 0)
		return;

	d->address = info.addresses().first();
	if (d->address.protocol() != QAbstractSocket::IPv4Protocol)
	{
		for (const QHostAddress &addr : info.addresses())
		{
			if (addr.protocol() == QAbstractSocket::IPv4Protocol)
				d->address = addr;
		}
	}
	d->port = port;
}
