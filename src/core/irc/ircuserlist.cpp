//------------------------------------------------------------------------------
// ircuserlist.cpp
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
#include "irc/ircuserinfo.h"
#include "ircuserlist.h"

IRCUserList::~IRCUserList()
{
	qDeleteAll(usersArray);
}

bool IRCUserList::appendNameToCachedList(const IRCUserInfo &userInfo)
{
	int index = this->indexOfName(userInfo.cleanNickname());
	if (index >= 0)
	{
		*usersArray[index] = userInfo;
		return false;
	}
	else
	{
		auto pUserInfo = new IRCUserInfo(userInfo);
		usersArray.append(pUserInfo);
		return true;
	}
}

bool IRCUserList::changeNick(const QString &oldNickname, const QString &newNickname)
{
	const IRCUserInfo *pExistingInfo = user(oldNickname);
	if (pExistingInfo == nullptr)
		return false;

	IRCUserInfo user = *pExistingInfo;
	user.setPrefixedNickname(newNickname);
	removeNameFromCachedList(oldNickname);
	appendNameToCachedList(user);
	return true;
}

bool IRCUserList::hasUser(const QString &nickname) const
{
	return indexOfName(nickname) != -1;
}

int IRCUserList::indexOfName(const QString &nickname) const
{
	for (int i = 0; i < usersArray.size(); ++i)
	{
		const IRCUserInfo &storedUser = *usersArray[i];
		if (storedUser.isSameNickname(nickname))
			return i;
	}

	return -1;
}

bool IRCUserList::removeNameFromCachedList(const QString &nickname)
{
	int index = indexOfName(nickname);
	if (index < 0)
		return false;

	delete usersArray[index];
	usersArray.remove(index);
	return true;
}

void IRCUserList::setUserModes(const QString &nickname, const QList<char> &modes)
{
	int index = this->indexOfName(nickname);
	if (index >= 0)
	{
		IRCUserInfo &userInfo = *usersArray[index];
		userInfo.setModes(modes);
	}
}

QStringList IRCUserList::toStringList() const
{
	QStringList nicksList;
	for (auto *i : usersArray)
		nicksList << i->prefixedName();

	return nicksList;
}

const IRCUserInfo *IRCUserList::user(const QString &nickname) const
{
	int index = this->indexOfName(nickname);
	if (index < 0)
		return nullptr;

	return usersArray[index];
}

IRCUserInfo IRCUserList::userCopy(const QString &nickname) const
{
	const IRCUserInfo *pUserInfo = user(nickname);
	if (pUserInfo == nullptr)
		return IRCUserInfo();
	return *pUserInfo;
}
