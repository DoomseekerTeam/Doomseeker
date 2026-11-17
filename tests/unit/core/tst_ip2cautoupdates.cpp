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
#include "doomseekerpolicy.h"
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
#include <QTcpSocket>
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
	void cleanup();
	void load_data();
	void load();
	void configuration_data();
	void configuration();
	void manualUpdateCheck_data();
	void manualUpdateCheck();
	void manualDownload_data();
	void manualDownload();
	void manualLoaderUpdate_data();
	void manualLoaderUpdate();

private:
	ScopedDoomseekerPolicy baseline{DoomseekerPolicy::Builder().build()};
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

void TestIP2CAutoUpdates::cleanup()
{
	// Request owners have been destroyed before draining the isolated proxy.
	while (proxy.hasPendingConnections())
		delete proxy.nextPendingConnection();
	IP2C::deinstantiate();
}

void TestIP2CAutoUpdates::load_data()
{
	QTest::addColumn<bool>("generalAllowed");
	QTest::addColumn<bool>("automaticAllowed");
	QTest::addColumn<bool>("autoUpdateSetting");
	QTest::addColumn<bool>("hasDatabase");
	QTest::newRow("general-denied-no-database") << false << true << true << false;
	QTest::newRow("general-denied-local-database") << false << true << true << true;
	QTest::newRow("automatic-denied-no-database") << true << false << true << false;
	QTest::newRow("automatic-denied-local-database") << true << false << true << true;
	QTest::newRow("setting-off-no-database") << true << true << false << false;
	QTest::newRow("setting-off-local-database") << true << true << false << true;
}

