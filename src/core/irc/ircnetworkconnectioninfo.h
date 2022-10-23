//------------------------------------------------------------------------------
// ircnetworkconnectioninfo.h
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
#ifndef __IRCNETWORKCONNECTIONINFO_H__
#define __IRCNETWORKCONNECTIONINFO_H__

#include "irc/entities/ircnetworkentity.h"
#include "strings.hpp"
#include <QHostAddress>
#include <QString>

/**
 *	@brief Struct containing information about client's connection to the
 *	IRC server.
 */
struct IRCNetworkConnectionInfo
{
	/**
	 * @brief Alternate nickname in case if ' nick  ' is taken when
	 * connecting.
	 */
	QString alternateNick;

	/**
	 * @brief Information about the network to which we will connect.
	 */
	IRCNetworkEntity networkEntity;

	/**
	 * @brief Original nickname. This variable will always store the current
	 * nickname of the client.
	 */
	QString nick;

	/**
	 * @brief User's real name. Optional.
	 */
	QString realName;

	/**
	 * @brief User name sent in /user command.
	 */
	QString userName;

	/**
	 * @brief Fills missing data with presets.
	 *
	 * - If nick is empty it is changed to
	 *   Doomseeker_ + <4 random alphanum chars>
	 * - If alternateNick is empty it is changed to nick + "`"
	 * - If realName is empty it is changed to nick.
	 * - If userName is empty it is changed to nick.
	 */
	IRCNetworkConnectionInfo autoFilled() const
	{
		IRCNetworkConnectionInfo filled = *this;
		filled.fillInMissingFields();
		return filled;
	}

private:
	void fillInMissingFields()
	{
		if (nick.trimmed().isEmpty())
			nick = "Doomseeker_" + Strings::createRandomAlphaNumericString(4);

		if (alternateNick.trimmed().isEmpty())
			alternateNick = nick + "`";

		if (realName.trimmed().isEmpty())
			realName = nick;

		if (userName.trimmed().isEmpty())
			userName = nick;
	}
};

#endif
