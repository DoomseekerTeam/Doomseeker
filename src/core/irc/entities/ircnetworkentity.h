//------------------------------------------------------------------------------
// ircnetworkentity.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __IRCNETWORKENTITY_H__
#define __IRCNETWORKENTITY_H__

#include "dptr.h"
#include "global.h"
#include <QString>
#include <QStringList>

class PatternList;
class QVariant;

/**
 * @ingroup group_pluginapi
 * @brief Data structure that describes and defines a connection to an IRC
 * network or server.
 */
class MAIN_EXPORT IRCNetworkEntity
{
public:
	static IRCNetworkEntity deserializeQVariant(const QVariant &var);

	IRCNetworkEntity();
	virtual ~IRCNetworkEntity();

	/**
	 * @brief Address of the server or network to connect to.
	 */
	const QString &address() const;

	/**
	 * @brief List of channels to which a /join command will be issued
	 * automatically when a connection with this network is established.
	 */
	const QStringList &autojoinChannels() const;
	QStringList &autojoinChannels();
	/**
	 * @brief List of commands executed on network join.
	 */
	const QStringList &autojoinCommands() const;
	QStringList &autojoinCommands();

	/**
	 * @brief A short, human-readable description for the network.
	 * (Preferably a single word).
	 *
	 * Only limited set of characters is allowed in network description,
	 * as this description is used to name the directory where
	 * chat logs are being kept. These characters are letters, digits,
	 * spaces, and also '_' and '-'. All other characters will be
	 * automatically converted to '_' by setDescription().
	 */
	const QString &description() const;

	const PatternList &ignoredUsers() const;
	void setIgnoredUsers(const PatternList &val);

	/**
	 * @brief Join this network when Doomseeker starts up.
	 */
	bool isAutojoinNetwork() const;

	bool isValid() const;

	const QString &nickservCommand() const;
	const QString &nickservPassword() const;

	/**
	 * @brief Sorts by description.
	 */
	bool operator<(const IRCNetworkEntity &other) const;

	/**
	 * @brief Checks to see if this is the same server.
	 */
	bool operator==(const IRCNetworkEntity &other) const;

	/**
	 * @brief Password for the server or network. Ignored if empty.
	 */
	const QString &password() const;

	/**
	 * @brief Port of the server or network to connect to.
	 *
	 * Default value: 6667
	 */
	unsigned short port() const;

	QVariant serializeQVariant() const;

	void setAddress(const QString &v);
	void setAutojoinChannels(const QStringList &v);
	void setAutojoinCommands(const QStringList &v);
	void setAutojoinNetwork(bool v);
	void setDescription(const QString &v);
	void setNickservCommand(const QString &v);
	void setNickservPassword(const QString &v);
	void setPassword(const QString &v);
	void setPort(unsigned short v);

private:
	DPtr<IRCNetworkEntity> d;
};

#endif
