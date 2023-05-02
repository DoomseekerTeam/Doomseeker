//------------------------------------------------------------------------------
// Turok2Exmasterclient.h
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
// Copyright (C) 2017 "Edward Richardson" <Edward850@crantime.org>
//------------------------------------------------------------------------------

#ifndef __TUROK2EX_MASTERCLIENT_H__
#define __TUROK2EX_MASTERCLIENT_H__

#include <serverapi/masterclient.h>

class EnginePlugin;

class Turok2ExMasterClient : public MasterClient
{
	Q_OBJECT

public:
	Turok2ExMasterClient();

	const EnginePlugin *plugin() const override;

	void updateAddress();

public slots:
	void refreshStarts() override;

protected:

	QByteArray createServerListRequest() override;
	Response readMasterResponse(const QByteArray &data) override;

private:
	unsigned int seedIP;
	unsigned short seedPort;
};

#endif /* __Turok2ExMASTERCLIENT_H__ */
