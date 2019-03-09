//------------------------------------------------------------------------------
// odamexbinaries.h
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------

#ifndef DOOMSEEKER_PLUGIN_ODAMEX_BINARIES_H_
#define DOOMSEEKER_PLUGIN_ODAMEX_BINARIES_H_

#include "dptr.h"

#include <serverapi/exefile.h>

class OdamexServer;
class IniSection;
class InstallationPublicData;

class OdamexClientExeFile : public ExeFile
{
	Q_OBJECT
public:
	OdamexClientExeFile(const QSharedPointer<const OdamexServer> &server);
	QString pathToExe(Message& message, const InternalClientsInspection internalCliInspection = NoInternalChecking);

	const InstallationPublicData createInstallationData() const;
	QString workingDirectory();
	const QUrl getUrl(Message& message) const;
	void postInstallationSteps(Message& message);
	const QStringList getExecutableNames() const;
private:
	bool spawnInternalBatchFile(const QString& versionDir, QString& fullPathToFile, Message& message);
	QString tryFindCompatibleSystemEngine(Message &message);
	QString version() const;
	QString versionInstallPath() const;
	DPtr<OdamexClientExeFile> d;
};

#endif