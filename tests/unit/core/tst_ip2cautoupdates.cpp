//------------------------------------------------------------------------------
// tst_ip2cautoupdates.cpp
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
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "doomseekerfilepaths.h"
#include "gui/configuration/cfgip2c.h"
#include "ip2c/ip2c.h"
#include "ip2c/ip2cloader.h"
#include "ip2c/ip2cupdater.h"

#include <QCheckBox>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QNetworkProxy>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QtTest>

namespace
{
QByteArray databaseFixture()
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream.writeRawData("IP2C", 4);
	stream << quint16(2);
	const char country[] = "Poland\0POL\0";
	stream.writeRawData(country, sizeof(country) - 1);
	stream << quint32(1)
		<< QHostAddress("203.0.113.0").toIPv4Address()
		<< QHostAddress("203.0.113.255").toIPv4Address();
	return data;
}
}

class TestIP2CAutoUpdates : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void load_data();
	void load();
	void configuration();
	void manualUpdateCheck();
	void manualDownload();

private:
	QTemporaryDir storage;
	QTcpServer proxy;
	QNetworkProxy originalProxy;
	QStringList originalSearchPaths;
};

void TestIP2CAutoUpdates::initTestCase()
{
	QVERIFY(storage.isValid());
	QStandardPaths::setTestModeEnabled(true);
	DataPaths::initDefault(false);
	gDefaultDataPaths->setBaseDir(storage.path());
	DoomseekerFilePaths::pDataPaths = gDefaultDataPaths;
	const QString databaseDirectory = gDefaultDataPaths->localDataLocationPath();
	QVERIFY(QDir().mkpath(databaseDirectory));
	originalSearchPaths = QDir::searchPaths("data");
	QDir::setSearchPaths("data", QStringList{databaseDirectory});

	// Keep even a regressed automatic check away from the real update service.
	// This proxy accepts connections but never responds, so an unwanted request
	// would also prevent the loader from reporting that it has finished.
	QVERIFY(proxy.listen(QHostAddress::LocalHost));
	originalProxy = QNetworkProxy::applicationProxy();
	QNetworkProxy::setApplicationProxy(QNetworkProxy(
		QNetworkProxy::HttpProxy, "127.0.0.1", proxy.serverPort()));
}

void TestIP2CAutoUpdates::cleanupTestCase()
{
	QNetworkProxy::setApplicationProxy(originalProxy);
	QDir::setSearchPaths("data", originalSearchPaths);
	DoomseekerFilePaths::pDataPaths = nullptr;
	IP2C::deinstantiate();
	DoomseekerConfig::dispose();
}

void TestIP2CAutoUpdates::init()
{
	QFile::remove(DoomseekerFilePaths::ip2cDatabase());
	IP2C::deinstantiate();
}

void TestIP2CAutoUpdates::load_data()
{
	QTest::addColumn<bool>("autoUpdateSetting");
	QTest::addColumn<bool>("hasDatabase");
	QTest::newRow("setting-off-no-database") << false << false;
	QTest::newRow("setting-off-local-database") << false << true;
	#ifndef DOOMSEEKER_WITH_IP2C_AUTOUPDATES
	QTest::newRow("setting-on-no-database") << true << false;
	QTest::newRow("setting-on-local-database") << true << true;
	#endif
}

