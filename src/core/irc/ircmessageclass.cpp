//------------------------------------------------------------------------------
// ircmessageclass.cpp
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
#include "irc/configuration/ircconfig.h"
#include "ircmessageclass.h"

QString IRCMessageClass::colorFromConfig() const
{
	IRCConfig::AppearanceCfg &appearance = gIRCConfig.appearance;
	switch (className)
	{
	case ChannelAction:
		return appearance.channelActionColor;
	case Error:
		return appearance.errorColor;
	case Normal:
		return appearance.defaultTextColor;
	case NetworkAction:
		return appearance.networkActionColor;
	case Ctcp:
		return appearance.ctcpColor;
	default:
		return "#000000";
	}
}

QString IRCMessageClass::toStyleSheetClassName(ClassName className)
{
	switch (className)
	{
	case ChannelAction:
		return "channelAction";
	case Ctcp:
		return "ctcp";
	case Error:
		return "error";
	case Normal:
		return "";
	case NetworkAction:
		return "networkAction";
	default:
		return "appErr";
	}
}
