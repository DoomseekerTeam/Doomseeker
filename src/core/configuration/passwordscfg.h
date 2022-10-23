//------------------------------------------------------------------------------
// passwordscfg.h
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
#ifndef id1E18D75F_28FA_48A1_9CACE55638B41394
#define id1E18D75F_28FA_48A1_9CACE55638B41394

#include "dptr.h"

#include <QList>
#include <QSettings>
#include <QString>
#include <QStringList>

class Ini;
class Server;
class ServerPassword;
class ServerPasswordSummary;

class PasswordsCfg
{
public:
	static void initIni(const QString &path);

	PasswordsCfg();
	~PasswordsCfg();

	bool isHidingPasswords() const;
	bool isRememberingConnectPhrase() const;
	int maxNumberOfServersPerPassword() const;
	void removeServerPhrase(const QString &phrase);
	/**
	 * @brief Stores server phrase in persistence along its use case.
	 *
	 * @param phrase
	 *     Phrase to store
	 * @param server
	 *     Server on which the password is being used.
	 * @param type
	 *     Password type, one of ServerPasswordType consts or custom.
	 */
	void saveServerPhrase(const QString &phrase, const Server *server,
		const QString &type);
	QList<ServerPassword> serverPasswords() const;
	QStringList serverPhrases() const;
	void setHidePasswords(bool val);
	void setMaxNumberOfServersPerPassword(int val);
	void setRememberConnectPhrase(bool val);
	void setServerPasswords(const QList<ServerPassword> &val);
	/**
	 * @brief Suggests best password basing on several criteria.
	 *
	 * @param server
	 *     Server for which a password shall be suggested.
	 * @param type
	 *     Password type, one of ServerPasswordType consts or custom.
	 */
	ServerPassword suggestPassword(const Server *server, const QString &type);

private:
	static bool serverDateDescending(ServerPasswordSummary &s1, ServerPasswordSummary &s2);

	static Ini *ini;
	static QSettings *settings;

	DPtr<PasswordsCfg> d;

	void cutServers(QList<ServerPassword> &passwords) const;
	void cutStoredServers();
	void storeServerPasswords(const QList<ServerPassword> &val);
};

#endif
