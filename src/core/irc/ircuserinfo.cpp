//------------------------------------------------------------------------------
// ircuserinfo.cpp
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
#include "ircuserinfo.h"

#include "irc/entities/ircuserprefix.h"
#include "irc/ircglobal.h"
#include "irc/ircnetworkadapter.h"
#include <cassert>

IRCUserInfo::IRCUserInfo()
{
	this->parentNetwork = nullptr;
}

IRCUserInfo::IRCUserInfo(const QString &nickname, const IRCNetworkAdapter *parentNetwork,
	const QString &fullSignature)
{
	this->fullSignature = fullSignature;
	this->parentNetwork = parentNetwork;

	if (nickname.isEmpty() || parentNetwork == nullptr)
		return;

	this->userName = parentNetwork->userPrefixes().cleanNickname(nickname);
	this->userModes << parentNetwork->userPrefixes().modeFromNickname(nickname);
}

QString IRCUserInfo::cleanNickname() const
{
	return prefixes().cleanNickname(userName);
}

QString IRCUserInfo::cleanNicknameLowerCase() const
{
	return IRCGlobal::toIrcLower(this->cleanNickname());
}

QString IRCUserInfo::extractHostnameFromFullSignature() const
{
	if (!this->fullSignature.isEmpty())
	{
		int indexOfDelimiterChar = this->fullSignature.indexOf('@');
		QString hostName = this->fullSignature;
		hostName.remove(0, indexOfDelimiterChar + 1);

		return hostName;
	}

	return "";
}

bool IRCUserInfo::isOp() const
{
	return modes().contains('o');
}

bool IRCUserInfo::isSameNickname(const IRCUserInfo &otherUser) const
{
	return (*this) == otherUser;
}

bool IRCUserInfo::isSameNickname(const QString &otherNickname) const
{
	IRCUserInfo otherUser(otherNickname, network());
	return isSameNickname(otherUser);
}

bool IRCUserInfo::isValid() const
{
	return !userName.isEmpty() && parentNetwork != nullptr;
}

const QList<char> &IRCUserInfo::modes() const
{
	return userModes;
}

const IRCNetworkAdapter *IRCUserInfo::network() const
{
	return parentNetwork;
}

void IRCUserInfo::setPrefixedNickname(const QString &nickname)
{
	this->userName = prefixes().cleanNickname(nickname);
	if (prefixes().modeFromNickname(nickname) != 0)
		setMode(prefixes().modeFromNickname(nickname));
}

bool IRCUserInfo::operator==(const IRCUserInfo &otherUser) const
{
	QString thisNickname = this->cleanNicknameLowerCase();
	QString otherNickname = otherUser.cleanNicknameLowerCase();

	return thisNickname.compare(otherNickname) == 0;
}

bool IRCUserInfo::operator<=(const IRCUserInfo &otherUser) const
{
	assert(parentNetwork != nullptr);
	char mode1 = prefixes().topMostMode(modes());
	char mode2 = prefixes().topMostMode(otherUser.modes());
	if (prefixes().compare(mode1, mode2) != 0)
		return prefixes().compare(mode1, mode2) < 0;

	QString thisNickname = this->cleanNicknameLowerCase();
	QString otherNickname = otherUser.cleanNicknameLowerCase();

	bool bIsThisAlphabeticallySmaller = (thisNickname.compare(otherNickname) <= 0);
	return bIsThisAlphabeticallySmaller;
}

QString IRCUserInfo::prefixedName() const
{
	assert(parentNetwork != nullptr);
	char mode = prefixes().topMostMode(modes());
	if (mode != 0)
	{
		return QString("%1%2").arg(prefixes().prefixForMode(mode))
			.arg(cleanNickname());
	}
	else
		return cleanNickname();
}

QString IRCUserInfo::prefixedNameLowerCase() const
{
	return IRCGlobal::toIrcLower(this->prefixedName());
}

const IRCUserPrefix &IRCUserInfo::prefixes() const
{
	return parentNetwork->userPrefixes();
}

void IRCUserInfo::setModes(const QList<char> &modes)
{
	this->userModes = modes;
}

void IRCUserInfo::setMode(char mode)
{
	if (!userModes.contains(mode))
		userModes << mode;
}

void IRCUserInfo::unsetMode(char mode)
{
	userModes.removeAll(mode);
}
