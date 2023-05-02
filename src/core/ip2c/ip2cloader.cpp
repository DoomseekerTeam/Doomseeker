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
#include <QElapsedTimer>
#include <QFile>
#include <QTimer>
#include <QThread>

namespace
{
class IP2CParserThread : public QThread
{
	Q_OBJECT

public:
	QString filePath;
	IP2CParser parser;
	bool success;

	IP2CParserThread(QString filePath)
		: filePath(filePath)
	{}

	virtual void run() override
	{
		QFile file(filePath);
		if (file.open(QIODevice::ReadOnly))
		{
			QElapsedTimer time;
			time.start();
			this->success = parser.parse(file);

			if (this->success)
			{
				gLog << tr("IP2C database read in %1 ms; IP ranges: %2").arg(time.elapsed()).arg(parser.ranges().size());
			}
		}
		else
		{
			gLog << tr("Unable to open IP2C file: %1").arg(filePath);
			this->success = false;
		}
	}
};
}

DClass<IP2CLoader>
{
public:
	IP2C *ip2c;
	IP2CParserThread *parserThread;
	QScopedPointer<IP2CUpdater> ip2cUpdater;
	bool updateInProgress;
	bool inFallbackMode;

	bool isParsing() const
	{
		return parserThread != nullptr && parserThread->isRunning();
	}
};

DPointeredNoCopy(IP2CLoader)

///////////////////////////////////////////////////////////////////////////////
IP2CLoader::IP2CLoader(IP2C &ip2c, QObject *parent)
	: QObject(parent)
{
	d->ip2c = &ip2c;
	d->parserThread = nullptr;
	d->updateInProgress = false;
	d->inFallbackMode = false;

	d->ip2cUpdater.reset(new IP2CUpdater());
	connect(d->ip2cUpdater.data(), &IP2CUpdater::databaseDownloadFinished,
		this, &IP2CLoader::onUpdateFinished);
	connect(d->ip2cUpdater.data(), &IP2CUpdater::downloadProgress,
		this, &IP2CLoader::downloadProgress);
	connect(d->ip2cUpdater.data(), &IP2CUpdater::updateNeeded,
		this, &IP2CLoader::onUpdateNeeded);
}

void IP2CLoader::load()
{
	if (gConfig.doomseeker.bIP2CountryAutoUpdate)
		d->ip2cUpdater->needsUpdate(DoomseekerFilePaths::ip2cDatabaseAny());

	if (d->isParsing())
		return;

	QString filePath = DoomseekerFilePaths::ip2cDatabaseAny();
	if (!filePath.isEmpty())
	{
		d->inFallbackMode = false;
		d->ip2c->setDataAccessLockEnabled(true);
		ip2cReadDatabase(filePath);
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

void IP2CLoader::onUpdateNeeded(int status)
{
	if (status == IP2CUpdater::UpdateNeeded)
	{
		update();
	}
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
	if (!d->isParsing())
	{
		gLog << tr("Starting IP2C update.");
		IP2C::instance()->setDataAccessLockEnabled(true);
		d->ip2cUpdater->downloadDatabase(DoomseekerFilePaths::ip2cDatabase());
	}
}

void IP2CLoader::onUpdateFinished(const QByteArray &downloadedData)
{
	d->updateInProgress = false;
	if (!downloadedData.isEmpty())
	{
		gLog << tr("IP2C database finished downloading.");
		QString filePath = DoomseekerFilePaths::ip2cDatabase();
		d->ip2cUpdater->getRollbackData(filePath);
		if (!d->ip2cUpdater->saveDownloadedData(filePath))
			gLog << tr("Unable to save IP2C database at path: %1").arg(filePath);
		ip2cReadDatabase(filePath);
	}
	else
	{
		gLog << tr("IP2C download has failed.");
		ip2cJobsFinished();
	}
}

void IP2CLoader::onParsingFinished()
{
	auto finally = [this]() {
		if (d->updateInProgress) {
			// Retrigger the update.
			QTimer::singleShot(0, this, &IP2CLoader::update);
		}
		this->ip2cJobsFinished();
	};

	IP2CParserThread *parserThread = d->parserThread;
	if (parserThread == nullptr) {
		gLog << tr("IP2C unable to retrieve the parsing result.");
		finally();
		return;
	}
	d->parserThread = nullptr; // object deleted by the deleteLater() slot
	QString filePath = DoomseekerFilePaths::ip2cDatabase();
	if (!parserThread->success)
	{
		if (d->inFallbackMode)
		{
			gLog << tr("Failed to read the IP2C fallback. Stopping.");
			finally();
			return;
		}
		gLog << tr("Failed to read the IP2C database. Reverting...");

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
				gLog << tr("Trying to use the preinstalled IP2C database.");
				ip2cReadDatabase(preinstalledDbPath);
			}
			else
			{
				finally();
			}
		}
		else
		{
			// Revert to old content.
			d->ip2cUpdater->rollback(filePath);

			// Must succeed now.
			ip2cReadDatabase(filePath);
		}
	}
	else
	{
		const IP2CParser &parser = parserThread->parser;
		gLog << tr("IP2C parsing finished.");
		d->ip2c->setRangesDatabase(parser.ranges());
		d->ip2c->setLicence(parser.licences());
		d->ip2c->setUrl(parser.url());
		finally();
	}
}

void IP2CLoader::ip2cJobsFinished()
{
	if (!d->ip2cUpdater->isWorking() && !d->isParsing() && !d->updateInProgress)
	{
		d->ip2c->setDataAccessLockEnabled(false);
		emit finished();
	}
}

void IP2CLoader::ip2cReadDatabase(const QString &filePath)
{
	gLog << tr("IP2C database is being read. This may take some time.");

	if (d->parserThread != nullptr) {
		d->parserThread->disconnect(this);
	}

	d->parserThread = new IP2CParserThread(filePath);
	connect(d->parserThread, &IP2CParserThread::finished,
		this, &IP2CLoader::onParsingFinished);
	connect(d->parserThread, &IP2CParserThread::finished,
		d->parserThread, &QObject::deleteLater);
	d->parserThread->start();
}

#include "ip2cloader.moc"
