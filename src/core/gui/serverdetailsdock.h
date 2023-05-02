//------------------------------------------------------------------------------
// serverdetailsdock.h
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
// Copyright (C) 2014 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef __SERVERDETAILSDOCK_H__
#define __SERVERDETAILSDOCK_H__

#include <QDockWidget>

#include "dptr.h"
#include "serverapi/serverptr.h"

class ServerDetailsDock : public QDockWidget
{
	Q_OBJECT

public:
	ServerDetailsDock(QWidget *parent);
	~ServerDetailsDock() override;

public slots:
	void displaySelection(QList<ServerPtr> &);

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	void clear();
	void reorientContentsBasingOnDimensions();
	ServerPtr selectServer(QList<ServerPtr> &);

	DPtr<ServerDetailsDock> d;
};

#endif
