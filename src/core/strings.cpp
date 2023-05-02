//------------------------------------------------------------------------------
// strings.cpp
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
#include "strings.hpp"

#include "random.h"

#include "datastreamoperatorwrapper.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"

#include <cassert>
#include <cmath>

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QStringList>
#include <QUrl>

const char Strings::RANDOM_CHAR_POOL[RANDOM_CHAR_POOL_SIZE] =
{
	'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u',
	'v', 'w', 'x', 'y', 'z', '0', '1',
	'2', '3', '4', '5', '6', '7', '8',
	'9'
};

QString Strings::colorizeString(const QString &str, int current)
{
	static const char colorChart[22][7] =
	{
		"FF91A4", //a
		"D2B48C", //b
		"808080", //c
		"32CD32", //d
		"918151", //e
		"F4C430", //f
		"E32636", //g
		"0000FF", //h
		"FF8C00", //i
		"C0C0C0", //j
		"FFD700", //k
		"E34234", //l
		"000000", //m
		"4169E1", //n
		"FFDEAD", //o
		"465945", //p
		"228b22", //q
		"800000", //r
		"704214", //s
		"A020F0", //t
		"404040", //u
		"007F7F", //v
	};

	QString ret;
	bool colored = false;
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == ESCAPE_COLOR_CHAR)
		{
			i++;
			if (i >= str.length())
				break;
			QChar colorChar = str[i].toLower();
			int color = colorChar.toLatin1() - 97;

			// special cases
			if (colorChar == '+')
				color = current == 0 ? 19 : current - 1; // + is the current minus one, wrap if needed.
			else if (colorChar == '*')
				color = 3; // Chat color which is usally green
			else if (colorChar == '!')
				color = 16; // Team char (usually green, but made dark green here for distinction)
			else if (colorChar == '[') // Named!
			{
				int end = str.indexOf(']', i);
				if (end == -1)
					break;
				QString colorName = str.mid(i + 1, end - i - 1);
				if (colorName.indexOf('"') == -1) // Just in case there's a security problem.
					ret += QString("<span style=\"color: " + colorName + "\">");
				i += colorName.length() + 1;
				colored = true;
				continue;
			}
			else if (colorChar == '-')
			{
				if (colored)
					ret += "</span>";
				colored = false;
				continue;
			}

			if (colored)
			{
				ret += "</span>";
				colored = false;
			}

			if (color >= 0 && color < 22)
			{
				ret += QString("<span style=\"color: #") + colorChart[color] + "\">";
				colored = true;
			}
			continue;
		}
		ret += str[i];
	}
	if (colored)
		ret += "</span>";
	return ret;
}

QStringList Strings::combineManyPaths(const QStringList &fronts, const QString &pathEnd)
{
	QStringList result;
	for (const QString &s : fronts)
	{
		result << combinePaths(s, pathEnd);
	}
	return result;
}

QString Strings::combinePaths(QString pathFront, QString pathEnd)
{
	QString combinedPath;

	// One of them is nullptr
	if (pathFront.isEmpty())
	{
		return pathEnd;
	}

	if (pathEnd.isEmpty())
	{
		return pathFront;
	}

	pathFront = Strings::trimr(pathFront, "/\\");
	pathEnd = Strings::triml(pathEnd, "/\\");

	combinedPath = pathFront + "/" + pathEnd;
	combinedPath = normalizePath(combinedPath);

	return combinedPath;
}

QString Strings::createRandomAlphaNumericString(unsigned numChars)
{
	QString generatedString = "";
	for (unsigned i = 0; i < numChars; ++i)
	{
		auto index = (unsigned) Random::nextUShort(RANDOM_CHAR_POOL_SIZE);
		generatedString += RANDOM_CHAR_POOL[index];
	}

	return generatedString;
}

