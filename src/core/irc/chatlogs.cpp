//------------------------------------------------------------------------------
// chatlogs.cpp
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
#include "chatlogs.h"

#include "irc/configuration/chatlogscfg.h"
#include "irc/entities/ircnetworkentity.h"
#include <QDir>
#include <QMessageBox>

DClass<ChatLogs>
{
public:
	QString rootPath() const
	{
		return ChatLogsCfg().chatLogsRootDir();
	}
};

DPointered(ChatLogs)


ChatLogs::ChatLogs()
{
}

ChatLogs::~ChatLogs()
{
}

QString ChatLogs::logFilePath(const IRCNetworkEntity &entity, const QString &recipient) const
{
	return QString("%1/%2.txt").arg(networkDirPath(entity), logFileName(recipient));
}

QString ChatLogs::logFileName(const QString &recipient) const
{
	if (!recipient.trimmed().isEmpty())
		return recipient.trimmed().toLower();
	else
		return "@main";
}

bool ChatLogs::mkLogDir(const IRCNetworkEntity &entity)
{
	QDir dir(networkDirPath(entity));
	return dir.mkpath(".");
}

QString ChatLogs::networkDirPath(const IRCNetworkEntity &entity) const
{
	return QString("%1/%2").arg(d->rootPath(), entity.description().trimmed().toLower());
}

bool ChatLogs::renameNetwork(QWidget *parentUi, QString oldName, QString newName)
{
	oldName = oldName.trimmed().toLower();
	newName = newName.trimmed().toLower();
	if (oldName == newName)
		return true;
	QDir dir(d->rootPath());
	if (!dir.exists(oldName))
		return true;
	while (true)
	{
		QMessageBox::StandardButton result = QMessageBox::Ok;
		QString error;
		if (dir.exists(newName))
		{
			error = tr("Won't transfer chat logs from \"%1\" to \"%2\" as directory \"%2\""
				"already exists.").arg(oldName, newName);
		}
		else if (!dir.rename(oldName, newName))
			error = tr(R"(Failed to transfer chat from "%1" to "%2")").arg(oldName, newName);

		if (error.isEmpty())
			break;
		result = QMessageBox::warning(parentUi, tr("Chat logs transfer"), error,
			QMessageBox::Ignore | QMessageBox::Retry | QMessageBox::Abort);
		if (result != QMessageBox::Retry)
			return result == QMessageBox::Ignore;
	}
	return true;
}
