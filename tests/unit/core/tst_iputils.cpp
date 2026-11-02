//------------------------------------------------------------------------------
// tst_iputils.cpp
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
#include "iputils.h"

#include <QHostAddress>
#include <QtTest>

class TestIpUtils : public QObject
{
	Q_OBJECT

private slots:
	void isIPv4Mapped_data();
	void isIPv4Mapped();
};

void TestIpUtils::isIPv4Mapped_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<bool>("expected");

	QTest::newRow("null") << QString() << false;
	QTest::newRow("ipv4") << "192.0.2.1" << false;
	QTest::newRow("ipv4-zero") << "0.0.0.0" << false;
	QTest::newRow("ipv4-loopback") << "127.0.0.1" << false;
	QTest::newRow("mapped") << "::ffff:192.0.2.1" << true;
	QTest::newRow("mapped-hex") << "::ffff:c000:201" << true;
	QTest::newRow("mapped-zero") << "::ffff:0.0.0.0" << true;
	QTest::newRow("mapped-loopback") << "::ffff:127.0.0.1" << true;
	QTest::newRow("mapped-broadcast") << "::ffff:255.255.255.255" << true;
	QTest::newRow("ipv6") << "2001:db8::1" << false;
	QTest::newRow("ipv6-unspecified") << "::" << false;
	QTest::newRow("ipv6-loopback") << "::1" << false;
	QTest::newRow("ipv4-compatible") << "::192.0.2.1" << false;
	QTest::newRow("ipv4-translated") << "::ffff:0:192.0.2.1" << false;
	QTest::newRow("nat64") << "64:ff9b::192.0.2.1" << false;
	QTest::newRow("scoped-ipv6") << "fe80::1%eth0" << false;
	QTest::newRow("wrong-prefix-start") << "1::ffff:192.0.2.1" << false;
	QTest::newRow("wrong-prefix-marker") << "::fffe:192.0.2.1" << false;
}

void TestIpUtils::isIPv4Mapped()
{
	// Given: a null, native IPv4, mapped IPv6, or genuine IPv6 address.
	QFETCH(QString, input);
	QFETCH(bool, expected);
	const QHostAddress address(input);

	// And sanity-check: all nonempty fixtures are valid addresses.
	if (!input.isEmpty())
		QVERIFY(!address.isNull());

	// When: checking for the IPv4-mapped IPv6 prefix.
	const bool result = IpUtils::isIPv4Mapped(address);

	// Then: only IPv6 addresses with the complete mapped prefix match.
	QCOMPARE(result, expected);
}

QTEST_GUILESS_MAIN(TestIpUtils)

#include "tst_iputils.moc"