QString Strings::createRandomAlphaNumericStringWithNewLines(unsigned numCharsPerLine, unsigned numLines)
{
	QString generatedString = "";
	for (unsigned i = 0; i < numLines; ++i)
	{
		generatedString += createRandomAlphaNumericString(numCharsPerLine) + "\n";
	}

	return generatedString;
}

// NOTE: Be sure that '\\' is the first thing in the array otherwise it will re-escape.
static char escapeCharacters[] = {'\\', '"', 0};
const QString &Strings::escape(QString &str)
{
	for (unsigned int i = 0; escapeCharacters[i] != 0; i++)
	{
		// += 2 because we'll be inserting 1 character.
		for (int p = 0; p < str.length() && (p = str.indexOf(escapeCharacters[i], p)) != -1; p += 2)
		{
			str.insert(p, '\\');
		}
	}
	return str;
}
const QString &Strings::unescape(QString &str)
{
	for (unsigned int i = 0; escapeCharacters[i] != 0; i++)
	{
		QString sequence = "\\" + QString(escapeCharacters[i]);
		for (int p = 0; p < str.length() && (p = str.indexOf(sequence, p)) != -1; p++)
			str.replace(str.indexOf(sequence, p), 2, escapeCharacters[i]);
	}
	return str;
}

QString Strings::formatDataAmount(qint64 bytes)
{
	DataUnit dataUnit;

	auto fBytes = (float)bytes;
	fBytes = scaleDataUnit(fBytes, dataUnit);

	QString formattedString = QString("%1 ").arg(fBytes, 0, 'f', 2);
	switch (dataUnit)
	{
	case Byte:
		formattedString += "B";
		break;

	case Kilobyte:
		formattedString += "kB";
		break;

	case Megabyte:
		formattedString += "MB";
		break;

	case Gigabyte:
		formattedString += "GB";
		break;

	default:
		// Shouldn't really happen.
		return "#ERR: Formatting data amount error.";
	}

	return formattedString;
}

QString Strings::formatDataSpeed(float speedInBytesPerSecond)
{
	DataUnit dataUnit;

	speedInBytesPerSecond = scaleDataUnit(speedInBytesPerSecond, dataUnit);

	QString formattedString = QString("%1 ").arg(speedInBytesPerSecond, 0, 'f', 2);
	switch (dataUnit)
	{
	case Byte:
		formattedString += "B/s";
		break;

	case Kilobyte:
		formattedString += "kB/s";
		break;

	case Megabyte:
		formattedString += "MB/s";
		break;

	case Gigabyte:
		formattedString += "GB/s";
		break;

	default:
		// Shouldn't really happen.
		return "#ERR: Formatting speed error.";
	}

	return formattedString;
}

QString Strings::formatTime(float seconds)
{
	if (seconds < 0.0f)
	{
		return "#ERR: Formatting time error.";
	}

	seconds = ceil(seconds);

	// QTime is a 24-hour clock. It cannot be used here since seconds input
	// can be larger than that.

	int hours = 0;
	int minutes = 0;
	int remainingSeconds = 0;

	if (seconds >= 3600.0f)
	{
		// An hour or more.
		hours = seconds / 3600.0f;
		seconds -= hours * 3600.0f;
	}

	if (seconds >= 60.0f)
	{
		// A minute or more.
		minutes = seconds / 60.0f;
		seconds -= minutes * 60.0f;
	}

	remainingSeconds = (int)seconds;

	QString formattedString;
	if (hours > 0)
	{
		formattedString += QString("%1h ").arg(hours);
	}

	if (hours > 0 || minutes > 0)
	{
		formattedString += QString("%1min. ").arg(minutes);
	}

	formattedString += QString("%1s").arg(remainingSeconds);

	return formattedString;
}

bool Strings::isCharOnCharList(char c, const QString &charList)
{
	for (const auto &candidate : charList)
	{
		if (candidate == c)
			return true;
	}

	return false;
}

