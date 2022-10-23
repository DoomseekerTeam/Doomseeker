//------------------------------------------------------------------------------
// passwordscfg.cpp
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
#include "passwordscfg.h"

#include "configuration/doomseekerconfig.h"
#include "configuration/serverpassword.h"
#include "configuration/serverpasswordsummary.h"
#include "ini/ini.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "ini/settingsproviderqt.h"
#include "serverapi/server.h"
#include "serverapi/serversummary.h"
#include <cassert>
#include <QDebug>

const QString SECTION_NAME = "Passwords";

const QString MAX_NUMBER_OF_SERVERS_PER_PASSWORD_KEY = "MaxNumberOfServersPerPassword";
const QString REMEMBER_CONNECT_PASSWORD = "RememberConnectPassword";
const QString SERVER_PASSWORDS_KEY = "ServerPasswords";

// TODO: [Zalewa] We have 3 different ini files, all instantiated
// in Main and kept in different places. Perhaps we should move them
// out to a separate singleton? If not, then perhaps we could at least
// move the instantiation out of Main.
Ini *PasswordsCfg::ini = nullptr;
QSettings *PasswordsCfg::settings = nullptr;


DClass<PasswordsCfg>
{
public:
	IniSection section;
};

DPointered(PasswordsCfg)

PasswordsCfg::PasswordsCfg()
{
	assert(ini != nullptr && "instantiated PasswordsCfg() without initing ini");
	d->section = ini->section(SECTION_NAME);
}

PasswordsCfg::~PasswordsCfg()
{
}

void PasswordsCfg::cutServers(QList<ServerPassword> &passwords) const
{
	QMutableListIterator<ServerPassword> it(passwords);
	while (it.hasNext())
	{
		ServerPassword &password = it.next();
		QList<ServerPasswordSummary> sortedServers = password.servers();
		std::sort(sortedServers.begin(), sortedServers.end(), serverDateDescending);
		password.setServers(sortedServers.mid(0, maxNumberOfServersPerPassword()));
	}
}

void PasswordsCfg::cutStoredServers()
{
	QList<ServerPassword> passwords = serverPasswords();
	cutServers(passwords);
	storeServerPasswords(passwords);
}

void PasswordsCfg::initIni(const QString &path)
{
	assert(ini == nullptr && "tried to re-init password ini");
	if (ini != nullptr)
	{
		qDebug() << "Error: tried to re-init password ini";
		return;
	}
	settings = new QSettings(path, QSettings::IniFormat);
	// SettingsProviderQt won't be deleted. Not really a memory leak, as
	// this is expected to remain memory for whole program runtime.
	ini = new Ini(new SettingsProviderQt(settings));
}

bool PasswordsCfg::isHidingPasswords() const
{
	return gConfig.doomseeker.bHidePasswords;
}

bool PasswordsCfg::isRememberingConnectPhrase() const
{
	return d->section.value(REMEMBER_CONNECT_PASSWORD, false).toBool();
}

int PasswordsCfg::maxNumberOfServersPerPassword() const
{
	return d->section.value(MAX_NUMBER_OF_SERVERS_PER_PASSWORD_KEY, 5).toInt();
}

void PasswordsCfg::removeServerPhrase(const QString &phrase)
{
	QList<ServerPassword> allPasswords = serverPasswords();
	QMutableListIterator<ServerPassword> it(allPasswords);
	while (it.hasNext())
	{
		ServerPassword existingPass = it.next();
		if (existingPass.phrase() == phrase)
			it.remove();
	}
	storeServerPasswords(allPasswords);
}

void PasswordsCfg::saveServerPhrase(const QString &phrase, const Server *server,
	const QString &type)
{
	if (phrase.isEmpty())
		return;

	ServerPasswordSummary serverInfo;
	if (server != nullptr)
	{
		ServerSummary serverSummary(server);
		serverSummary.setTime(QDateTime::currentDateTime());
		serverInfo.setServerSummary(serverSummary);
	}
	serverInfo.setType(type);
	QList<ServerPassword> allPasswords = serverPasswords();
	QMutableListIterator<ServerPassword> it(allPasswords);
	while (it.hasNext())
	{
		// Try to add server to existing password.
		ServerPassword &existingPass = it.next();
		if (existingPass.phrase() == phrase)
		{
			if (serverInfo.isValid())
				existingPass.addServer(serverInfo);
			setServerPasswords(allPasswords);
			return;
		}
	}
	// Add new.
	ServerPassword pass;
	pass.setPhrase(phrase);
	pass.addServer(serverInfo);
	allPasswords << pass;
	storeServerPasswords(allPasswords);
}

bool PasswordsCfg::serverDateDescending(ServerPasswordSummary &s1, ServerPasswordSummary &s2)
{
	return s1.time() > s2.time();
}

void PasswordsCfg::setHidePasswords(bool val)
{
	gConfig.doomseeker.bHidePasswords = val;
}

QList<ServerPassword> PasswordsCfg::serverPasswords() const
{
	QList<ServerPassword> result;
	QVariantList vars = d->section[SERVER_PASSWORDS_KEY].value().toList();
	for (const QVariant &var : vars)
	{
		result << ServerPassword::deserializeQVariant(var);
	}
	return result;
}

QStringList PasswordsCfg::serverPhrases() const
{
	QStringList result;
	for (const ServerPassword &pass : serverPasswords())
	{
		result << pass.phrase();
	}
	return result;
}

void PasswordsCfg::setMaxNumberOfServersPerPassword(int val)
{
	bool shallCut = val < maxNumberOfServersPerPassword();
	d->section.setValue(MAX_NUMBER_OF_SERVERS_PER_PASSWORD_KEY, val);
	if (shallCut)
		cutStoredServers();
}

void PasswordsCfg::setRememberConnectPhrase(bool val)
{
	return d->section.setValue(REMEMBER_CONNECT_PASSWORD, val);
}

void PasswordsCfg::setServerPasswords(const QList<ServerPassword> &val)
{
	QList<ServerPassword> passwords = val;
	cutServers(passwords);
	storeServerPasswords(passwords);
}

void PasswordsCfg::storeServerPasswords(const QList<ServerPassword> &val)
{
	QVariantList vars;
	for (const ServerPassword obj : val)
	{
		vars << obj.serializeQVariant();
	}
	d->section.setValue(SERVER_PASSWORDS_KEY, vars);
}

ServerPassword PasswordsCfg::suggestPassword(const Server *server, const QString &type)
{
	// This method would probably work better as a separate class.
	// If there's ever any need to expand it, extract it
	// to a PasswordSuggester or something like that first.
	ServerPasswordSummary serverSummary(server, type);

	ServerPassword password;
	ServerPasswordSummary bestFit;
	for (const ServerPassword &potentialPassword : serverPasswords())
	{
		float newSimilarity;
		ServerPasswordSummary candidate = potentialPassword.mostSimilarServer(serverSummary, &newSimilarity);
		if (candidate.isValid())
		{
			if (newSimilarity > bestFit.similarity(serverSummary))
			{
				bestFit = candidate;
				password = potentialPassword;
			}
			else if (qFuzzyCompare(newSimilarity, bestFit.similarity(serverSummary))
				&& candidate.time() > bestFit.time())
			{
				bestFit = candidate;
				password = potentialPassword;
			}
		}
	}
	return password;
}
