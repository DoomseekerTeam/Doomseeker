//------------------------------------------------------------------------------
// engineplugin.h
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
#ifndef PLUGIN_ENGINEPLUGIN_H
#define PLUGIN_ENGINEPLUGIN_H

#include "plugins/engineplugin.h"

class PluginEnginePlugin : public EnginePlugin
{
	DECLARE_PLUGIN(PluginEnginePlugin)
public:
	PluginEnginePlugin();
	~PluginEnginePlugin();

	QList<DMFlagsSection> dmFlags() const;
	GameHost *gameHost();
	ServerPtr mkServer(const QHostAddress &address, unsigned short port) const;

	bool isMasterResponderInstantiated() const;
	void startMasterResponder();
	void start();

private:
	class PrivData;
	PrivData *d;
};

#endif
