//------------------------------------------------------------------------------
// freedoomquery.h
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
#ifndef ida836a012_9be3_4ce5_b9d3_f94c2f8e061d
#define ida836a012_9be3_4ce5_b9d3_f94c2f8e061d

#include "dptr.h"
#include <QObject>

class ModSet;

class FreedoomQuery : public QObject
{
	Q_OBJECT

public:
	FreedoomQuery(QObject *parent = nullptr);
	~FreedoomQuery() override;

	const QString &error() const;
	const ModSet &modSet() const;
	void start();

signals:
	void finished();

private:
	DPtr<FreedoomQuery> d;

private slots:
	void onNetworkQueryFinished();
};

#endif
