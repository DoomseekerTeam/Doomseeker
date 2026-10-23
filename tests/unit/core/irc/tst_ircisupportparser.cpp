//------------------------------------------------------------------------------
// tst_ircisupportparser.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "irc/entities/ircuserprefix.h"
#include "irc/ircisupportparser.h"

#include <QtTest>

class TestIRCISupportParser : public QObject
{
	Q_OBJECT

private slots:
	void userPrefixes_data();
	void userPrefixes();
};

void TestIRCISupportParser::userPrefixes_data()
{
	QTest::addColumn<QStringList>("lines");
	QTest::addColumn<QByteArray>("expectedModes");
	QTest::addColumn<QByteArray>("expectedPrefixes");

	QTest::newRow("explicit-prefixes") << QStringList{
		"AWAYLEN=300 CALLERID=g CASEMAPPING=rfc1459 "
		"CHANMODES=IXZbegw,k,FHJLdfjl,ABCDKMNOPQRSTcimnprstuz CHANNELLEN=64 "
		"CHANTYPES=# CHARSET=ascii ELIST=MU ESILENCE EXCEPTS=e "
		"EXTBAN=,ABCNOQRSTUcjmprsz FNC INVEX=I :are supported by this server",
		"KICKLEN=420 MAP MAXBANS=60 MAXCHANNELS=75 MAXPARA=32 "
		"MAXTARGETS=20 MODES=20 NAMESX NETWORK=ExampleNetwork NICKLEN=20 OPERLOG "
		"OVERRIDE PREFIX=(ohav)@%^+ :are supported by this server",
		"REMOVE SECURELIST SILENCE=32 SSL=109.74.206.218:6697 "
		"STARTTLS STATUSMSG=@%+ TOPICLEN=420 UHNAMES USERIP VBANLIST WALLCHOPS "
		"WALLVOICES WATCH=64 :are supported by this server"
	} << QByteArray("ohav") << QByteArray("@%^+");
	QTest::newRow("default-prefixes")
		<< QStringList{"AWAYLEN=300 :are supported by this server"}
		<< QByteArray("oh\0v", 4) << QByteArray("@%\0+", 4);
}

void TestIRCISupportParser::userPrefixes()
{
	// Given: ISUPPORT lines with an explicit PREFIX or no PREFIX at all.
	QFETCH(QStringList, lines);
	QFETCH(QByteArray, expectedModes);
	QFETCH(QByteArray, expectedPrefixes);
	IRCISupportParser parser;
	for (const QString &line : lines)
		parser.appendLine(line);

	// When: parsing the server's supported features.
	parser.parse();
	const IRCUserPrefix prefixes = parser.userPrefixes();

	// Then: prefix-to-mode and mode-to-prefix mappings match the server or defaults.
	const QByteArray queriedPrefixes("@%^+");
	const QByteArray queriedModes("ohav");
	for (int i = 0; i < 4; ++i)
	{
		QCOMPARE(prefixes.modeForPrefix(queriedPrefixes[i]), expectedModes[i]);
		QCOMPARE(prefixes.prefixForMode(queriedModes[i]), expectedPrefixes[i]);
	}
}

QTEST_GUILESS_MAIN(TestIRCISupportParser)

#include "tst_ircisupportparser.moc"
