//------------------------------------------------------------------------------
// patternlist.cpp
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
#include "patternlist.h"

#include "log.h"
#include <QDataStream>
#include <QDebug>
#include <QIODevice>

bool PatternList::isExactMatchAny(const QString &candidate) const
{
	for (const auto &matcher : *this)
	{
		auto match = matcher.match(candidate, 0, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);
		if (match.hasMatch() && match.capturedLength(0) == candidate.length())
		{
			return true;
		}
	}
	return false;
}

PatternList PatternList::deserializeQVariant(const QVariant &var)
{
	PatternList result;
	for (const QVariant &element : var.toList())
	{
		if (element.canConvert<Pattern>())
		{
			result << element.value<Pattern>();
		}
		else if (element.canConvert<QRegularExpression>())
		{
			// Should never be reached, but if it is this is better than losing the pattern entirely
			result << element.toRegularExpression();
		}
		else
		{
			// Qt6 technically has QRegExp as part of QtCore5Compat module, but
			// it would be silly to bring in the whole library for one type so
			// lets read the old setting raw. We could of course use QRegExp on
			// Qt5 builds, but why maintain two branches?

			QByteArray rawData;
			{
				QDataStream stream(&rawData, QIODevice::WriteOnly);
				// This is the version that QSettings uses so probably good enough for us to keep things simple
				stream.setVersion(QDataStream::Qt_4_0);
				stream << element;
			}

			QDataStream stream(&rawData, QIODevice::ReadOnly);
			stream.setVersion(QDataStream::Qt_4_0);

			constexpr quint32 REGEXP_TYPE = 27;
			quint32 type = 0;
			stream >> type;
			if (stream.status() != QDataStream::Ok)
			{
				qDebug() << "Attempt to migrate QRegExp could not read variant type";
				continue;
			}
			if (type != REGEXP_TYPE)
			{
				qDebug() << "Attempt to migrate QRegExp encounter unexpected type:" << type;
				continue;
			}

			constexpr quint8 REGEXP_SYNTAX_WILDCARD = 1;

			QString regex;
			quint8 caseSensitivity = 0;
			quint8 syntax = 0;
			quint8 minimal = 0;
			stream >> regex >> caseSensitivity >> syntax >> minimal;
			if (stream.status() != QDataStream::Ok)
			{
				qDebug() << "Attempt to migrate QRegExp could not read contents";
				continue;
			}

			result << Pattern(regex, caseSensitivity == Qt::CaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption, syntax == REGEXP_SYNTAX_WILDCARD ? Pattern::Wildcard : Pattern::RegExp);
		}
	}
	return result;
}

QVariant PatternList::serializeQVariant() const
{
	QVariantList var;
	for (const auto &pattern : *this)
	{
		var << QVariant::fromValue(pattern);
	}
	return var;
}
