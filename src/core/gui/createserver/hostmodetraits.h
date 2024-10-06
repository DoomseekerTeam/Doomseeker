//------------------------------------------------------------------------------
// hostmodetraits.h
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_GUI_CREATESERVER_HOSTMODETRAITS_H
#define DOOMSEEKER_GUI_CREATESERVER_HOSTMODETRAITS_H

#include "serverapi/gamecreateparams.h"

/**
 * UI-related feature checker for GameCreateParams::HostMode.
 */
class HostModeTraits
{
public:
	GameCreateParams::HostMode mode;

	HostModeTraits(GameCreateParams::HostMode mode = GameCreateParams::Host)
		: mode(mode)
	{}

	/**
	 * Create HostMode from a config name (cfgName()).
	 */
	static HostModeTraits fromCfgName(const QString &name)
	{
		if (name == "offline")
			return GameCreateParams::Offline;
		// Default to Host if there's any other value.
		return GameCreateParams::Host;
	}

	/**
	 * Map the GameCreateParams::HostMode to a name saveable in a config.
	 */
	QString cfgName() const
	{
		switch (mode)
		{
		default:
		case GameCreateParams::Host: return "host";
		case GameCreateParams::Offline: return "offline";
		}
	}

	HostModeTraits &operator=(GameCreateParams::HostMode mode)
	{
		this->mode = mode;
		return *this;
	}

	operator GameCreateParams::HostMode () const
	{
		return mode;
	}

	/**
	 * Is changing the game allowed for this hosting mode?
	 */
	bool canChangeGame() const
	{
		return mode == GameCreateParams::Host
			|| mode == GameCreateParams::Offline;
	}

	/**
	 * Can we switch the UI dynamically from this mode to another?
	 */
	bool canChangeMode() const
	{
		return mode == GameCreateParams::Host
			|| mode == GameCreateParams::Offline;
	}

	/**
	 * Does this mode allow to setup demo recording?
	 */
	bool canRecordDemo() const
	{
		return mode == GameCreateParams::Offline;
	}

	/**
	 * Can a launch command line be generated in this mode?
	 */
	bool canShowCommandLine() const
	{
		return mode != GameCreateParams::Remote;
	}

	/**
	 * Are we launching a brand-new game server?
	 */
	bool isCreatingServer() const
	{
		return mode == GameCreateParams::Host;
	}

	/**
	 * Are we setting up an online game (our own server or an existing one)?
	 */
	bool isSettingUpOnlineGame() const
	{
		return mode == GameCreateParams::Host
			|| mode == GameCreateParams::Remote;
	}
};

#endif
