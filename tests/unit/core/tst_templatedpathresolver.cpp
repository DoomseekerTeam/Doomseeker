//------------------------------------------------------------------------------
// tst_templatedpathresolver.cpp
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "templatedpathresolver.h"
#include "strings.hpp"

#include <QtTest>
#include <QDir>
#include <QMap>
#include <QProcessEnvironment>

#ifdef DOOMSEEKER_TEMPLATED_PATH_RESOLVER_TILDEUSER
#include <sys/types.h>
#include <pwd.h>
#endif

class TestTemplatedPathResolver : public QObject
{
	Q_OBJECT

private slots:
	void init();
	void cleanup();
	void resolve_data();
	void resolve();
#ifdef DOOMSEEKER_TEMPLATED_PATH_RESOLVER_TILDEUSER
	void namedUser_data();
	void namedUser();
	void unknownUser_data();
	void unknownUser();
#endif

private:
	const QList<QByteArray> variables = {
		"PROGDIR", "PROGDIRECTORY", "G00dV4r", "an0ther", "badvar44"
	};
	QMap<QByteArray, QByteArray> savedEnvironment;
};

void TestTemplatedPathResolver::init()
{
	savedEnvironment.clear();
	const auto environment = QProcessEnvironment::systemEnvironment();
	for (const QByteArray &name : variables)
	{
		if (environment.contains(QString::fromLatin1(name)))
			savedEnvironment.insert(name, qgetenv(name.constData()));
	}
	QVERIFY(qputenv("PROGDIR", "this env. var should be ignored"));
	QVERIFY(qputenv("PROGDIRECTORY", "some/folder"));
	QVERIFY(qputenv("G00dV4r", "/data/is/mine"));
	QVERIFY(qputenv("an0ther", "ok"));
	QVERIFY(qunsetenv("badvar44"));
}

void TestTemplatedPathResolver::cleanup()
{
	for (const QByteArray &name : variables)
	{
		if (savedEnvironment.contains(name))
			qputenv(name.constData(), savedEnvironment.value(name));
		else
			qunsetenv(name.constData());
	}
}

void TestTemplatedPathResolver::resolve_data()
{
	QTest::addColumn<bool>("envEnabled");
	QTest::addColumn<bool>("progdirEnabled");
	QTest::addColumn<bool>("homeEnabled");
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("expected");

	const QString progdir = QCoreApplication::applicationDirPath();
	const QString logs = Strings::combinePaths(progdir, "logs");
	QTest::newRow("progdir") << true << true << true << "$PROGDIR" << progdir;
	QTest::newRow("progdir-suffixed") << true << true << true << "$PROGDIR/logs" << logs;
	QTest::newRow("progdir-in-middle") << true << true << true
		<< "/home/alice/$PROGDIR/logs" << "/home/alice/" + logs;
	QTest::newRow("home") << true << true << true << "~" << QDir::homePath();
	QTest::newRow("complex") << true << true << true
		<< "~/logs/$an0ther/$G00dV4r/$an0ther/$badvar44/done"
		<< QDir::homePath() + "/logs/ok//data/is/mine/ok//done";
	QTest::newRow("similar-variable-name") << true << true << true
		<< "$PROGDIRECTORY/logs" << "some/folder/logs";
	QTest::newRow("env-disabled") << false << true << true
		<< "$PROGDIRECTORY/logs" << "$PROGDIRECTORY/logs";
	QTest::newRow("env-disabled-progdir-enabled") << false << true << true
		<< "$PROGDIR" << progdir;
	QTest::newRow("progdir-disabled-other-variable") << true << false << true
		<< "$PROGDIRECTORY/logs" << "some/folder/logs";
	QTest::newRow("progdir-disabled-falls-back-to-env") << true << false << true
		<< "$PROGDIR" << "this env. var should be ignored";
	QTest::newRow("both-disabled-other-variable") << false << false << true
		<< "$PROGDIRECTORY/logs" << "$PROGDIRECTORY/logs";
	QTest::newRow("both-disabled-progdir") << false << false << true
		<< "$PROGDIR" << "$PROGDIR";
	QTest::newRow("home-disabled") << true << true << false << "~" << "~";
	QTest::newRow("named-home-disabled") << true << true << false << "~root" << "~root";
	QTest::newRow("all-disabled") << false << false << false
		<< "~/$PROGDIR/$PROGDIRECTORY/log.txt" << "~/$PROGDIR/$PROGDIRECTORY/log.txt";
#ifndef DOOMSEEKER_TEMPLATED_PATH_RESOLVER_TILDEUSER
	QTest::newRow("named-home-unsupported") << true << true << true
		<< "~guest/stuff" << "~guest/stuff";
#endif
}

void TestTemplatedPathResolver::resolve()
{
	// Given: controlled environment variables and the selected resolver features.
	QFETCH(bool, envEnabled);
	QFETCH(bool, progdirEnabled);
	QFETCH(bool, homeEnabled);
	QFETCH(QString, input);
	QFETCH(QString, expected);
	TemplatedPathResolver resolver;
	resolver.setEnvVarsEnabled(envEnabled);
	resolver.setProgdirEnabled(progdirEnabled);
	resolver.setUserHomeEnabled(homeEnabled);

	// When: expanding the templated path.
	const QString result = resolver.resolve(input);

	// Then: only enabled placeholders are expanded, with PROGDIR taking precedence.
	QCOMPARE(result, expected);
}

#ifdef DOOMSEEKER_TEMPLATED_PATH_RESOLVER_TILDEUSER
void TestTemplatedPathResolver::namedUser_data()
{
	QTest::addColumn<QString>("suffix");
	QTest::newRow("plain") << "";
	QTest::newRow("trailing-separator") << "/";
	QTest::newRow("subdirectory") << "/extra/more";
}

void TestTemplatedPathResolver::namedUser()
{
	// Given: a named account supported by the host's user database.
	QFETCH(QString, suffix);
	const struct passwd *entry = getpwnam("root");
	if (entry == nullptr)
		QSKIP("The host has no root account");
	const QString home = QString::fromLocal8Bit(entry->pw_dir);
	TemplatedPathResolver resolver;
	resolver.setUserHomeEnabled(true);

	// When: resolving the account's home with an optional suffix.
	const QString result = resolver.resolve("~root" + suffix);

	// Then: the home directory is expanded and the suffix is preserved.
	QCOMPARE(result, home + suffix);
}

void TestTemplatedPathResolver::unknownUser_data()
{
	QTest::addColumn<QString>("suffix");
	QTest::newRow("plain") << "";
	QTest::newRow("trailing-separator") << "/";
	QTest::newRow("subdirectory") << "/extra";
}

void TestTemplatedPathResolver::unknownUser()
{
	// Given: a username confirmed absent from the host's user database.
	QFETCH(QString, suffix);
	QString username = "doomseeker_no_user";
	while (getpwnam(username.toLatin1().constData()) != nullptr)
		username += "1";
	const QString input = "~" + username + suffix;
	TemplatedPathResolver resolver;
	resolver.setUserHomeEnabled(true);

	// When: attempting to resolve the absent user's home.
	const QString result = resolver.resolve(input);

	// Then: an unknown user is left unchanged.
	QCOMPARE(result, input);
}
#endif

QTEST_GUILESS_MAIN(TestTemplatedPathResolver)

#include "tst_templatedpathresolver.moc"
