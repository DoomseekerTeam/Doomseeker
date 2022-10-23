//------------------------------------------------------------------------------
// zandronumqbinaries.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUMQ_BINARIES_H
#define DOOMSEEKER_PLUGIN_ZANDRONUMQ_BINARIES_H

#include <QNetworkReply>
#include <QProgressDialog>

#include <serverapi/exefile.h>

class EnginePlugin;
class IniSection;
class ZandronumqServer;
class QDir;

class ZandronumqClientExeFile : public ExeFile
{
	Q_OBJECT

public:
	static QString scriptFileExtension();

	ZandronumqClientExeFile(const QSharedPointer<const ZandronumqServer> &server);
	~ZandronumqClientExeFile() override;

	Message install(QWidget *parent) override;

	/**
	 * If the parent Server is a normal server simple path to executable
	 * file is returned. If this is a testing server, a shell script is
	 * created	if necessary and a path to this shell script s returned.
	 */
	QString pathToExe(Message &message) override;
	QString workingDirectory(Message &message) override;

private:
	class PrivData;
	PrivData *d;

	IniSection &config();
};

#endif
