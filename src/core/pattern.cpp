//------------------------------------------------------------------------------
// pattern.cpp
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

#include "pattern.h"

#include <QDataStream>

DClass<Pattern>
{
public:
	QString userPattern;
	Pattern::Syntax syntax = Pattern::RegExp;
};

DPointered(Pattern)

Pattern::Pattern() = default;
Pattern::Pattern(const Pattern &pattern) = default;

Pattern::Pattern(const QString &pattern, QRegularExpression::PatternOptions options, Syntax syntax)
	: QRegularExpression()
{
	setUserPattern(pattern, syntax);
	setPatternOptions(options);
}

Pattern::Pattern(const QRegularExpression &re)
	: QRegularExpression(re)
{
	d->userPattern = re.pattern();
}

Pattern::~Pattern() = default;

QString Pattern::userPattern() const
{
	return d->userPattern;
}

void Pattern::setUserPattern(const QString &pattern, Syntax syntax)
{
	d->userPattern = pattern;
	d->syntax = syntax;

	if (syntax == Wildcard)
		setPattern(QRegularExpression::wildcardToRegularExpression(pattern));
	else
		setPattern(pattern);
}

QDataStream &operator<<(QDataStream &out, const Pattern &re)
{
	return out << static_cast<const QRegularExpression &>(re) << re.d->userPattern << re.d->syntax;
}

QDataStream &operator>>(QDataStream &in, Pattern &re)
{
	return in >> static_cast<QRegularExpression &>(re) >> re.d->userPattern >> re.d->syntax;
}
