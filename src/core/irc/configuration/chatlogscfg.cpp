//------------------------------------------------------------------------------
// chatlogscfg.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "chatlogscfg.h"

#include "datapaths.h"
#include "irc/configuration/ircconfig.h"

DClass<ChatLogsCfg>
{
};

DPointered(ChatLogsCfg)


ChatLogsCfg::ChatLogsCfg()
{
}

ChatLogsCfg::~ChatLogsCfg()
{
}

QString ChatLogsCfg::chatLogsRootDir() const
{
	return value("ChatLogsRootDir", gDefaultDataPaths->localDataLocationPath(
		DataPaths::CHATLOGS_DIR_NAME)).toString();
}

void ChatLogsCfg::setChatLogsRootDir(const QString &val)
{
	setValue("ChatLogsRootDir", val);
}

bool ChatLogsCfg::isStoreLogs() const
{
	return value("StoreLogs", true).toBool();
}

void ChatLogsCfg::setStoreLogs(bool b)
{
	setValue("StoreLogs", b);
}

bool ChatLogsCfg::isRestoreChatFromLogs() const
{
	return value("RestoreChatFromLogs", true).toBool();
}

void ChatLogsCfg::setRestoreChatFromLogs(bool b)
{
	setValue("RestoreChatFromLogs", b);
}

bool ChatLogsCfg::isRemoveOldLogs() const
{
	return value("RemoveOldLogs", false).toBool();
}
void ChatLogsCfg::setRemoveOldLogs(bool b)
{
	setValue("RemoveOldLogs", b);
}

int ChatLogsCfg::oldLogsRemovalDaysThreshold() const
{
	int val = value("OldLogsRemovalDaysThreshold", 365).toInt();
	return qMax(1, val);
}

void ChatLogsCfg::setOldLogsRemovalDaysThreshold(int val)
{
	setValue("OldlogsRemovalDaysThreshold", val);
}

void ChatLogsCfg::setValue(const QString &key, const QVariant &value)
{
	gIRCConfig.ini()->section("Logs").setValue(key, value);
}

QVariant ChatLogsCfg::value(const QString &key, const QVariant &defValue) const
{
	return gIRCConfig.ini()->section("Logs").value(key, defValue);
}