void TestIP2CAutoUpdates::load()
{
	// Given: automatic updates are disabled by the user or the policy, with or
	// without an installed database. A saved preference must not override the
	// policy.
	QFETCH(bool, generalAllowed);
	QFETCH(bool, automaticAllowed);
	QFETCH(bool, autoUpdateSetting);
	QFETCH(bool, hasDatabase);
	ScopedDoomseekerPolicy policy(DoomseekerPolicy::Builder()
		.setIP2CUpdateAllowed(generalAllowed)
		.setIP2CAutoUpdateAllowed(automaticAllowed).build());
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

void TestIP2CAutoUpdates::configuration_data()
{
	QTest::addColumn<bool>("generalAllowed");
	QTest::addColumn<bool>("automaticAllowed");
	QTest::addColumn<bool>("expectedAutomatic");
	QTest::newRow("deny-all") << false << false << false;
	QTest::newRow("general-denied") << false << true << false;
	QTest::newRow("manual-only") << true << false << false;
	QTest::newRow("automatic-permitted") << true << true << true;
}

void TestIP2CAutoUpdates::configuration()
{
	// Given: explicit permissions, a saved automatic-update preference, and the
	// IP2C settings page, constructed only after installing the policy.
	QFETCH(bool, generalAllowed);
	QFETCH(bool, automaticAllowed);
	QFETCH(bool, expectedAutomatic);
	ScopedDoomseekerPolicy policy(DoomseekerPolicy::Builder()
		.setIP2CUpdateAllowed(generalAllowed)
		.setIP2CAutoUpdateAllowed(automaticAllowed).build());
	QScopedValueRollback<bool> autoUpdate(gConfig.doomseeker.bIP2CountryAutoUpdate, true);
	QScopedValueRollback<bool> honorCountries(gConfig.doomseeker.bHonorServerCountries, true);
	CFGIP2C page(nullptr);

	// When: the settings are read.
	page.read();
	auto *automatic = page.findChild<QCheckBox *>("cbAutoUpdate");
	auto *honor = page.findChild<QCheckBox *>("cbHonorServerCountries");

	// Then: automatic updates are offered only when the policy allows them.
	QVERIFY(automatic != nullptr);
	QVERIFY(honor != nullptr);
	QCOMPARE(!automatic->isHidden(), expectedAutomatic);
	QCOMPARE(automatic->isEnabled(), expectedAutomatic);
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

void TestIP2CAutoUpdates::manualUpdateCheck_data()
{
	QTest::addColumn<bool>("generalAllowed");
	QTest::addColumn<bool>("hasDatabase");
	QTest::addColumn<bool>("expectedRequest");
	QTest::addColumn<int>("expectedStatus");
	QTest::newRow("denied-no-database")
		<< false << false << false << int(IP2CUpdater::UpdateCheckError);
	QTest::newRow("denied-local-database")
		<< false << true << false << int(IP2CUpdater::UpdateCheckError);
	QTest::newRow("allowed-no-database")
		<< true << false << false << int(IP2CUpdater::UpdateNeeded);
	// No synchronous status is expected while the proxy holds the request.
	QTest::newRow("allowed-local-database") << true << true << true << -1;
}

void TestIP2CAutoUpdates::manualUpdateCheck()
{
	// Given: an updater used by the manual update dialog, with or without an
	// installed database. Service restrictions must also cover checksum checks.
	QFETCH(bool, generalAllowed);
	QFETCH(bool, hasDatabase);
	QFETCH(bool, expectedRequest);
	QFETCH(int, expectedStatus);
	ScopedDoomseekerPolicy policy(DoomseekerPolicy::Builder()
		.setIP2CUpdateAllowed(generalAllowed).build());
	if (hasDatabase)
	{
		QFile file(DoomseekerFilePaths::ip2cDatabase());
		QVERIFY(file.open(QIODevice::WriteOnly));
		const QByteArray data = databaseFixture();
		QCOMPARE(file.write(data), qint64(data.size()));
	}
	IP2CUpdater updater;
	QSignalSpy checked(&updater, SIGNAL(updateCheckFinished(int)));

	// When: the user explicitly checks whether an update is needed.
	updater.startUpdateCheck(DoomseekerFilePaths::ip2cDatabase());

	// Then: an allowed checksum request stays isolated at the local proxy until
	// the updater is destroyed, before the scoped policy is restored. Other
	// checks complete locally with the scenario's expected result.
	if (expectedRequest)
	{
		QVERIFY(updater.isWorking());
		QCOMPARE(checked.count(), 0);
		QTRY_VERIFY(proxy.hasPendingConnections());
	}
	else
	{
		QCOMPARE(checked.count(), 1);
		QCOMPARE(checked.at(0).at(0).toInt(), expectedStatus);
		QVERIFY(!updater.isWorking());
		QVERIFY(!proxy.hasPendingConnections());
	}
}

void TestIP2CAutoUpdates::manualDownload_data()
{
	QTest::addColumn<bool>("generalAllowed");
	QTest::newRow("denied") << false;
	QTest::newRow("manual-only") << true;
}

void TestIP2CAutoUpdates::manualDownload()
{
	// Given: explicit manual-update permission, with automatic updates disabled,
	// and an updater for a user-requested database download.
	QFETCH(bool, generalAllowed);
	ScopedDoomseekerPolicy policy(DoomseekerPolicy::Builder()
		.setIP2CUpdateAllowed(generalAllowed).build());
	IP2CUpdater updater;
	QSignalSpy downloaded(&updater, SIGNAL(databaseDownloadFinished(QByteArray)));

	// When: a manual download is started.
	updater.downloadDatabase(DoomseekerFilePaths::ip2cDatabase());

	// Then: the blanket service policy, not the automatic-update option,
	// determines whether a manual download can start.
	if (generalAllowed)
	{
		QVERIFY(updater.isWorking());
		QCOMPARE(downloaded.count(), 0);
		QTRY_VERIFY(proxy.hasPendingConnections());
	}
	else
	{
		QVERIFY(!updater.isWorking());
		QCOMPARE(downloaded.count(), 1);
		QVERIFY(downloaded.at(0).at(0).toByteArray().isEmpty());
		QVERIFY(!proxy.hasPendingConnections());
	}
}

void TestIP2CAutoUpdates::manualLoaderUpdate_data()
{
	manualDownload_data();
}

void TestIP2CAutoUpdates::manualLoaderUpdate()
{
	// Given: explicit manual-update permission and the loader used to install a
	// manually requested database update. It is destroyed before policy restore.
	QFETCH(bool, generalAllowed);
	ScopedDoomseekerPolicy policy(DoomseekerPolicy::Builder()
		.setIP2CUpdateAllowed(generalAllowed).build());
	IP2C &ip2c = *IP2C::instance();
	IP2CLoader loader(ip2c);
	QSignalSpy finished(&loader, SIGNAL(finished()));

	// When: a manual update is requested directly, bypassing the menu action.
	loader.update();

	// Then: an allowed download starts, while a refused download completes
	// without leaving lookups locked or the loader waiting for a network reply.
	if (generalAllowed)
	{
		QCOMPARE(finished.count(), 0);
		QVERIFY(ip2c.isDataAccessLocked());
		QTRY_VERIFY(proxy.hasPendingConnections());
	}
	else
	{
		QCOMPARE(finished.count(), 1);
		QVERIFY(!ip2c.isDataAccessLocked());
		QVERIFY(!proxy.hasPendingConnections());
	}
}

QTEST_MAIN(TestIP2CAutoUpdates)

#include "tst_ip2cautoupdates.moc"
