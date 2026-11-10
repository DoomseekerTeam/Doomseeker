//------------------------------------------------------------------------------
// tst_disabledupdates.cpp
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
#include "doomseeker_copts.h"
#include "updater/autoupdater.h"
#include "updater/updatechannel.h"
#include "updater/updateinstaller.h"

#include <QtTest>

#ifdef DOOMSEEKER_WITH_AUTOUPDATES
#error "These tests require automatic updates to be disabled."
#endif

class TestDisabledUpdates : public QObject
{
	Q_OBJECT

private slots:
	void updateCheck_data();
	void updateCheck();
	void pendingInstallation();
};

void TestDisabledUpdates::updateCheck_data()
{
	QTest::addColumn<QString>("channelName");
	QTest::newRow("no-channel") << QString();
	QTest::newRow("stable") << UpdateChannel::mkStable().name();
	QTest::newRow("beta") << UpdateChannel::mkBeta().name();
}

void TestDisabledUpdates::updateCheck()
{
	// Given: an updater in a build without automatic updates, even with a
	// valid channel. No application data paths are initialized: disabled
	// updates must not access update storage or proceed to a network request.
	QFETCH(QString, channelName);
	AutoUpdater updater;
	updater.setChannel(UpdateChannel::fromName(channelName));
	QSignalSpy finished(&updater, SIGNAL(finished()));
	QSignalSpy status(&updater, SIGNAL(statusMessage(QString)));
	QSignalSpy progress(&updater, SIGNAL(packageDownloadProgress(qint64,qint64)));
	QSignalSpy confirmation(&updater, SIGNAL(downloadAndInstallConfirmationRequested()));

	// When: a caller tries to start an update check directly.
	updater.start();

	// Then: the check is synchronously aborted before starting any work.
	QCOMPARE(finished.count(), 1);
	QCOMPARE(updater.errorCode(), AutoUpdater::EC_Aborted);
	QVERIFY(!updater.isRunning());
	QVERIFY(updater.downloadedPackagesFilenames().isEmpty());
	QCOMPARE(status.count(), 0);
	QCOMPARE(progress.count(), 0);
	QCOMPARE(confirmation.count(), 0);
}

void TestDisabledUpdates::pendingInstallation()
{
	// Given: a disabled installer with no application data paths initialized.
	// It must not even inspect storage for pending updates from earlier builds.
	UpdateInstaller installer;

	// When: a caller tries to install updates directly.
	const auto result = installer.startInstallation();

	// Then: no installation is attempted and update storage is not accessed.
	QCOMPARE(result, UpdateInstaller::EC_NothingToUpdate);
}

QTEST_GUILESS_MAIN(TestDisabledUpdates)

#include "tst_disabledupdates.moc"
