//------------------------------------------------------------------------------
// zandronumbinaries.cpp
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
#include "log.h"

#include "../wadseeker/zip/unarchive.h"
#include "application.h"
#include "plugins/engineplugin.h"
#include "serverapi/message.h"
#include "version.h"
#include "zandronumbinaries.h"
#include "zandronumengineplugin.h"
#include "zandronumserver.h"

#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QScopedPointer>
#include <strings.hpp>

#define TESTING_BINARY_URL_BASE "https://zandronum.com/downloads/testing/%1/ZandroDev%1-%2"

#ifdef Q_OS_WIN32
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "windows.zip"
#define ZANDRONUM_BINARY_NAME "zandronum.exe"
#define SCRIPT_FILE_EXTENSION ".bat"
#else

#ifndef __x86_64__
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "linux-x86.tar.bz2"
#else
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "linux-x86_64.tar.bz2"
#endif

#ifndef Q_OS_DARWIN
#define ZANDRONUM_BINARY_NAME "zandronum"
#else
#define ZANDRONUM_BINARY_NAME "Zandronum.app"
#define ZANDRONUM_APP_BUNDLE_BIN "/Contents/MacOS/zandronum"
#endif
#define SCRIPT_FILE_EXTENSION ".sh"
#endif

class ZandronumClientExeFile::PrivData
{
public:
	QSharedPointer<const ZandronumServer> server;

