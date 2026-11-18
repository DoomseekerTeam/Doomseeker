//------------------------------------------------------------------------------
// tst_odamexcvar.cpp
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
#include "odamexcvar.h"

#include "datastreamoperatorwrapper.h"
#include <QBuffer>
#include <QDataStream>
#include <QVariant>
#include <QtTest>

using OdamexCvar::CvarType;
using OdamexCvar::IntegerInterpretation;

class TestOdamexCvar : public QObject
{
	Q_OBJECT

private slots:
	void integerInterpretation_data();
	void integerInterpretation();
	void readValue_data();
	void readValue();
	void convertIntegerValue_data();
	void convertIntegerValue();
};

void TestOdamexCvar::integerInterpretation_data()
{
	QTest::addColumn<QString>("name");
	QTest::addColumn<int>("wireType");
	QTest::addColumn<bool>("unsignedValue");

	QTest::newRow("score-byte") << QString("sv_scorelimit") << 2 << true;
	QTest::newRow("score-word") << QString("sv_scorelimit") << 3 << false;
	QTest::newRow("score-integer") << QString("sv_scorelimit") << 4 << false;
	QTest::newRow("score-float") << QString("sv_scorelimit") << 5 << false;
	QTest::newRow("score-string") << QString("sv_scorelimit") << 6 << false;
	QTest::newRow("team-capacity-byte") << QString("sv_maxplayersperteam") << 2 << true;
	QTest::newRow("team-capacity-word") << QString("sv_maxplayersperteam") << 3 << false;
	QTest::newRow("frag-limit-word") << QString("sv_fraglimit") << 3 << true;
	QTest::newRow("frag-limit-byte") << QString("sv_fraglimit") << 2 << false;
	QTest::newRow("frag-limit-integer") << QString("sv_fraglimit") << 4 << false;
	QTest::newRow("unknown-name") << QString("unknown_byte") << 2 << false;
	QTest::newRow("negative-sentinel") << QString("g_thingfilter") << 2 << false;
	QTest::newRow("name-suffix") << QString("sv_scorelimit_extra") << 2 << false;
	QTest::newRow("name-case") << QString("SV_SCORELIMIT") << 2 << false;
}

void TestOdamexCvar::integerInterpretation()
{
	// Given: a name/type pair and the current narrowly scoped interpretation policy.
	QFETCH(QString, name);
	QFETCH(int, wireType);
	QFETCH(bool, unsignedValue);
	const auto expected = unsignedValue
		? IntegerInterpretation::Unsigned : IntegerInterpretation::Signed;

	// When: resolving the policy through the real helper used by OdamexServer.
	const auto actual = OdamexCvar::integerInterpretationFor(
		name, static_cast<CvarType>(wireType));

	// Then: only an exact name AND type match selects the unsigned exception.
	QCOMPARE(static_cast<int>(actual), static_cast<int>(expected));
}

void TestOdamexCvar::readValue_data()
{
	QTest::addColumn<int>("wireType");
	QTest::addColumn<bool>("unsignedValue");
	QTest::addColumn<QByteArray>("payload");
	QTest::addColumn<QVariant>("expected");

	const auto row = [](const char *name, int type, bool unsignedValue,
		const char *hex, const char *expected)
	{
		const QString text = QString::fromLatin1(expected);
		QVariant value = text;
		if (type == 1)
			value = true;
		else if (type >= 2 && type <= 4)
		{
			if (unsignedValue)
				value = text.toUInt();
			else
				value = text.toInt();
		}
		QTest::newRow(name) << type << unsignedValue << QByteArray::fromHex(hex) << value;
	};
	row("boolean-signed", 1, false, "", "1");
	row("boolean-unsigned", 1, true, "", "1");
	row("byte-signed-zero", 2, false, "00", "0");
	row("byte-signed-max", 2, false, "7f", "127");
	row("byte-signed-min", 2, false, "80", "-128");
	row("byte-signed-minus-one", 2, false, "ff", "-1");
	row("byte-unsigned-zero", 2, true, "00", "0");
	row("byte-unsigned-high-bit", 2, true, "80", "128");
	row("byte-unsigned-max", 2, true, "ff", "255");
	row("word-signed-zero", 3, false, "0000", "0");
	row("word-signed-max", 3, false, "ff7f", "32767");
	row("word-signed-min", 3, false, "0080", "-32768");
	row("word-signed-minus-one", 3, false, "ffff", "-1");
	row("word-unsigned-zero", 3, true, "0000", "0");
	row("word-unsigned-high-bit", 3, true, "0080", "32768");
	row("word-unsigned-max", 3, true, "ffff", "65535");
	row("integer-signed-zero", 4, false, "00000000", "0");
	row("integer-signed-max", 4, false, "ffffff7f", "2147483647");
	row("integer-signed-min", 4, false, "00000080", "-2147483648");
	row("integer-signed-minus-one", 4, false, "ffffffff", "-1");
	row("integer-unsigned-zero", 4, true, "00000000", "0");
	row("integer-unsigned-high-bit", 4, true, "00000080", "2147483648");
	row("integer-unsigned-max", 4, true, "ffffffff", "4294967295");
	row("float-signed", 5, false, "31302e3500", "10.5");
	row("float-unsigned", 5, true, "31302e3500", "10.5");
	row("float-original-text", 5, false, "312e30303030652b303100", "1.0000e+01");
	row("float-invalid-text", 5, false, "6e6f706500", "nope");
	row("string-signed", 6, false, "61263c00", "a&<");
	row("string-unsigned", 6, true, "61263c00", "a&<");
	row("empty-string", 6, false, "00", "");
	// Preserve the existing fallback; stricter unknown-tag validation is separate work.
	row("unknown-tag-signed", 255, false, "61626300", "abc");
	row("unknown-tag-unsigned", 255, true, "61626300", "abc");
}

