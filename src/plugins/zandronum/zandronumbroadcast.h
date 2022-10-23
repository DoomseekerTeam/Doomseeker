//------------------------------------------------------------------------------
// zandronumbroadcast.h
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
#ifndef ide4ecbce5_6700_4d65_8e47_160a349b2211
#define ide4ecbce5_6700_4d65_8e47_160a349b2211

#include <dptr.h>
#include <QObject>
#include <serverapi/broadcast.h>
#include <serverapi/serverptr.h>

class ZandronumBroadcast : public Broadcast
{
	Q_OBJECT

public:
	ZandronumBroadcast();
	~ZandronumBroadcast() override;

	EnginePlugin *plugin() const override;
	void start();

private:
	DPtr<ZandronumBroadcast> d;

private slots:
	void bindSocket();
	void readAllPendingDatagrams();
	void terminateOldServers();
};

#endif
