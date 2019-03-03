//------------------------------------------------------------------------------
// internalclientinstaller.h
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

#ifndef __INTERNALCLIENTINSTALLER_H_
#define __INTERNALCLIENTINSTALLER_H_

#include <QDialog>
#include <QDir>
#include <QNetworkReply>

#include "dptr.h"
#include "global.h"

class ExeFile;
class Message;
class QAbstractButton;

/**
 * @brief Contains information that will be exposed to the user by the
 * InternalClientInstaller class
 *
 */
struct MAIN_EXPORT InstallationPublicData
{
	QPixmap engineIcon;
	QString engineName;
	QString engineVersion;
	QDir installationPath;
};

/**
 * @ingroup group_pluginapi
 * @brief Handles most of the installation of an internal client.
 *
 * This class will handle the common steps of the installation of an
 * internal client, while at the same time allowing a given ExeFile execute a
 * function before the download and after the download, plus leting it
 * completely handle the creation of the url from which InternalClientInstaller
 * will retrieve the archives. The usual steps to use this class are as follows:
 *
 * - Once an instance of this class is created and opened, the user will confirm
 * the installation.
 * - Some sanity checks are run regarding the installation directory.
 * - The function preInstallationSteps() of ExeFile is called.
 * - The function getUrl() of ExeFile is called and the files are downloaded,
 *   unpacked, and the executables are set with their propper permissions.
 * - the function postInstallationSteps() of ExeFile is called.
 * The user can cancel the process during all steps.
 * The window will close itself once everything is done.
 */
class MAIN_EXPORT InternalClientInstaller : public QDialog
{
	Q_OBJECT;
public:
	/**
	 * @brief Create a new instance of InternalClientInstaller.
	 *
	 * @param parent the parent window.
	 * @param engine the ExeFile of the plugin we're trying to install. It must
	 * suport internal clients for doomseeker.
	 */
	InternalClientInstaller(QWidget *parent, ExeFile *engine);
	~InternalClientInstaller();
	/**
	 * @brief Get the Message object with the information of whether everything
	 *        was successful or not.
	 *
	 * Doomseeker will behave differently depending on
	 * the Type of the message:
	 * - Message::Type::SUCCESSFUL if installation completes properly,
	 *   in this case Doomseeker continues any operation that was
	 *   interrupted by the installation procedure (like game launch).
	 * - Message::Type::CANCELLED if installation was cancelled by the user,
	 *   in this case Doomseeker aborts silently.
	 * - Return error type to prompt Doomseeker to display an error message.
	 */
	Message& getMessage() const;
private:
	DPtr<InternalClientInstaller> d;
	void getUrl(const QUrl& url);
	void isDownloading(const bool& downlading);
	void unpackDownload();
	void directorySanityCheck();
	void reject(Message message);
private slots:
	void buttonBoxClicked(QAbstractButton* button);
	void reject() override;
	void errorReceived(QNetworkReply::NetworkError);
	void downloadFinished();
	void downloadProgress(qint64 value, qint64 max);

};

#endif