void TestOdamexCvar::readValue()
{
	// Given: one encoded value followed by a sentinel representing the next field.
	QFETCH(int, wireType);
	QFETCH(bool, unsignedValue);
	QFETCH(QByteArray, payload);
	QFETCH(QVariant, expected);
	QBuffer buffer;
	buffer.setData(payload + char(0xA5));
	QVERIFY(buffer.open(QIODevice::ReadOnly));
	QDataStream stream(&buffer);
	stream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper in(&stream);
	const auto interpretation = unsignedValue
		? IntegerInterpretation::Unsigned : IntegerInterpretation::Signed;

	// When: the name-independent reader uses the explicitly selected interpretation.
	const QVariant actual = OdamexCvar::readCvarValue(
		in, static_cast<CvarType>(wireType), interpretation);

	// Then: the native type and value retain sign, unsigned range, and original text.
	QCOMPARE(actual.userType(), expected.userType());
	QCOMPARE(actual, expected);

	// And sanity-check: exactly the value was consumed, leaving the next field intact.
	QCOMPARE(in.readQUInt8(), quint8(0xA5));
	QVERIFY(buffer.atEnd());
	QCOMPARE(stream.status(), QDataStream::Ok);
}

void TestOdamexCvar::convertIntegerValue_data()
{
	QTest::addColumn<QVariant>("value");
	QTest::addColumn<int>("expectedSigned");
	QTest::addColumn<uint>("expectedUnsigned");

	QTest::newRow("true") << QVariant(true) << 1 << 1u;
	QTest::newRow("signed-negative") << QVariant(-1) << -1 << 0u;
	QTest::newRow("signed-minimum") << QVariant(-2147483647 - 1) << (-2147483647 - 1) << 0u;
	QTest::newRow("unsigned-fits-int") << QVariant(2147483647u) << 2147483647 << 2147483647u;
	QTest::newRow("unsigned-maximum") << QVariant(4294967295u) << 0 << 4294967295u;
	QTest::newRow("decimal-text") << QVariant(QString(" +128 ")) << 128 << 128u;
	QTest::newRow("negative-text") << QVariant(QString("-128")) << -128 << 0u;
	QTest::newRow("text-above-int") << QVariant(QString("2147483648")) << 0 << 2147483648u;
	QTest::newRow("text-at-uint-max") << QVariant(QString("4294967295")) << 0 << 4294967295u;
	QTest::newRow("text-above-uint") << QVariant(QString("4294967296")) << 0 << 0u;
	QTest::newRow("text-below-int") << QVariant(QString("-2147483649")) << 0 << 0u;
	QTest::newRow("very-large-text") << QVariant(QString("9223372036854775808")) << 0 << 0u;
	QTest::newRow("fractional-text") << QVariant(QString("10.5")) << 0 << 0u;
	QTest::newRow("nonnumeric-text") << QVariant(QString("invalid")) << 0 << 0u;
}

void TestOdamexCvar::convertIntegerValue()
{
	// Given: a decoded numeric, boolean, or text value for an integer property.
	QFETCH(QVariant, value);
	QFETCH(int, expectedSigned);
	QFETCH(uint, expectedUnsigned);

	// When: converting the value to the property's integer representation.
	const int signedValue = OdamexCvar::valueToInt(value);
	const uint unsignedValue = OdamexCvar::valueToUInt(value);

	// Then: representable values survive; invalid values become zero, not wrapped.
	QCOMPARE(signedValue, expectedSigned);
	QCOMPARE(unsignedValue, expectedUnsigned);
}

QTEST_GUILESS_MAIN(TestOdamexCvar)

#include "tst_odamexcvar.moc"
