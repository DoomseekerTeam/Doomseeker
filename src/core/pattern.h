//------------------------------------------------------------------------------
// pattern.h
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
// Copyright (C) 2021 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef PATTERN_H
#define PATTERN_H

#include "dptr.h"

#include <QRegularExpression>

class QDataStream;

/**
 * Wraps a regular expression which is potentially user visibile (i.e. created
 * by the user). In these situations we need to keep the input pattern as well
 * as the syntax used so that we can read it later.
 */
class Pattern : public QRegularExpression
{
	DPtr<Pattern> d;

	friend QDataStream &operator<<(QDataStream &out, const Pattern &re);
	friend QDataStream &operator>>(QDataStream &in, Pattern &re);

public:
	enum Syntax
	{
		RegExp,
		Wildcard
	};

	Pattern();
	Pattern(const Pattern &pattern);
	Pattern(const QRegularExpression &re);
	Pattern(const QString &pattern, QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption, Syntax syntax = RegExp);

	~Pattern();

	QString userPattern() const;
	void setUserPattern(const QString &pattern, Syntax syntax = RegExp);

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	static QString wildcardToRegularExpression(const QString &pattern)
	{
		return QRegularExpression::wildcardToRegularExpression(pattern);
	}
#else
	// Copy of Qt 5.15 version of this function is included in patternwildcard.cpp
	static QString wildcardToRegularExpression(const QString &pattern);
#endif
};

QDataStream &operator<<(QDataStream &out, const Pattern &re);
QDataStream &operator>>(QDataStream &in, Pattern &re);

Q_DECLARE_METATYPE(Pattern);

#endif
