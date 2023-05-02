//------------------------------------------------------------------------------
// exefile.h
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id97064B0C_7D3A_489E_B2241AD6943450FB
#define id97064B0C_7D3A_489E_B2241AD6943450FB

#include "dptr.h"
#include "global.h"
#include <QHash>
#include <QObject>
#include <QString>

class Message;
class IniSection;
class InstallationPublicData;
class Server;

/**
 * @ingroup group_pluginapi
 * @brief Access to external program executables
 *        (game clients, servers, and so on).
 *
 * ExeFile is an interface to external programs executables. It obtains
 * directory and file paths to the executable with pathToExe()
 * and workingDirectory(), defines name of
 * configKey() under which the executable path can be stored,
 * names the program in general through programName() and also
 * names the purpose of the exec within that program through
 * exeTypeName() and lastly, it optionally handles install() procedure
 * if user and plugin desire so.
 */
class MAIN_EXPORT ExeFile : public QObject
{
	Q_OBJECT

public:
	ExeFile(const QSharedPointer<Server> &server);
	virtual ~ExeFile() override;

	/**
	 * @brief Config key where executable path on current system
	 *        can be remembered.
	 */
	const QString &configKey() const;
	/**
	 * @brief Name of the type of the executable (server, client, etc.).
	 */
	const QString &exeTypeName() const;
	/**
	 * @brief Name of the program this executable belongs to (ex. "Odamex").
	 */
	const QString &programName() const;

	/**
	 * @brief Used in pathToExe(), Allows you to adjust the amount of
	 * checking it's being done in relation to internal clients.
	 */
	enum InternalClientsInspection
	{
		NoInternalChecking,
		InternalCheckingWithoutSystemCheck,
		InternalChecking
	};

	/**
	 * @brief Returns the path to the executable file.
	 *
	 * @param [out] message
	 *     Information or error message, if any.
	 *     The type of the message might be relevant in some cases,
	 *     for instance Message::Type::GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED
	 *     will be returned if the plugin is capable of installing its own
	 *     client.
	 *
	 * @param internalCliInspection
	 *     Adjusts the amount of checking it's being done in relation to
	 *     internal clients. For instance, passing InternalChecking will
	 *     in some cases run the system's client to check if it's
	 *     compatible.
	 *
	 * @return Path to the file if all fine or empty if error.
	 */
	virtual const QString pathToExe(Message &message, const InternalClientsInspection internalCliInspection = NoInternalChecking) const;

	/**
	 * @brief Plugin setter for configKey().
	 */
	void setConfigKey(const QString &keyName);
	/**
	 * @brief Plugin setter for exeTypeName().
	 */
	void setExeTypeName(const QString &name);
	/**
	 * @brief Plugin setter for programName().
	 */
	void setProgramName(const QString &name);

	/**
	 * @brief Path to this executable working directory.
	 *
	 * Default behavior returns directory of the default pathToExe(), but
	 * you can override this to provide different working directory
	 * if needed.
	 *
	 * @param [out] message
	 *     Information or error message, if any.
	 * @return Path to the working directory if all fine or empty if error.
	 */
	virtual QString workingDirectory(Message &message);

	/**
	 * @brief Path to a directory based on the version of the server.
	 * This path should be used for the internal clients.
	 */
	virtual const QString versionInstallPath() const;

	/**
	 * @brief Create a InstallationPublicData object that contains
	 * information that will be exposed to the user during the installation
	 * of an internal client.
	 *
	 * Default implementation returns info gathered based on the plugin's
	 * information.
	 */
	virtual const InstallationPublicData createInstallationData() const;

	/**
	 * @brief Allows the execution of pre-installation steps during the
	 * installation of an internal client.
	 *
	 * Default implementation does nothing.
	 */
	virtual void preInstallationSteps(Message &message) const;

	/**
	 * @brief Allows the execution of post-installation steps during the
	 * installation of an internal client.
	 *
	 * Default implementation does nothing.
	 */
	virtual void postInstallationSteps(Message &message) const;

	/**
	 * @brief Create a QUrl object that contains the url from which the
	 * internal client must be downloaded.
	 *
	 * Default implementation does nothing and returns ignorable object.
	 */
	virtual const QUrl getUrl(Message &message) const;

	/**
	 * @brief Returns a list of all the executables that a client must have.
	 * Generally, this list will be platform-dependant, and will only be one
	 * entry.
	 *
	 * Default implementation does nothing and returns ignorable list.
	 */
	virtual const QString getClientExecutableName() const;

	/**
	 * @brief Returns the file extension of a batch script. OS dependant.
	 */
	const static QString scriptFileExtension();

protected:
	/**
	 * @brief Path specified in the config.
	 */
	const QString configBinaryPath() const;

	/**
	 * Creates Unix .sh file or Windows .bat file to
	 * launch client for parent server. Returns true if the file
	 * already exists. This implementation can be overwritten.
	 * @param versionDir - convenience parameter. This is the directory
	 *        where the internal package was unpacked.
	 * @param [out] fullPathToFile - path to created script file.
	 * @param [out] error - error if return == false.
	 * @return false if fail.
	 */
	virtual bool spawnInternalBatchFile(const QString& versionDir, QString& fullPathToFile, Message& message) const;

	/**
	 * @brief IniSection of the current plugin.
	 */
	const IniSection config() const;

	/**
	 * @brief Version of the server.
	 */
	virtual const QString version() const;

	/**
	 * @brief Access the Server class.
	 */
	const QSharedPointer<Server> &server() const;

	/**
	 * @brief Check if directory is empty.
	 */
	const static bool isEmptyDir(const QString &path);

	/**
	 * @brief Check if directory is not empty.
	 */
	const static bool isNonEmptyDir(const QString &path);
	DPtr<ExeFile> d;
};

/**
 * @ingroup group_pluginapi
 * @brief A simple executable path & working dir structure.
 */
class MAIN_EXPORT ExeFilePath
{
public:
	ExeFilePath();
	/**
	 * @brief Infers working dir from path.
	 */
	ExeFilePath(const QString &path);
	ExeFilePath(const ExeFilePath &other);
	virtual ~ExeFilePath();
	ExeFilePath &operator=(const ExeFilePath &other);

	QString path() const;
	ExeFilePath &setPath(const QString &path);

	QString workingDir() const;
	ExeFilePath &setWorkingDir(const QString &path);

private:
	DPtr<ExeFilePath> d;
};

#endif
