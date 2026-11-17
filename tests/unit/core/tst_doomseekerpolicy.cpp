//------------------------------------------------------------------------------
// tst_doomseekerpolicy.cpp
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
#include "doomseekerpolicy.h"

#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QScopedPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>
#include <cstdio>
#include <cstdlib>
#include <type_traits>

static_assert(std::is_copy_constructible<DoomseekerPolicy>::value, "Policy is a value");
static_assert(std::is_copy_assignable<DoomseekerPolicy>::value, "Policy is assignable");
static_assert(!std::is_copy_constructible<ScopedDoomseekerPolicy>::value, "Binding cannot copy");
static_assert(!std::is_move_constructible<ScopedDoomseekerPolicy>::value, "Binding cannot move");
static_assert(std::is_same<decltype(gPolicy), const DoomseekerPolicy &>::value,
	"Installed policy is read-only");

class TestDoomseekerPolicy : public QObject
{
	Q_OBJECT

private slots:
	void defaults();
	void permissions_data();
	void permissions();
	void builderReuse();
	void binding();
	void missingPolicy_data();
	void missingPolicy();
	void coreNetworkAccess();
};

void TestDoomseekerPolicy::defaults()
{
	// Given: a Builder with no permissions selected.
	const DoomseekerPolicy::Builder builder;

	// When: a policy value is built without any global installation.
	const auto policy = builder.build();

	// Then: all actions are denied.
	QVERIFY(!policy.isAutoUpdateAllowed());
	QVERIFY(!policy.isIP2CUpdateAllowed());
	QVERIFY(!policy.isIP2CAutoUpdateAllowed());
}

void TestDoomseekerPolicy::permissions_data()
{
	QTest::addColumn<bool>("application");
	QTest::addColumn<bool>("general");
	QTest::addColumn<bool>("automatic");
	QTest::addColumn<bool>("expectedAutomatic");
	QTest::newRow("deny-all") << false << false << false << false;
	QTest::newRow("auto-without-general") << false << false << true << false;
	QTest::newRow("manual-only") << false << true << false << false;
	QTest::newRow("ip2c-only") << false << true << true << true;
	QTest::newRow("application-only") << true << false << false << false;
	QTest::newRow("application-and-invalid-auto") << true << false << true << false;
	QTest::newRow("application-and-manual") << true << true << false << false;
	QTest::newRow("allow-all") << true << true << true << true;
}

void TestDoomseekerPolicy::permissions()
{
	// Given: explicit permissions, including application updates on any host.
	QFETCH(bool, application);
	QFETCH(bool, general);
	QFETCH(bool, automatic);
	QFETCH(bool, expectedAutomatic);

	// When: policies are built with both orders of the IP2C setters.
	const auto forward = DoomseekerPolicy::Builder()
		.setAutoUpdateAllowed(application)
		.setIP2CUpdateAllowed(general)
		.setIP2CAutoUpdateAllowed(automatic).build();
	const auto reverse = DoomseekerPolicy::Builder()
		.setIP2CAutoUpdateAllowed(automatic)
		.setIP2CUpdateAllowed(general)
		.setAutoUpdateAllowed(application).build();

	// Then: only automatic IP2C permission is normalized, independent of order.
	for (const auto &policy : {forward, reverse})
	{
		QCOMPARE(policy.isAutoUpdateAllowed(), application);
		QCOMPARE(policy.isIP2CUpdateAllowed(), general);
		QCOMPARE(policy.isIP2CAutoUpdateAllowed(), expectedAutomatic);
	}
}

void TestDoomseekerPolicy::builderReuse()
{
	// Given: a reusable Builder and a value built from its initial state.
	DoomseekerPolicy::Builder builder;
	builder.setAutoUpdateAllowed(true).setIP2CAutoUpdateAllowed(true);
	const auto first = builder.build();

	// When: the Builder changes, builds another value, and changes again.
	const auto second = builder.setAutoUpdateAllowed(false)
		.setIP2CUpdateAllowed(true).build();
	builder.setIP2CAutoUpdateAllowed(false).setIP2CUpdateAllowed(false);

	// Then: prior values retain their independent effective permissions.
	QVERIFY(first.isAutoUpdateAllowed());
	QVERIFY(!first.isIP2CUpdateAllowed());
	QVERIFY(!first.isIP2CAutoUpdateAllowed());
	QVERIFY(!second.isAutoUpdateAllowed());
	QVERIFY(second.isIP2CUpdateAllowed());
	QVERIFY(second.isIP2CAutoUpdateAllowed());
}

