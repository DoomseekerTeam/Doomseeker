//------------------------------------------------------------------------------
// tst_odamexmasterclient.cpp
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
#include "odamexmasterclient.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "serverapi/server.h"
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QtTest>

namespace
{
// Challenge, count 2, then 192.0.2.1:10666 and 198.51.100.2:10667.
const QByteArray twoServers = QByteArray::fromHex("a3db0b000200c0000201aa29c6336402ab29");
}

class TestOdamexMasterClient : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void parseMasterList_data();
	void parseMasterList();
	void rejectMalformedMasterList_data();
	void rejectMalformedMasterList();
};

void TestOdamexMasterClient::initTestCase()
{
	QStandardPaths::setTestModeEnabled(true);
	DataPaths::initDefault(false);
}

void TestOdamexMasterClient::parseMasterList_data()
{
	QTest::addColumn<QByteArray>("trailingData");
	QTest::newRow("exact-length") << QByteArray();
	QTest::newRow("trailing-endpoint") << QByteArray::fromHex("cb007101ac29");
}

void TestOdamexMasterClient::parseMasterList()
{
	// Given: a complete two-endpoint list, possibly followed by extra bytes.
	QFETCH(QByteArray, trailingData);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexMasterClient master;
	QSignalSpy updates(&master, &MasterClient::listUpdated);

	// When: the master receives the list.
	const auto response = master.readResponse(twoServers + trailingData);

	// Then: only the advertised IPv4 addresses and ports are registered, in order.
	QCOMPARE(response, MasterClient::RESPONSE_GOOD);
	QCOMPARE(updates.size(), 1);
	QCOMPARE(master.numServers(), 2);
	QCOMPARE(master[0]->address(), QHostAddress("192.0.2.1"));
	QCOMPARE(master[0]->port(), quint16(10666));
	QCOMPARE(master[1]->address(), QHostAddress("198.51.100.2"));
	QCOMPARE(master[1]->port(), quint16(10667));

	// When: receiving a valid empty list.
	const auto emptyResponse = master.readResponse(
		QByteArray::fromHex("a3db0b000000") + trailingData);

	// Then: the empty list is distinguished from a missing count.
	QCOMPARE(emptyResponse, MasterClient::RESPONSE_GOOD);
	QCOMPARE(updates.size(), 2);
	QCOMPARE(master.numServers(), 0);
}

void TestOdamexMasterClient::rejectMalformedMasterList_data()
{
	QTest::addColumn<QByteArray>("data");
	for (int i = 0; i < twoServers.size(); ++i)
		QTest::newRow(qPrintable(QString("prefix-%1").arg(i))) << twoServers.left(i);
	QTest::newRow("wrong-challenge") << QByteArray::fromHex("abcd00000000");
	QTest::newRow("overstated-count") << QByteArray::fromHex("a3db0b000300");
}

void TestOdamexMasterClient::rejectMalformedMasterList()
{
	// Given: a malformed master datagram and a client with a valid list.
	QFETCH(QByteArray, data);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexMasterClient master;

	// And: a previous reply has populated the parser's list.
	QCOMPARE(master.readResponse(twoServers), MasterClient::RESPONSE_GOOD);
	const QList<ServerPtr> previous = master.servers();
	QSignalSpy updates(&master, &MasterClient::listUpdated);

	// When: parsing the malformed datagram directly, without starting a refresh.
	const auto response = master.readResponse(data);

	// Then: the parser neither modifies its list nor announces a successful update.
	QCOMPARE(response, MasterClient::RESPONSE_BAD);
	QVERIFY(master.servers() == previous);
	QCOMPARE(updates.size(), 0);
}

QTEST_GUILESS_MAIN(TestOdamexMasterClient)
#include "tst_odamexmasterclient.moc"
