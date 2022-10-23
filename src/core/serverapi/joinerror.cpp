//------------------------------------------------------------------------------
// joinerror.cpp
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
#include "joinerror.h"
#include "serverapi/serverstructs.h"

#include <QList>

DClass<JoinError>
{
public:
	JoinError::JoinErrorType type;
	QString error;

	/**
	 * This is valid only if type == MissingWads.
	 */
	QString missingIwad;

	/**
	 * This is valid only if type == MissingWads.
	 */
	QList<PWad> missingWads;
	QList<PWad> incompatibleWads;
};

DPointered(JoinError)

JoinError::JoinError()
{
	d->type = NoError;
}

JoinError::JoinError(JoinError::JoinErrorType type)
{
	d->type = type;
}

JoinError::JoinError(const JoinError &other)
{
	d = other.d;
}

JoinError &JoinError::operator=(const JoinError &other)
{
	d = other.d;
	return *this;
}

JoinError::~JoinError()
{
}

void JoinError::addMissingWad(const PWad &wad)
{
	d->missingWads << wad;
}

void JoinError::clearMissingWads()
{
	d->missingWads.clear();
}

const QString &JoinError::error() const
{
	return d->error;
}

bool JoinError::isError() const
{
	return d->type != NoError;
}

bool JoinError::isMissingIwadOnly() const
{
	return d->type == MissingWads
		&& !d->missingIwad.isEmpty()
		&& d->missingWads.isEmpty()
		&& d->incompatibleWads.isEmpty();
}

bool JoinError::isMissingWadsError() const
{
	return d->type == MissingWads;
}

const QString &JoinError::missingIwad() const
{
	return d->missingIwad;
}

const QList<PWad> &JoinError::missingWads() const
{
	return d->missingWads;
}

const QList<PWad> &JoinError::incompatibleWads() const
{
	return d->incompatibleWads;
}

void JoinError::setError(const QString &error)
{
	d->error = error;
}

void JoinError::setMissingIwad(const QString &iwad)
{
	d->missingIwad = iwad;
}

void JoinError::setMissingWads(const QList<PWad> &wads)
{
	d->missingWads = wads;
}

void JoinError::setIncompatibleWads(const QList<PWad> &wads)
{
	d->incompatibleWads = wads;
}

void JoinError::setType(JoinErrorType type)
{
	d->type = type;
}

JoinError::JoinErrorType JoinError::type() const
{
	return d->type;
}
