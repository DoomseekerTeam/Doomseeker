//------------------------------------------------------------------------------
// tst_packetdumper.cpp
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
#include "log.h"
#include "refresher/packetdumper.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <QtTest>

#if QT_VERSION < 0x060000
Q_DECLARE_METATYPE(QHostAddress)
#endif
Q_DECLARE_METATYPE(PacketDumper::Direction)

class TestPacketDumper : public QObject
{
	Q_OBJECT

private slots:
	void defaultDirectory();
	void dump_data();
	void dump();
	void repeatedCaptures();
	void createsDirectory_data();
	void createsDirectory();
	void invalidDirectory();
};

void TestPacketDumper::defaultDirectory()
{
	QCOMPARE(PacketDumper::defaultDirectory(),
		QDir(QDir::tempPath()).filePath("doomseeker-packet-dumps"));
}

void TestPacketDumper::dump_data()
{
	QTest::addColumn<QHostAddress>("address");
	QTest::addColumn<quint16>("port");
	QTest::addColumn<QString>("label");
	QTest::addColumn<QString>("prefix");
	QTest::addColumn<QByteArray>("data");
	QTest::addColumn<PacketDumper::Direction>("direction");

	const QByteArray binary = QByteArray::fromHex("032003ad00010200ff0d0a");
	const auto recv = PacketDumper::Direction::Recv;
	const auto sent = PacketDumper::Direction::Sent;
	QTest::newRow("ipv4-binary") << QHostAddress("192.0.2.1") << quint16(10666)
		<< "raw" << "192.0.2.1_10666_recv_raw_" << binary << recv;
	QTest::newRow("ipv4-mapped-recv") << QHostAddress("::ffff:192.0.2.1") << quint16(10666)
		<< "raw" << "192.0.2.1_10666_recv_raw_" << binary << recv;
	QTest::newRow("ipv4-mapped-sent") << QHostAddress("::ffff:192.0.2.1") << quint16(10666)
		<< "raw" << "192.0.2.1_10666_sent_raw_" << binary << sent;
	QTest::newRow("ipv4-mapped-hex") << QHostAddress("::ffff:c000:201") << quint16(10666)
		<< "raw" << "192.0.2.1_10666_recv_raw_" << binary << recv;
	QTest::newRow("ipv4-mapped-zero") << QHostAddress("::ffff:0.0.0.0") << quint16(10666)
		<< "raw" << "0.0.0.0_10666_recv_raw_" << binary << recv;
	QTest::newRow("ipv6-loopback") << QHostAddress("::1") << quint16(15000)
		<< "raw" << "__1_15000_recv_raw_" << binary << recv;
	QTest::newRow("ipv6-unspecified") << QHostAddress("::") << quint16(15000)
		<< "raw" << "___15000_recv_raw_" << binary << recv;
	QTest::newRow("ipv6") << QHostAddress("2001:db8::1") << quint16(15000)
		<< "raw" << "2001_db8__1_15000_recv_raw_" << binary << recv;
	QTest::newRow("ipv6-scope") << QHostAddress("fe80::1%eth0") << quint16(10666)
		<< "raw" << "fe80__1_eth0_10666_recv_raw_" << binary << recv;
	QTest::newRow("plugin-decoded") << QHostAddress("192.0.2.1") << quint16(10666)
		<< "odamex-decoded" << "192.0.2.1_10666_recv_odamex-decoded_" << binary << recv;
	QTest::newRow("unsafe-label") << QHostAddress("192.0.2.1") << quint16(10666)
		<< "../decoded:stage\\part" << "192.0.2.1_10666_recv_.._decoded_stage_part_"
		<< binary << recv;
	QTest::newRow("empty-payload") << QHostAddress("192.0.2.1") << quint16(10666)
		<< "raw" << "192.0.2.1_10666_recv_raw_" << QByteArray() << recv;
	QTest::newRow("large-payload") << QHostAddress("192.0.2.1") << quint16(10666)
		<< "raw" << "192.0.2.1_10666_recv_raw_" << QByteArray(65507, char(0xff)) << recv;
	QTest::newRow("sent-ipv4") << QHostAddress("192.0.2.1") << quint16(10666)
		<< "raw" << "192.0.2.1_10666_sent_raw_" << binary << sent;
	QTest::newRow("sent-ipv6") << QHostAddress("2001:db8::1") << quint16(15000)
		<< "raw" << "2001_db8__1_15000_sent_raw_" << binary << sent;
}

void TestPacketDumper::dump()
{
	// Given: a peer, direction, payload, label, and an isolated capture directory.
	QFETCH(QHostAddress, address);
	QFETCH(quint16, port);
	QFETCH(QString, label);
	QFETCH(QString, prefix);
	QFETCH(QByteArray, data);
	QFETCH(PacketDumper::Direction, direction);
	QTemporaryDir directory;
	QVERIFY(directory.isValid());
	QSignalSpy logEntries(&gLog, SIGNAL(newEntry(QString)));
	PacketDumper::Options options;
	options.label = label;
	options.directory = directory.path();
	options.timestamp = QDateTime::fromString("2026-01-02T15:23:05.006+02:00", Qt::ISODateWithMs);
	QVERIFY(options.timestamp.isValid());

	// When: the dumper saves the supplied bytes with an explicit time instant.
	const QString path = PacketDumper::dump(address, port, direction, data, options);

	// Then: the file identifies the peer and direction and contains only those bytes.
	QVERIFY(!path.isEmpty());
	const QFileInfo info(path);
	QCOMPARE(info.absolutePath(), directory.path());
	QVERIFY(info.fileName().startsWith(prefix + "20260102T132305006Z_"));
	QVERIFY(info.fileName().endsWith(".bin"));
	QVERIFY(QRegularExpression("^[A-Za-z0-9_.-]+$").match(info.fileName()).hasMatch());
	QVERIFY(QRegularExpression("_[0-9]{8}T[0-9]{9}Z_[A-Za-z0-9]+\\.bin$")
		.match(info.fileName()).hasMatch());
	QFile file(path);
	QVERIFY(file.open(QIODevice::ReadOnly));
	QCOMPARE(file.readAll(), data);
	QCOMPARE(logEntries.count(), 1);
	QVERIFY(logEntries.first().first().toString().contains(path));
}

