//------------------------------------------------------------------------------
// freedoom.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "freedoom.h"

#include "entities/modset.h"
#include "protocols/freedoom/freedoomquery.h"
#include <QStringList>

DClass<Freedoom>
{
public:
	QScopedPointer<FreedoomQuery> query;
};
DPointeredNoCopy(Freedoom)

Freedoom::Freedoom(QObject *parent)
	: QObject(parent)
{
	d->query.reset(new FreedoomQuery());
}

Freedoom::~Freedoom()
{
}

const QString &Freedoom::error() const
{
	return d->query->error();
}

bool Freedoom::isError() const
{
	return !error().isEmpty();
}

bool Freedoom::hasFreedoomReplacement(const QString &fileName)
{
	QStringList candidates;
	candidates << "doom.wad" << "doom2.wad" << "tnt.wad" << "plutonia.wad";
	return candidates.contains(fileName, Qt::CaseInsensitive);
}

const ModSet &Freedoom::modSet() const
{
	return d->query->modSet();
}

void Freedoom::requestModSet()
{
	d->query.reset(new FreedoomQuery());
	this->connect(d->query.data(), SIGNAL(finished()), SIGNAL(finished()));
	d->query->start();
}