void TestDoomseekerPolicy::binding()
{
	// Given: an explicitly installed policy owned by its binding.
	ScopedDoomseekerPolicy outer(DoomseekerPolicy::Builder()
		.setAutoUpdateAllowed(true).build());
	const DoomseekerPolicy *original = &gPolicy;
	QVERIFY(original->isAutoUpdateAllowed());
	QVERIFY(!original->isIP2CUpdateAllowed());

	// When: a nested binding owns a temporary Builder result.
	{
		ScopedDoomseekerPolicy inner(DoomseekerPolicy::Builder()
			.setIP2CUpdateAllowed(true).setIP2CAutoUpdateAllowed(true).build());

		// Then: gPolicy accesses the inner value without mutating the outer one.
		QVERIFY(&gPolicy != original);
		QVERIFY(!gPolicy.isAutoUpdateAllowed());
		QVERIFY(gPolicy.isIP2CUpdateAllowed());
		QVERIFY(gPolicy.isIP2CAutoUpdateAllowed());
		QVERIFY(original->isAutoUpdateAllowed());
		QVERIFY(!original->isIP2CUpdateAllowed());
	}

	// Then: normal LIFO teardown restores the same outer value.
	QCOMPARE(&gPolicy, original);
	QVERIFY(gPolicy.isAutoUpdateAllowed());
	QVERIFY(!gPolicy.isIP2CUpdateAllowed());
}

void TestDoomseekerPolicy::missingPolicy_data()
{
	QTest::addColumn<QString>("probe");
	QTest::newRow("never-installed") << QString("--probe-missing-policy");
	QTest::newRow("binding-destroyed") << QString("--probe-policy-teardown");
}

void TestDoomseekerPolicy::missingPolicy()
{
	// Given: a separate process that either never installs a policy or destroys
	// its last binding. A fatal contract violation must not kill this runner.
	QFETCH(QString, probe);
	QProcess child;
	child.setProgram(QCoreApplication::applicationFilePath());
	child.setArguments(QStringList{probe});

	// When: the child accesses gPolicy without a binding.
	child.start();
	QVERIFY(child.waitForStarted());
	QVERIFY(child.waitForFinished());

	// Then: release and debug builds both fail clearly, never lazily recreating.
	QVERIFY(child.exitStatus() == QProcess::CrashExit || child.exitCode() != 0);
	QVERIFY(child.readAllStandardError().contains(
		"Doomseeker policy accessed without an installed ScopedDoomseekerPolicy"));
}

void TestDoomseekerPolicy::coreNetworkAccess()
{
	// Given: a restricted policy and a local HTTP server unrelated to project
	// services. Avoid the Internet and the machine's proxy settings.
	ScopedDoomseekerPolicy policy(DoomseekerPolicy::Builder().build());
	QTcpServer server;
	QVERIFY(server.listen(QHostAddress::LocalHost));
	connect(&server, &QTcpServer::newConnection, &server, [&server]() {
		QTcpSocket *socket = server.nextPendingConnection();
		connect(socket, &QTcpSocket::readyRead, socket, [socket]() {
			socket->readAll();
			socket->write("HTTP/1.1 200 OK\r\nContent-Length: 4\r\n"
				"Connection: close\r\n\r\ncore");
			socket->disconnectFromHost();
		});
	});
	QNetworkAccessManager network;
	network.setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
	const QUrl url(QString("http://127.0.0.1:%1/").arg(server.serverPort()));

	// When: an ordinary network operation is performed.
	QScopedPointer<QNetworkReply> reply(network.get(QNetworkRequest(url)));
	QSignalSpy finished(reply.data(), SIGNAL(finished()));

	// Then: non-service networking works even with project services disabled.
	QTRY_COMPARE(finished.count(), 1);
	QCOMPARE(reply->error(), QNetworkReply::NoError);
	QCOMPARE(reply->readAll(), QByteArray("core"));
}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	const QStringList arguments = app.arguments();
	if (arguments.contains("--probe-policy-teardown"))
	{
		ScopedDoomseekerPolicy policy(DoomseekerPolicy::Builder().build());
	}
	if (arguments.contains("--probe-missing-policy")
		|| arguments.contains("--probe-policy-teardown"))
	{
		// Turn Qt's fatal diagnostic into a failing process exit without crash
		// dialogs or core dumps. Warnings must not make the probe succeed.
		qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &, const QString &message) {
			const QByteArray text = message.toLocal8Bit();
			std::fprintf(stderr, "%s\n", text.constData());
			std::fflush(stderr);
			if (type == QtFatalMsg)
				std::_Exit(EXIT_FAILURE);
		});
		(void)gPolicy;
		return 0;
	}
	TestDoomseekerPolicy test;
	return QTest::qExec(&test, argc, argv);
}

#include "tst_doomseekerpolicy.moc"