void TestPacketDumper::repeatedCaptures()
{
	// Given: several payloads from the same sender with an identical fixed timestamp.
	QTemporaryDir directory;
	QVERIFY(directory.isValid());
	const QHostAddress address("192.0.2.1");
	PacketDumper::Options options;
	options.directory = directory.path();
	options.timestamp = QDateTime::fromString("2026-01-02T15:23:05.006Z", Qt::ISODateWithMs);
	QVERIFY(options.timestamp.isValid());
	QStringList paths;
	QList<QByteArray> payloads;
	for (int i = 0; i < 16; ++i)
		payloads << QByteArray::number(i);

	// When: every datagram is saved using the same timestamp and filename template.
	for (const QByteArray &payload : payloads)
		paths << PacketDumper::dump(address, 10666, PacketDumper::Direction::Recv,
			payload, options);

	// Then: every capture has a distinct filename and all earlier bytes remain intact.
	QSet<QString> uniquePaths;
	for (int i = 0; i < paths.size(); ++i)
	{
		QVERIFY(!paths[i].isEmpty());
		QVERIFY(QFileInfo(paths[i]).fileName().startsWith(
			"192.0.2.1_10666_recv_raw_20260102T152305006Z_"));
		uniquePaths.insert(paths[i]);
		QFile file(paths[i]);
		QVERIFY(file.open(QIODevice::ReadOnly));
		QCOMPARE(file.readAll(), payloads[i]);
	}
	QCOMPARE(uniquePaths.size(), paths.size());
	QCOMPARE(QDir(directory.path()).entryList(QDir::Files).size(), paths.size());
}

void TestPacketDumper::createsDirectory_data()
{
	QTest::addColumn<QString>("subdirectory");
	QTest::newRow("odamex") << "p-odamex";
	QTest::newRow("zandronum") << "p-zandronum";
	QTest::newRow("unknown-plugin") << "doomseeker";
	QTest::newRow("nested") << "new/captures";
}

void TestPacketDumper::createsDirectory()
{
	// Given: a plugin, fallback, or nested capture directory that does not exist.
	QFETCH(QString, subdirectory);
	QTemporaryDir parent;
	QVERIFY(parent.isValid());
	const QString directory = QDir(parent.path()).filePath(subdirectory);
	QVERIFY(!QDir(directory).exists());
	const QByteArray payload("reply");
	PacketDumper::Options options;
	options.directory = directory;

	// When: the dumper saves a payload there, with the default label and timestamp.
	const QString path = PacketDumper::dump(QHostAddress("192.0.2.1"),
		10666, PacketDumper::Direction::Recv, payload, options);

	// Then: the directory is created and contains the complete capture.
	QVERIFY(!path.isEmpty());
	QCOMPARE(QFileInfo(path).absolutePath(), directory);
	QVERIFY(QFileInfo(path).fileName().startsWith("192.0.2.1_10666_recv_raw_"));
	QVERIFY(QRegularExpression("_[0-9]{8}T[0-9]{9}Z_[A-Za-z0-9]+\\.bin$")
		.match(QFileInfo(path).fileName()).hasMatch());
	QFile file(path);
	QVERIFY(file.open(QIODevice::ReadOnly));
	QCOMPARE(file.readAll(), payload);
}

void TestPacketDumper::invalidDirectory()
{
	// Given: a regular file in place of a required parent directory.
	QTemporaryDir directory;
	QVERIFY(directory.isValid());
	const QString blockerPath = QDir(directory.path()).filePath("not-a-directory");
	QFile blocker(blockerPath);
	QVERIFY(blocker.open(QIODevice::WriteOnly));
	QCOMPARE(blocker.write("keep"), qint64(4));
	blocker.close();
	QSignalSpy logEntries(&gLog, SIGNAL(newEntry(QString)));
	PacketDumper::Options options;
	options.directory = blockerPath + "/captures";

	// When: the dumper attempts to create a capture below the regular file.
	const QString path = PacketDumper::dump(QHostAddress("192.0.2.1"),
		10666, PacketDumper::Direction::Recv, QByteArray("reply"), options);

	// Then: failure is reported without changing the existing file.
	QVERIFY(path.isEmpty());
	QVERIFY(blocker.open(QIODevice::ReadOnly));
	QCOMPARE(blocker.readAll(), QByteArray("keep"));
	QCOMPARE(QDir(directory.path()).entryList(QDir::Files).size(), 1);
	QCOMPARE(logEntries.count(), 1);
	QVERIFY(logEntries.first().first().toString().contains("cannot create directory"));
}

QTEST_GUILESS_MAIN(TestPacketDumper)

#include "tst_packetdumper.moc"
