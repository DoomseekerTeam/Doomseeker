//------------------------------------------------------------------------------
// rconprotocol.h
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
// Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#ifndef __RCON_PROTOCOL_H_
#define __RCON_PROTOCOL_H_

#include "dptr.h"
#include "player.h"
#include "serverapi/polymorphism.h"
#include "serverapi/serverptr.h"
#include <QThread>
#include <QUdpSocket>

/**
 * @ingroup group_pluginapi
 * @brief Remote console protocol interface.
 *
 * Doomseeker can be used to administrate remote server
 * through the server's remote console protocol. Implementation, if provided,
 * is returned through an instance of Server that also needs to be
 * implemented by the plugin.
 *
 * @section impl_and_comm Implementation and communication with Doomseeker.
 *
 * A working example of implementation is provided in Zandronum plugin
 * source code.
 *
 * Plugin implementation needs to provide all <b>[Pure Virtual]</b>
 * methods.  Doomseeker starts by asking user for RCon password. At
 * this point RConProtocol isn't even called yet. Once user provides a
 * password, the first call to sendPassword() is made. It's at this
 * point that the RConProtocol implementation should first establish
 * connection with the server, interpret any responses and send the
 * password. It's up to the plugin to determine when setConnected()
 * should be called and what conditions mean that it should be set to
 * true or false. All messages that should be displayed by Doomseeker
 * should be emitted through messageReceived() signal. This also
 * includes important error messages such as "You've been banned from
 * this server." If password specified by the user is invalid, an
 * invalidPassword() signal should be emitted so that Doomseeker can
 * ask the user for correction. When connection is lost or closed, a
 * disconnected() signal should be emitted. This signal should also be
 * emitted after a call to disconnectFromServer(), but only after the
 * plugin is certain that the connection is in fact closed. This
 * allows Doomseeker to close connection gracefully. Keep in mind that
 * Doomseeker can also call disconnectFromServer() even if it already
 * knows that isConnected() returns false. In this case it's also
 * expected that the plugin will emit disconnected() signal, even if
 * that's the only thing that it'll do in its implementation of
 * disconnectFromServer(). Each command input by the user is sent
 * through sendCommand() slot.
 *
 * There are also some utilities that allow Doomseeker to update
 * certain parts of display. For example, whenever server name changes
 * (or is first made known), the serverNameChanged() signal should be
 * emitted. Similar goes for player list which has corresponding
 * playerListUpdated() signal, upon which Doomseeker accesses
 * players() list.
 */
class MAIN_EXPORT RConProtocol : public QObject
{
	Q_OBJECT

public:
	virtual ~RConProtocol() override;

	/**
	 * @brief Whether connection is established or not.
	 *
	 * This is set manually by the plugin implementation through
	 * setConnected() setter. It means that plugin decides on its own
	 * what it means "to be connected" and when exactly the connection
	 * is established.
	 */
	bool isConnected() const;
	/**
	 * @brief List of players present on the server.
	 *
	 * This is a read-only accessor. It's used by Doomseeker to
	 * display players list. It's accessed whenever the
	 * playerListUpdated() signal is emitted. List of players can
	 * be modified through playersMutable() accessor.
	 */
	const QList<Player> &players() const;

public slots:
	/**
	 * @brief <b>[Pure Virtual]</b> Close connection with the server.
	 *
	 * Disconnects remote console and sets isConnected() state to false.
	 */
	void disconnectFromServer();
	/**
	 * @brief <b>[Pure Virtual]</b> Send command to remote server.
	 *
	 * Command is provided through Doomseeker user interface. It
	 * can be literally anything, and if the plugin needs to
	 * perform any special parsing, it needs to do so on its own.
	 */
	void sendCommand(const QString &cmd);
	/**
	 * @brief <b>[Pure Virtual]</b> Send remote console password.
	 *
	 * Password is provided through Doomseeker user interface.
	 */
	void sendPassword(const QString &password);

signals:
	/**
	 * @brief Emitted by implementation when connection with the
	 * remote server is closed.
	 */
	void disconnected();
	/**
	 * @brief Emitted by implementation if remote console password
	 * is invalid.
	 *
	 * This ought to be emitted after a call to sendPassword() if
	 * that call fails.
	 */
	void invalidPassword();
	/**
	 * @brief Passes all messages from the remote server that
	 * should be displayed to the user.
	 */
	void messageReceived(const QString &msg);
	/**
	 * @brief Emitted by implementation everytime the list of
	 * players is modified.
	 *
	 * This signal is used to refresh players list. Emit it
	 * whenever you modify players list through playersMutable()
	 * accessor.
	 */
	void playerListUpdated();
	/**
	 * @brief Emitted by implementation when server name changes.
	 *
	 * This signal is used to refresh remote console title.
	 */
	void serverNameChanged(const QString &name);

protected:
	RConProtocol(ServerPtr server);

	POLYMORPHIC_SETTER_DECLARE(void, RConProtocol, disconnectFromServer, ())
	POLYMORPHIC_SETTER_DECLARE(void, RConProtocol, sendCommand, (const QString &cmd))
	POLYMORPHIC_SETTER_DECLARE(void, RConProtocol, sendPassword, (const QString &password))

	/**
	 * @brief Address of the server to which this remote console
	 * should connect.
	 */
	const QHostAddress &address() const;
	/**
	 * @brief Port of the server to which this remote console
	 * should connect.
	 */
	quint16 port() const;
	/**
	 * @brief Accessor to players list that allows modification.
	 *
	 * Implementation should use this accessor to update players
	 * list. Everytime modification is made, a
	 * playerListUpdated() signal should be emitted.
	 */
	QList<Player> &playersMutable();
	/**
	 * @brief UDP socket that can be used to communicate with
	 * remote server.
	 *
	 * This socket is not connected in any way. It's provided as a
	 * mere convenience. RConProtocol constructor calls bind() on
	 * that socket and its destructor calls close(). Everything
	 * else needs to be programmed by the implementation. If
	 * remote console for your implementation uses a different
	 * means of communication (TCP, HTTP), you may ignore this
	 * socket and implement your own.
	 */
	QUdpSocket &socket();
	/**
	 * @brief Set this to true when connection is successfully
	 * established, set to false upon disconnect.
	 */
	void setConnected(bool b);

	friend class Server;

private:
	DPtr<RConProtocol> d;

	void disconnectFromServer_default();
	void sendCommand_default(const QString &cmd);
	void sendPassword_default(const QString &password);
};

#endif
