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
	~ZandronumClientExeFile() override;

	Message install(QWidget *parent) override;

	/**
	 * If the parent Server is a normal server simple path to executable
	 * file is returned. If this is a testing server, a shell script is
	 * created	if necessary and a path to this shell script s returned.
	 */
	QString pathToExe(Message &message) override;
	QString workingDirectory(Message &message) override;

protected:
	bool downloadTestingBinaries(const QDir &destination, QWidget *parent);
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
	bool spawnTestingBatchFile(const QString &versionDir, QString &fullPathToFile, Message &message);

private:
	class PrivData;
	PrivData *d;

	IniSection &config();
	/**
	 * @return true - user accepts installation; false - user refuses.
	 */
	bool askToInstallTestingVersion(QWidget *parent) const;
	QString testingVersion() const;
	QString testingVersionInstallPath() const;
};

class TestingProgressDialog : public QProgressDialog
{
	Q_OBJECT

public:
	TestingProgressDialog(const QUrl &url, QWidget *parent);

	const QByteArray &data() const { return downloadedFileData; }
	QString error() const { return networkError; }
	const QString &filename() const { return downloadedFilename; }

private slots:
	void abort();
	void errorReceived(QNetworkReply::NetworkError);
	void downloadFinished();
	void downloadProgress(qint64 value, qint64 max);


private:
	QByteArray downloadedFileData;
	QString downloadedFilename;
	QNetworkAccessManager networkAccessManager;
	QNetworkReply *pNetworkReply;
	QString networkError;

	void getUrl(const QUrl &url);
};

#endif
