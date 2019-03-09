//------------------------------------------------------------------------------
// zandronumbinaries.h
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
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUM_BINARIES_H
#define DOOMSEEKER_PLUGIN_ZANDRONUM_BINARIES_H

#include <QNetworkReply>
#include <QProgressDialog>

#include <serverapi/exefile.h>

class EnginePlugin;
class IniSection;
class ZandronumServer;
class QDir;

class ZandronumClientExeFile : public ExeFile
{
	Q_OBJECT

public:
	static QString scriptFileExtension();

	ZandronumClientExeFile(const QSharedPointer<const ZandronumServer> &server);
	~ZandronumClientExeFile();

	/**
	 * If the parent Server is a normal server simple path to executable
	 * file is returned. If the client is an internal client, a shell
	 * script is created if necessary and a path to this shell script is
	 * returned.
	 */
	QString pathToExe(Message &message, const InternalClientsInspection internalCliInspection = NoInternalChecking);

	const InstallationPublicData createInstallationData() const;
	void preInstallationSteps(Message& message);
	const QUrl getUrl(Message& message) const;
	const QStringList getExecutableNames() const;
protected:
	/**
	 * Creates Unix .sh file or Windows .bat file to
	 * launch client for parent server. Returns true if the file
	 * already exists.
	 * @param versionDir - convenience parameter. This is the directory
	 *     where testing package was unpacked. This path was
	 *     already created in clientBinary() method so let's reuse it.
	 * @param [out] fullPathToFile - path to created script file
	 * @param [out] error - error if return == false
	 * @return false if fail
	 */
	bool spawnInternalBatchFile(const QString& versionDir, QString& fullPathToFile, Message& message);

private:
	class PrivData;
	PrivData *d;

	IniSection &config();
	QString version() const;
	QString versionInstallPath() const;
	QString tryFindCompatibleSystemEngine(Message &message);
	QString workingDirectory();
};

#endif
