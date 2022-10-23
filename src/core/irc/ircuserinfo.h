//------------------------------------------------------------------------------
// ircuserinfo.h
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
#ifndef __IRCUSERINFO_H__
#define __IRCUSERINFO_H__

#include <QList>
#include <QString>

class IRCUserPrefix;
class IRCNetworkAdapter;

/**
 * @brief Holds information flags about given nickname.
 */
class IRCUserInfo
{
public:
	/**
	 * @brief Full user signature with nickname and hostname.
	 *
	 * It must be noted that this variable value might not be
	 * valid even if IRCUserInfo is containing a valid nickname
	 * information. In fact this value will always be empty
	 * for chat adapters. However network adapters will require
	 * this signature info to deliver bans.
	 */
	QString fullSignature;

	/**
	 * @brief Creates invalid IRCUserInfo.
	 *
	 * isValid() returns false.
	 */
	IRCUserInfo();
	/**
	 * @brief Constructor will set flags based on nickname's
	 * prefix.
	 */
	IRCUserInfo(const QString &nickname, const IRCNetworkAdapter *parentNetwork,
		const QString &fullSignature = "");

	/**
	 * @brief Returns nickname with no prefixes, contrary to the
	 *        prefixedName() .
	 */
	QString cleanNickname() const;

	/**
	 * @brief Returns cleanNickname() with a call to
	 * IRCGlobal::toIrcLower() .
	 */
	QString cleanNicknameLowerCase() const;

	QString extractHostnameFromFullSignature() const;

	/**
	 * @brief Check if this user and user specified as parameter
	 * are the same user.
	 *
	 * Calls the operator==.
	 */
	bool isSameNickname(const IRCUserInfo &otherUser) const;
	bool isSameNickname(const QString &otherNickname) const;

	bool isOp() const;
	bool isValid() const;

	const QList<char> &modes() const;
	const IRCNetworkAdapter *network() const;

	bool operator==(const IRCUserInfo &otherUser) const;

	/**
	 * @brief This operator can be used for sorting.
	 *
	 * @return True if this user's nickname is earlier in alphabet
	 * than other user's nickname. Also true if this user has one of the
	 * following flags over the other user: op, voice (in this order).
	 */
	bool operator<=(const IRCUserInfo &otherUser) const;

	/**
	 * @brief Will generate prefix based on the user flags.
	 */
	QString prefixedName() const;

	/**
	 * @brief Returns prefixedName() with a call to
	 * IRCGlobal::toIrcLower() .
	 */
	QString prefixedNameLowerCase() const;

	void setModes(const QList<char> &modes);
	void setMode(char mode);
	void setPrefixedNickname(const QString &nickname);
	void unsetMode(char mode);

private:
	const IRCNetworkAdapter *parentNetwork;
	QList<char> userModes;
	QString userName;

	const IRCUserPrefix &prefixes() const;
};

#endif
