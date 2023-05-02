//------------------------------------------------------------------------------
// zandronumqmasterclient.h
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net> (skulltagmasterclient.h)
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUMQMASTERCLIENT_H
#define DOOMSEEKER_PLUGIN_ZANDRONUMQMASTERCLIENT_H

#include <serverapi/masterclient.h>

class EnginePlugin;

class ZandronumqMasterClient : public MasterClient
{
	Q_OBJECT

public:
	ZandronumqMasterClient();

	const EnginePlugin *plugin() const override;

public slots:
	void refreshStarts() override;

protected:
	QByteArray createServerListRequest() override;
	Response readMasterResponse(const QByteArray &data) override;

	/// This is used as a marker to determine if we have an accurate reading for numPackets.
	bool readLastPacket;
	int numPackets;
	unsigned int packetsRead;

private:
	QString masterBanHelp() const;
};

#endif