	bool isEmptyDir(const QString &path) const
	{
		QDir dir(path);
		return dir.exists() && dir.entryList(
			QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}

	bool isNonEmptyDir(const QString &path) const
	{
		QDir dir(path);
		return dir.exists() && !dir.entryList(
			QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}
};

ZandronumClientExeFile::ZandronumClientExeFile(const QSharedPointer<const ZandronumServer> &server)
{
	d = new PrivData();
	d->server = server;
	setProgramName(server->plugin()->data()->name);
	setExeTypeName(tr("client"));
	setConfigKey("BinaryPath");
}

ZandronumClientExeFile::~ZandronumClientExeFile()
{
	delete d;
}

IniSection &ZandronumClientExeFile::config()
{
	return *ZandronumEnginePlugin::staticInstance()->data()->pConfig;
}

Message ZandronumClientExeFile::install(QWidget *parent)
{
	if (askToInstallTestingVersion(parent))
	{
		QString path = config()["TestingPath"];
		if (path.isEmpty())
		{
			return Message::customError(tr("No testing directory specified for Zandronum"));
		}

		QDir dir(path);
		if (!dir.mkpath("."))
		{
			return Message::customError(tr("Unable to create directory:\n%1").arg(path));
		}
		if (!dir.mkpath(testingVersion()))
		{
			return Message::customError(tr("Unable to create directory:\n%1/%2").arg(
				path, testingVersion()));
		}

		QString finalDestinationPath = path + '/' + testingVersion();

		// Now copy all .ini's. On Linux .ini's are kept in ~/.zandronum so this will
		// do nothing, but on Windows this should work like magic.
		Message message;
		QDir baseBinaryDir(workingDirectory(message));
		QStringList nameFilters;
		nameFilters << "*.ini";
		QStringList iniFiles = baseBinaryDir.entryList(nameFilters, QDir::Files);
		for (QString str : iniFiles)
		{
			QString sourcePath = baseBinaryDir.absolutePath() + '/' + str;
			QString targetPath = finalDestinationPath + '/' + str;
			QFile file(sourcePath);
			file.copy(targetPath);
		}

		if (!downloadTestingBinaries(finalDestinationPath, parent))
		{
			QMessageBox::information(parent, tr("Doomseeker"),
				tr("Please install now version \"%1\" into:\n%2").arg(testingVersion(), path));
		}

		return Message(Message::Type::SUCCESSFUL);
	}
	return Message(Message::Type::CANCELLED);
}

bool ZandronumClientExeFile::askToInstallTestingVersion(QWidget *parent) const
{
	QString question;
	QMessageBox::Icon icon = QMessageBox::NoIcon;
	if (d->isNonEmptyDir(testingVersionInstallPath()))
	{
		question = tr("<p>Installation of testing binaries for version %1 "
			"can potentially <b>overwrite</b> your files.</p>"
			"<p>Game will be installed to:<br>%2</p>"
			"<p>Do you want Doomseeker to extract Zandronum files, potentially "
			"<b>overwriting existing ones</b>, and to copy all your configuration "
			"files from your base directory?</p>").arg(
			testingVersion(), testingVersionInstallPath());
		icon = QMessageBox::Warning;
	}
	else
	{
		question = tr("Zandronum testing version %1 can be installed.\n\n"
			"Game will be installed to:\n%2\n\n"
			"Do you want Doomseeker to install that version and copy all "
			"your configuration files from your base directory?").arg(
			testingVersion(), testingVersionInstallPath());
		icon = QMessageBox::Question;
	}
	QMessageBox box(icon, tr("Doomseeker - install Zandronum testing version"), question,
		QMessageBox::Yes | QMessageBox::No, parent);

	// Gross hack to enable text copying from a QMessageBox.
	// http://stackoverflow.com/a/32595502/1089357
	box.setStyleSheet("QMessageBox { messagebox-text-interaction-flags: 5; }");

	return box.exec() == QMessageBox::Yes;
}

QString ZandronumClientExeFile::pathToExe(Message &message)
{
	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;

	if (!d->server->isTestingServer() || !config["EnableTesting"])
	{
		return ExeFile::pathToExe(message);
	}
	else
	{
		message = Message();
		QString error;

		QString testingPathRoot = config["TestingPath"];
		if (testingPathRoot.isEmpty())
		{
			error = tr("No testing directory configured for Zandronum");
			message = Message::customError(error);
			return QString();
		}

		QString path = testingVersionInstallPath();

		QFileInfo fi(path);
		if (!fi.exists() || d->isEmptyDir(path))
		{
			error = tr("%1\ndoesn't exist.\nYou need to install new testing binaries.").arg(path);
			message = Message(Message::Type::GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED, error);
			return QString();
		}

		if (!fi.isDir())
		{
			error = tr("%1\nexists but is NOT a directory.\nCannot proceed.").arg(path);
			message = Message::customError(error);
			return QString();
		}

		QString binPath = path + '/' + ZANDRONUM_BINARY_NAME;
		fi = QFileInfo(binPath);
		if (!fi.exists() || (fi.isDir() && !fi.isBundle()))
		{
			error = tr("%1\nexists but doesn't contain a Zandronum executable.\n\n"
				"Doomseeker can still install the game there if you want.").arg(path);
			message = Message(Message::Type::GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED, error);
			return QString();
		}

		// Everything checked out, so proceed to create (if necessary) and return path to the script file.
		QString retPath;
		if (!spawnTestingBatchFile(path, retPath, message))
		{
			// message is already specified inside spawnTestingBatchFile()
			return QString();
		}

		return retPath;
	}
}

QString ZandronumClientExeFile::testingVersion() const
{
	QString testingVersion = d->server->gameVersion();
	// Strip out extraneous data in the version number.
	return testingVersion.left(testingVersion.indexOf(' '));
}

QString ZandronumClientExeFile::testingVersionInstallPath() const
{
	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;
	QString path =  config["TestingPath"];
	return Strings::combinePaths(path, testingVersion());
}

QString ZandronumClientExeFile::workingDirectory(Message &message)
{
	Q_UNUSED(message);
	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;

	QFileInfo fi((QString)config["BinaryPath"]);
	return fi.absolutePath();
}

bool ZandronumClientExeFile::downloadTestingBinaries(const QDir &destination, QWidget *parent)
{
	#ifdef Q_OS_DARWIN
	// Can't do anything for Mac OS X at this time. :/
	return false;
	#else
	// Download testing binaries
	ZandronumVersion version(d->server->gameVersion());

	// Find the hg revision string
	QString hgVersion;
	if (version.hgVersionDate() < 4000) // Arbitrary number here to determine between old and new format
		hgVersion = QString("%1").arg(version.hgVersionDate());
	else
		hgVersion = QString("%1-%2").arg(version.hgVersionDate()).arg(version.hgVersionTime(), 4, 10, QChar('0'));

	// Get URL
	QString versionPrefix;
	if (version.revisionVersion() != 0)
	{
		versionPrefix = QString("%1.%2.%3").arg(version.majorVersion())
			.arg(version.minorVersion()).arg(version.revisionVersion());
	}
	else
	{
		versionPrefix = QString("%1.%2").arg(version.majorVersion())
			.arg(version.minorVersion());
	}

	QUrl url(QString(TESTING_BINARY_URL).arg(versionPrefix).arg(hgVersion));
	gLog << tr("Downloading Zandronum testing binary from URL: %1").arg(url.toString());
	TestingProgressDialog dialog(url, parent);
	int ret = dialog.exec();
	if (!dialog.error().isEmpty())
	{
		QMessageBox::critical(parent, tr("Doomseeker - download failed"),
			tr("Failed to download testing binary.\n\n%1").arg(dialog.error()));
		return false;
	}
	else if (ret == QDialog::Accepted)
	{
		// Extract the needed files.
		QString filename;
		QFileInfo fi(dialog.filename());
		QByteArray data = dialog.data();
		QBuffer dataBuffer(&data);
		QScopedPointer<UnArchive> archive(UnArchive::openArchive(fi, &dataBuffer));

		if (!archive.isNull())
		{
			for (int i = 0;; ++i)
			{
				filename = archive->fileNameFromIndex(i);

				if (filename.isNull())
				{
					break;
				}

				QString fileTargetPath = destination.path() + QDir::separator() + filename;
				gLog << tr("Unpacking file: %1").arg(fileTargetPath);
				if (archive->extract(i, fileTargetPath))
				{
					// Make sure we can execute the binary.
					if (filename == ZANDRONUM_BINARY_NAME)
					{
						QFile binaryFile(fileTargetPath);
						binaryFile.setPermissions(binaryFile.permissions() | QFile::ExeUser);
					}
				}
				else
				{
					QMessageBox::critical(parent, tr("Doomseeker - unpack failed"),
						tr("Failed to unpack: %1").arg(filename));
					return false;
				}
			}
			return true;
		}
	}

	return false;
	#endif
}

QString ZandronumClientExeFile::scriptFileExtension()
{
	return SCRIPT_FILE_EXTENSION;
}

bool ZandronumClientExeFile::spawnTestingBatchFile(const QString &versionDir, QString &fullPathToFile, Message &message)
{
	QString binaryPath = versionDir + '/' + ZANDRONUM_BINARY_NAME;
	// This will create an actual path to file, because there is no '/' at the end
	// of scriptFilepath.
	fullPathToFile = versionDir + SCRIPT_FILE_EXTENSION;
	QFileInfo fi(fullPathToFile);
	QFile file(fullPathToFile);
	if (fi.isDir())
	{
		QString error = tr("%1\n should be a script file but is a directory!").arg(fullPathToFile);
		message = Message::customError(error);
		return false;
	}

	if (fi.exists())
	{
		if ((file.permissions() & QFile::ExeUser) == 0)
		{
			QString error = tr("You don't have permissions to execute file: %1\n").arg(fullPathToFile);
			message = Message::customError(error);
			return false;
		}
		return true;
	}

	QString content;
	#ifdef Q_OS_WIN32
	// Create Windows batch file
	// Extract drive letter:
	QString driveLetter;
	QString workDir = workingDirectory(message);
	for (int i = 0; i < workDir.length(); ++i)
	{
		if (workDir[i] == ':')
		{
			driveLetter = workDir.left(i);
		}
	}

	if (!driveLetter.isEmpty())
	{
		content += driveLetter + ":\r\n";
	}

	QString cdDir = workingDirectory(message).replace('/', '\\');
	QString exePath = binaryPath.replace('/', '\\');

	content += "cd \"" + cdDir + "\"\r\n";
	content += "\"" + exePath + "\" %*"; // %* deals with all the parameters
	#else
	QString cdDir = workingDirectory(message);

	// Create Unix script file
	content  = "#!/bin/sh\n";
	content += "cd \"" + cdDir + "\" \n";
	content += "export LANG=C\n";
	#ifdef Q_OS_DARWIN
	content += "\"" + binaryPath + ZANDRONUM_APP_BUNDLE_BIN "\" \"$@\"\n";
	#else
	content += "\"" + binaryPath + "\" \"$@\"\n"; // "$@" deals with all the parameters
	#endif
	#endif

	if (!file.open(QIODevice::WriteOnly))
	{
		QString error = tr("Couldn't open batch file \"%1\" for writing").arg(fullPathToFile);
		message = Message::customError(error);
		return false;
	}

	if (file.write(content.toUtf8()) < 0)
	{
		QString error = tr("Error while writing batch file \"%1\"").arg(fullPathToFile);
		message = Message::customError(error);
		file.close();
		return false;
	}

	file.close();

	if (!file.setPermissions(file.permissions() | QFile::ExeUser))
	{
		QString error = tr("Cannot set permissions for file:\n%1").arg(fullPathToFile);
		message = Message::customError(error);
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////

TestingProgressDialog::TestingProgressDialog(const QUrl &url, QWidget *parent)
	: QProgressDialog(tr("Downloading testing binaries..."), tr("Cancel"), 0, 0, parent)
{
	connect(this, SIGNAL(canceled()), this, SLOT(abort()));

	getUrl(url);

	setAutoClose(false);
	setAutoReset(false);
	setMinimumDuration(0);
}

void TestingProgressDialog::abort()
{
	pNetworkReply->abort();
}

void TestingProgressDialog::downloadProgress(qint64 value, qint64 max)
{
	setValue(value);
	setMaximum(max);
}

void TestingProgressDialog::downloadFinished()
{
	QUrl possibleRedirectUrl = pNetworkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	QUrl url = pNetworkReply->request().url();
	if (!possibleRedirectUrl.isEmpty()
		&& possibleRedirectUrl != url)
	{
		// Redirect.
		if (possibleRedirectUrl.isRelative())
		{
			possibleRedirectUrl = url.resolved(possibleRedirectUrl);
		}

		pNetworkReply->deleteLater();
		getUrl(possibleRedirectUrl);
	}
	else
	{
		QUrl url = pNetworkReply->request().url();
		QFileInfo fi(url.path());
		QString filename = fi.fileName();

		downloadedFilename = filename;
		downloadedFileData = pNetworkReply->readAll();

		accept();
	}
}

void TestingProgressDialog::errorReceived(QNetworkReply::NetworkError code)
{
	Q_UNUSED(code);
	networkError = pNetworkReply->errorString();
}

void TestingProgressDialog::getUrl(const QUrl &url)
{
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", Version::userAgent().toUtf8());

	this->pNetworkReply = networkAccessManager.get(request);

	connect(pNetworkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
	connect(pNetworkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorReceived(QNetworkReply::NetworkError)));
	connect(pNetworkReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}
