//------------------------------------------------------------------------------
// engineplugin.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------

#include "plugins/engineplugin.h"

#include "pluginengineplugin.h"
#include "plugingamehost.h"
#include "pluginmasterclient.h"
#include "pluginserver.h"
#include "responder/masterresponder.h"
#include "responder/respondercfg.h"
#include <cassert>

class PluginEnginePlugin::PrivData
{
public:
	MasterResponder *masterResponder;
};
///////////////////////////////////////////////////////////////////////////////
INSTALL_PLUGIN(PluginEnginePlugin)

QString masterAddress()
{
	return QString("127.0.0.1:%1").arg(ResponderCfg::masterServerPort());
}

PluginEnginePlugin::PluginEnginePlugin()
{
	d = new PrivData();
	d->masterResponder = nullptr;
}

PluginEnginePlugin::~PluginEnginePlugin()
{
	delete d->masterResponder;
	delete d;
}

QList<DMFlagsSection> PluginEnginePlugin::dmFlags() const
{
	DMFlagsSection flags1("Flags 1");
	flags1 << DMFlag("Flag1 0x1", 0x1);
	flags1 << DMFlag("Flag1 0x8", 0x8);
	flags1 << DMFlag("Flag1 0x10", 0x10);

	DMFlagsSection flags2("Flags 2");
	flags2 << DMFlag("Flag2 0x40", 0x40);
	flags2 << DMFlag("Flag2 0x80", 0x80);
	flags2 << DMFlag("Flag2 0x100", 0x100);

	QList<DMFlagsSection> sections;
	sections << flags1 << flags2;
	return sections;
}

GameHost *PluginEnginePlugin::gameHost()
{
	return new PluginGameHost();
}

ServerPtr PluginEnginePlugin::mkServer(const QHostAddress &address, unsigned short port) const
{
	return ServerPtr(new PluginServer(address, port));
}

bool PluginEnginePlugin::isMasterResponderInstantiated() const
{
	return d->masterResponder != nullptr;
}

void PluginEnginePlugin::startMasterResponder()
{
	assert(!isMasterResponderInstantiated());
	d->masterResponder = new MasterResponder();
	d->masterResponder->bind(ResponderCfg::masterServerPort());
}

void PluginEnginePlugin::start()
{
	init("Fake Plugin", nullptr,
		EP_Author, "The Doomseeker Team",
		EP_Version, PLUGIN_VERSION,

		EP_MasterClient, new PluginMasterClient(),
		EP_DefaultMaster, masterAddress().toUtf8().constData(),
		EP_Done
	);
}
