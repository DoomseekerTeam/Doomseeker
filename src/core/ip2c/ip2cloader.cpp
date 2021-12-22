//------------------------------------------------------------------------------
// ip2cloader.cpp
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
#include "ip2cloader.h"

#include "configuration/doomseekerconfig.h"
#include "doomseekerfilepaths.h"
#include "global.h"
#include "ip2c/ip2c.h"
#include "ip2c/ip2cparser.h"
#include "ip2c/ip2cupdater.h"
#include "log.h"
#include <QFile>
#include <QTimer>

DClass<IP2CLoader>
{
public:
	IP2CParser *ip2cParser;
	IP2CUpdater *ip2cUpdater;
	bool updateInProgress;
	bool inFallbackMode;
};

DPointered(IP2CLoader)

///////////////////////////////////////////////////////////////////////////////
IP2CLoader::IP2CLoader(QObject *parent)
	: QObject(parent)
{
	d->updateInProgress = false;
	d->inFallbackMode = false;

	d->ip2cParser = new IP2CParser();
	this->connect(d->ip2cParser, SIGNAL(parsingFinished(bool)),
		SLOT(ip2cFinishedParsing(bool)));

	d->ip2cUpdater = new IP2CUpdater();
	this->connect(d->ip2cUpdater, SIGNAL(databaseDownloadFinished(const QByteArray&)),
		SLOT(ip2cFinishUpdate(const QByteArray&)));
	this->connect(d->ip2cUpdater, SIGNAL(downloadProgress(qint64,qint64)),
		SIGNAL(downloadProgress(qint64,qint64)));
	this->connect(d->ip2cUpdater, SIGNAL(updateNeeded(int)),
		SLOT(onUpdateNeeded(int)));
}

IP2CLoader::~IP2CLoader()
{
	if (d->ip2cParser->isParsing())
	{
		gLog << tr("IP2C parser is still working, awaiting stop...");
		while (d->ip2cParser->isParsing())
		{
			Sleep::sleep(1);
		}
	}

	delete d->ip2cParser;
	delete d->ip2cUpdater;
}

void IP2CLoader::load()
{
	if (gConfig.doomseeker.bIP2CountryAutoUpdate)
		d->ip2cUpdater->needsUpdate(DoomseekerFilePaths::ip2cDatabaseAny());
	ip2cParseDatabase();
}

void IP2CLoader::onUpdateNeeded(int status)
{
	if (status == IP2CUpdater::UpdateNeeded)
		update();
	else
	{
		switch (status)
		{
		case IP2CUpdater::UpToDate:
			gLog << tr("IP2C update not needed.");
			break;
		case IP2CUpdater::UpdateCheckError:
			gLog << tr("IP2C update errored. See log for details.");
			break;
		default:
			gLog << tr("IP2C update bugged out.");
			break;
		}
		ip2cJobsFinished();
	}
}

void IP2CLoader::update()
{
	d->updateInProgress = true;
	if (!d->ip2cParser->isParsing())
	{
		gLog << tr("Starting IP2C update.");
		IP2C::instance()->setDataAccessLockEnabled(true);
		d->ip2cUpdater->downloadDatabase(DoomseekerFilePaths::ip2cDatabase());
	}
	else
	{
		// Delay in hope that parser finishes
		gLog << tr("IP2C update must wait until parsing of current database finishes. "
				"Waiting 1 second");
		QTimer::singleShot(1000, this, SLOT(update()));
	}
}

void IP2CLoader::ip2cFinishUpdate(const QByteArray &downloadedData)
{
	d->updateInProgress = false;
	if (!downloadedData.isEmpty())
	{
		gLog << tr("IP2C database finished downloading.");
		QString filePath = DoomseekerFilePaths::ip2cDatabase();
		d->ip2cUpdater->getRollbackData(filePath);
		if (!d->ip2cUpdater->saveDownloadedData(filePath))
			gLog << tr("Unable to save IP2C database at path: %1").arg(filePath);
		ip2cParseDatabase();
	}
	else
	{
		gLog << tr("IP2C download has failed.");
		ip2cJobsFinished();
	}
}

void IP2CLoader::ip2cFinishedParsing(bool bSuccess)
{
	QString filePath = DoomseekerFilePaths::ip2cDatabase();

	if (!bSuccess)
	{
		if (d->inFallbackMode)
		{
			gLog << tr("Failed to read IP2C fallback. Stopping.");
			ip2cJobsFinished();
			return;
		}
		gLog << tr("Failed to read IP2C database. Reverting...");

		d->inFallbackMode = true;
		if (d->ip2cUpdater == nullptr || !d->ip2cUpdater->hasRollbackData())
		{
			gLog << tr("IP2C revert attempt failed. Nothing to go back to.");

			// Delete file in this case.
			QFile file(filePath);
			file.remove();

			QString preinstalledDbPath = DoomseekerFilePaths::ip2cDatabaseAny();
			if (!preinstalledDbPath.isEmpty())
			{
				gLog << tr("Trying to use preinstalled IP2C database.");
				d->ip2cParser->readDatabaseThreaded(preinstalledDbPath);
			}
			else
				ip2cJobsFinished();
		}
		else
		{
			// Revert to old content.
			d->ip2cUpdater->rollback(filePath);

			// Must succeed now.
			d->ip2cParser->readDatabaseThreaded(filePath);
		}
	}
	else
	{
		gLog << tr("IP2C parsing finished.");
		IP2C::instance()->setRangesDatabase(d->ip2cParser->ranges());
		IP2C::instance()->setCountries(d->ip2cParser->countries());
		ip2cJobsFinished();
	}
}

void IP2CLoader::ip2cJobsFinished()
{
	if (!d->ip2cUpdater->isWorking() && !d->ip2cParser->isParsing() && !d->updateInProgress)
	{
		IP2C::instance()->setDataAccessLockEnabled(false);
		emit finished();
	}
}

void IP2CLoader::ip2cParseDatabase()
{
	QString filePath = DoomseekerFilePaths::ip2cDatabaseAny();
	if (!filePath.isEmpty())
	{
		gLog << tr("Please wait. IP2C database is being read. This may take some time.");
		// Attempt to read IP2C database.

		d->inFallbackMode = false;
		IP2C::instance()->setDataAccessLockEnabled(true);
		d->ip2cParser->readDatabaseThreaded(filePath);
	}
	else
	{
		if (!gConfig.doomseeker.bIP2CountryAutoUpdate)
		{
			gLog << tr("Did not find any IP2C database. IP2C functionality will be disabled.");
			gLog << tr("You may install an IP2C database from the \"File\" menu.");
		}
		ip2cJobsFinished();
	}
}
