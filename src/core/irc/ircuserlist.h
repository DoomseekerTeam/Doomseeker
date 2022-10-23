//------------------------------------------------------------------------------
// ircuserlist.h
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
#ifndef __IRCUSERLIST_H__
#define __IRCUSERLIST_H__

#include <QString>
#include <QStringList>
#include <QVector>

class IRCUserInfo;

/**
 * @brief Allows to perform operation on a list of users.
 *
 * This is used to represent users who are present
 * on a channel.
 *
 * @b Note: the users array might not be properly sorted.
 * It is up to the views to keep up with changes in this list
 * by capturing signals and ordering nicknames by themselves.
 */
class IRCUserList
{
public:
	~IRCUserList();

	/**
	 * @brief Appends a single name to the users array.
	 *
	 * Ensures that no duplicate names are found on the list. If
	 * a duplicate is found then info is updated.
	 *
	 * @return True if name was actually added to the list. False
	 * if duplicate names were detected.
	 */
	bool appendNameToCachedList(const IRCUserInfo &userInfo);

	/**
	 * @brief Changes a nickname while preserving user flags.
	 *
	 * @return True if oldNickname was found on this list.
	 */
	bool changeNick(const QString &oldNickname, const QString &newNickname);

	void clear();

	bool hasUser(const QString &nickname) const;

	const IRCUserInfo *operator[](int index) const
	{
		return usersArray[index];
	}

	/**
	 * @brief Removes a name from the sortecd cachedNames list.
	 *
	 * @return True if the name was actually removed, false otherwise.
	 */
	bool removeNameFromCachedList(const QString &nickname);

	void setUserModes(const QString &nickname, const QList<char> &modes);

	unsigned size() const
	{
		return this->usersArray.size();
	}

	/**
	 * @brief Returns all prefixed nicknames in a string list.
	 *
	 * Please note that this list is not ordered in any way.
	 */
	QStringList toStringList() const;

	const IRCUserInfo *user(const QString &nickname) const;

	/**
	 * @brief Gets a copy of the IRCUserInfo for user with given
	 * name.
	 *
	 * If user cannot be found it will return IRCUserInfo with
	 * no nickname.
	 */
	IRCUserInfo userCopy(const QString &nickname) const;

private:
	QVector<IRCUserInfo *> usersArray;

	int indexOfName(const QString &nickname) const;
};

#endif
