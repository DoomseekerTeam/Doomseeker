//------------------------------------------------------------------------------
// masterclient.h
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

#ifndef __MASTERSERVER_H__
#define __MASTERSERVER_H__

#include <QHostAddress>
#include <QList>
#include <QObject>

#include "dptr.h"
#include "global.h"
#include "polymorphism.h"
#include "serverapi/serverptr.h"

class Message;
class EnginePlugin;
class Server;
class QFile;
class QUdpSocket;

/**
 * @ingroup group_pluginapi
 * @brief Abstract base for all MasterClients.
 *
 * This is expected to fetch a list of IP addresses which will be turned
 * into Servers.
 */
class MAIN_EXPORT MasterClient : public QObject
{
	Q_OBJECT

public:
	enum Response
	{
		RESPONSE_GOOD, // Data is available
		RESPONSE_TIMEOUT, // Server didn't respond at all
		RESPONSE_WAIT, // Server responded with "wait"
		RESPONSE_BAD, // Probably refreshing too quickly
		RESPONSE_BANNED, // Won't receive data from this server ever.
		RESPONSE_NO_RESPONSE_YET, // "Dummy" response for servers that weren't refreshed yet
		RESPONSE_PENDING, // Waiting for additional packets
		RESPONSE_REPLY, // Ask the refresher to call createSendRequest again
		RESPONSE_OLD, // Client too old.
	};

	MasterClient();
	virtual ~MasterClient() override;

	/**
	 * Clears the server list.
	 */
	void clearServers();

	/**
	 * @brief Extracts engine name from pluginInfo() if available.
	 *
	 * @return If pluginInfo() is nullptr then this returns an empty string.
	 */
	QString engineName() const;

	/**
	 * @brief Returns true if the passed address:port is the same as this
	 * master server's.
	 */
	bool isAddressSame(const QHostAddress &address, unsigned short port) const;

	/**
	 * Serves as an informative role for MasterManager.
	 * If the master client is disabled, master manager will omit
	 * it during the refresh.
	 */
	bool isEnabled() const;
	/**
	 * @brief Indicates that the server has timeouted recently.
	 *
	 * This is reset to false by refresh() and set to true by
	 * timeoutRefresh(). If you reimplement refresh() please remember
	 * to set this to false.
	 */
	bool isTimeouted() const;

	/**
	 * @brief [Virtual] Help message displayed to the user when
	 * ban is detected.
	 *
	 * Doomseeker displays a "you're banned from this master
	 * server" message when RESPONSE_BANNED is returned by
	 * readResponse(). By redefining this method, your plugin may
	 * show additional help message next to the usual
	 * one. Normally, you'd have this return some kind of staff
	 * contact info. Basic HTML is supported in the display.
	 *
	 * Default implementation returns an empty string.
	 */
	QString masterBanHelp() const;

	void notifyResponse(Response response);
	int numServers() const;
	ServerPtr operator[] (int index) const;

	/**
	 * This is supposed to return the plugin this MasterClient belongs to.
	 * If it doesn't belong to any plugin then return nullptr.
	 * New instances of EnginePlugin shouldn't be created here. Instead
	 * each plugin should keep a global instance of EnginePlugin (singleton?)
	 * and a pointer to this instance should be returned.
	 */
	virtual const EnginePlugin *plugin() const = 0;

	void pushPacketToCache(const QByteArray &data);
	void resetPacketCaching();

	/**
	 * @brief Calls readMasterResponse and handles packet caching.
	 */
	Response readResponse(const QByteArray &data);

	/**
	 * @brief Sends request packet through socket.
	 */
	bool sendRequest(QUdpSocket *socket);

	const QList<ServerPtr> &servers() const;

	void updateAddress();

public slots:
	/**
	 * Requests an updated server list from the master.
	 *
	 * This function is virtual since MasterManager overrides it.
	 */
	virtual void refreshStarts();

	/**
	 * @see isEnabled()
	 */
	void setEnabled(bool b);

	/**
	 * @brief Times the refreshing process out.
	 *
	 * This calls timeoutRefreshEx() and then emits listUpdated() signal.
	 */
	void timeoutRefresh();

signals:
	void listUpdated();

	/**
	 * Plugins may use this to make Doomseeker display custom messages.
	 * Messages are dumped into the log.
	 */
	void message(const QString &title, const QString &content, bool isError);

	/**
	 * @brief Signal used to forward important message to Doomseeker.
	 *
	 * Important messages will be treated differently by Doomseeker than
	 * regular ones thrown by the message() signal. Doomseeker will display
	 * them in a location that will ensure that users see this message.
	 *
	 * <b>Example use:</b> notify user that they have been banned from the
	 * master server.
	 */
	void messageImportant(const Message &message);

protected:
	/**
	 * @brief Address of the master server.
	 */
	QHostAddress address() const;

	/**
	 * @brief Produce contents of server list request packet that is sent
	 *        to the master server.
	 *
	 * @return If empty array is returned refreshing of this master server
	 *         is immediately aborted.
	 */
	virtual QByteArray createServerListRequest() = 0;

	/**
	 * Clears the server list; alias to clearServers().
	 */
	void emptyServerList();

	POLYMORPHIC_SETTER_DECLARE_CONST(QString, MasterClient, masterBanHelp, ())
	QString masterBanHelp_default() const;

	/**
	 * @brief Network port of the master server.
	 */
	unsigned short port() const;

	bool preparePacketCache(bool write);

	/**
	 * @brief Called to read and analyze the response from the
	 * MasterServer.
	 */
	virtual Response readMasterResponse(const QByteArray &data) = 0;

	void readPacketCache();
	/**
	 * @brief Registers new server with this MasterClient.
	 */
	void registerNewServer(ServerPtr server);

	void setTimeouted(bool b);

	/**
	 * @brief Reimplement this for clean up purposes.
	 */
	virtual void timeoutRefreshEx();

private:
	DPtr<MasterClient> d;

	void emitBannedMessage();
};

#endif /* __MASTERSERVER_H__ */