void TestIP2CAutoUpdates::load()
{
	// Given: automatic updates are disabled by the user or the build, with or
	// without an installed database. A saved preference must not override the
	// build option.
	QFETCH(bool, autoUpdateSetting);
	QFETCH(bool, hasDatabase);
	QScopedValueRollback<bool> setting(
		gConfig.doomseeker.bIP2CountryAutoUpdate, autoUpdateSetting);
	if (hasDatabase)
	{
		QFile file(DoomseekerFilePaths::ip2cDatabase());
		QVERIFY(file.open(QIODevice::WriteOnly));
		const QByteArray data = databaseFixture();
		QCOMPARE(file.write(data), qint64(data.size()));
	}
	IP2C &ip2c = *IP2C::instance();
	IP2CLoader loader(ip2c);
	QSignalSpy finished(&loader, SIGNAL(finished()));
	QSignalSpy progress(&loader, SIGNAL(downloadProgress(qint64,qint64)));

	// When: the database is loaded as it would be on startup.
	loader.load();

	// Then: loading finishes without an automatic download or checksum check,
	// and an installed database still supports country lookups.
	if (hasDatabase)
		QTRY_COMPARE(finished.count(), 1);
	else
		QCOMPARE(finished.count(), 1);
	QCOMPARE(progress.count(), 0);
	QVERIFY(!proxy.hasPendingConnections());
	QVERIFY(!ip2c.isDataAccessLocked());
	QCOMPARE(ip2c.hasData(), hasDatabase);
	if (hasDatabase)
	{
		QCOMPARE(ip2c.numKnownEntries(), 1);
		QCOMPARE(ip2c.countryInfoForAddress(QHostAddress("203.0.113.23")).name,
			QString("Poland"));
	}
	QCOMPARE(gConfig.doomseeker.bIP2CountryAutoUpdate, autoUpdateSetting);
}

void TestIP2CAutoUpdates::configuration()
{
	// Given: a saved preference for automatic updates and the IP2C settings page.
	QScopedValueRollback<bool> autoUpdate(gConfig.doomseeker.bIP2CountryAutoUpdate, true);
	QScopedValueRollback<bool> honorCountries(gConfig.doomseeker.bHonorServerCountries, true);
	CFGIP2C page(nullptr);

	// When: the settings are read.
	page.read();
	auto *automatic = page.findChild<QCheckBox *>("cbAutoUpdate");
	auto *honor = page.findChild<QCheckBox *>("cbHonorServerCountries");

	// Then: only builds supporting automatic updates offer that setting.
	QVERIFY(automatic != nullptr);
	QVERIFY(honor != nullptr);
	#ifdef DOOMSEEKER_WITH_IP2C_AUTOUPDATES
	QVERIFY(!automatic->isHidden());
	QVERIFY(automatic->isEnabled());
	#else
	QVERIFY(automatic->isHidden());
	QVERIFY(!automatic->isEnabled());
	#endif
	QVERIFY(!honor->isHidden());
	QVERIFY(honor->isEnabled());

	// When: the user saves a change to the remaining IP2C setting.
	honor->setChecked(false);
	page.setAllowSave(true);
	QVERIFY(page.save());

	// Then: that setting is saved without discarding the automatic-update
	// preference that another build might still use.
	QVERIFY(!gConfig.doomseeker.bHonorServerCountries);
	QVERIFY(gConfig.doomseeker.bIP2CountryAutoUpdate);
}

void TestIP2CAutoUpdates::manualUpdateCheck()
{
	// Given: no local database and an updater used by the manual update dialog.
	IP2CUpdater updater;
	QSignalSpy checked(&updater, SIGNAL(updateCheckFinished(int)));

	// When: the user explicitly checks whether an update is needed.
	updater.startUpdateCheck(DoomseekerFilePaths::ip2cDatabase());

	// Then: the manual check still reports that a database needs downloading.
	QCOMPARE(checked.count(), 1);
	QCOMPARE(checked.at(0).at(0).toInt(), int(IP2CUpdater::UpdateNeeded));
	QVERIFY(!updater.isWorking());
}

void TestIP2CAutoUpdates::manualDownload()
{
	// Given: an updater for a user-requested database download.
	IP2CUpdater updater;

	// When: a manual download is started.
	updater.downloadDatabase(DoomseekerFilePaths::ip2cDatabase());

	// Then: the request is allowed even when automatic updates are disabled.
	QVERIFY(updater.isWorking());
}

QTEST_MAIN(TestIP2CAutoUpdates)

#include "tst_ip2cautoupdates.moc"