bool Strings::isUrlSafe(const QString &url)
{
	QUrl urlObject = url;

	QString scheme = urlObject.scheme();

	bool bIsSafe1 = scheme.isEmpty();
	bool bIsSafe2 = (scheme.compare("http", Qt::CaseInsensitive) == 0);
	bool bIsSafe3 = (scheme.compare("ftp", Qt::CaseInsensitive) == 0);
	bool bIsSafe4 = (scheme.compare("https", Qt::CaseInsensitive) == 0);


	return bIsSafe1 || bIsSafe2 || bIsSafe3 || bIsSafe4;
}

QString Strings::normalizePath(QString path)
{
	path = QDir::fromNativeSeparators(path);
	path = QDir::cleanPath(path);

	return path;
}

QByteArray Strings::readUntilByte(QDataStream &stream, unsigned char stopByte)
{
	DataStreamOperatorWrapper reader(&stream);
	return reader.readRawUntilByte(stopByte);
}

float Strings::scaleDataUnit(float bytes, DataUnit &outUnit)
{
	const static float UPPER_BOUNDARY = 900.0f;
	outUnit = Byte;

	while (bytes > UPPER_BOUNDARY && outUnit != Gigabyte)
	{
		bytes /= 1024.0f;
		outUnit = (DataUnit)((int)outUnit + 1);
	}

	return bytes;
}

QString Strings::timestamp(const QString &format)
{
	return QDateTime::currentDateTime().toString(format);
}

void Strings::translateServerAddress(const QString &addressString, QString &hostname, unsigned short &port, const QString &defaultAddress)
{
	port = 0;
	QStringList addressAndPort = addressString.split(":");
	QStringList defaultAddressAndPort = defaultAddress.split(":");

	if (addressAndPort.size() >= 1 && addressAndPort.size() <= 2)
	{
		hostname = addressAndPort[0];
		if (addressAndPort.size() == 2)
		{
			port = addressAndPort[1].toUShort();
		}
	}
	else
	{
		// if something is not right set default settings
		if (defaultAddressAndPort.size() >= 1)
		{
			hostname = defaultAddressAndPort[0];
		}
	}

	if (port == 0 && defaultAddressAndPort.size() >= 2)
	{
		port = defaultAddressAndPort[1].toUShort();
	}
}

QString &Strings::trimr(QString &str, const QString &charList)
{
	int i;
	for (i = str.length() - 1; i >= 0; --i)
	{
		if (!isCharOnCharList(str[i].toLatin1(), charList))
			break;
	}
	++i;

	return str.remove(i, str.length() - i);
}

QString &Strings::triml(QString &str, const QString &charList)
{
	int i;
	for (i = 0; i < str.length(); ++i)
	{
		if (!isCharOnCharList(str[i].toLatin1(), charList))
			break;
	}

	return str.remove(0, i);
}

QString Strings::wrapUrlsWithHtmlATags(const QString &str)
{
	static QString pluginSchemes;
	if (pluginSchemes.isEmpty())
	{
		pluginSchemes = "zds";

		for (unsigned int i = 0; i < gPlugins->numPlugins(); ++i)
			pluginSchemes = QString("%1|%2").arg(pluginSchemes)
				.arg(gPlugins->plugin(i)->info()->data()->scheme);
	}

	QRegExp pattern(QString("("
		"("
		"(http|https|ftp|%1)://"
		"|(www\\.)"
		")[\\w\\-\\.,@?^=%&amp;:/~\\+#\\(\\)]+"
		")").arg(pluginSchemes), Qt::CaseInsensitive);
	QString newString = str;

	int offset = 0;
	int index = -1;
	while ((index = pattern.indexIn(newString, offset)) >= 0)
	{
		QString cap = pattern.cap(1);
		int capLength = cap.length();

		QString replacement = cap;
		if (cap.startsWith("www.", Qt::CaseInsensitive))
		{
			replacement = "http://" + cap;
		}

		replacement = QString("<a href=\"%1\">%2</a>").arg(replacement, cap);

		newString.replace(index, capLength, replacement);
		offset = index + replacement.length();
	}

	return newString;
}
