//------------------------------------------------------------------------------
// chatlogs.h
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
#ifndef id8a9b47a8_4986_4c15_a37a_e718e8af36fa
#define id8a9b47a8_4986_4c15_a37a_e718e8af36fa

#include "dptr.h"

#include <QObject>
#include <QString>

class IRCNetworkEntity;

class ChatLogs : public QObject
{
	Q_OBJECT

public:
	ChatLogs();
	~ChatLogs() override;

	QString logFilePath(const IRCNetworkEntity &entity, const QString &recipient) const;
	QString logFileName(const QString &recipient) const;
	bool mkLogDir(const IRCNetworkEntity &entity);
	QString networkDirPath(const IRCNetworkEntity &entity) const;
	bool renameNetwork(QWidget *parentUi, QString oldName, QString newName);

private:
	DPtr<ChatLogs> d;
};

#endif
