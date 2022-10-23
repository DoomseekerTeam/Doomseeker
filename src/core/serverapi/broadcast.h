//------------------------------------------------------------------------------
// broadcast.h
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
#ifndef idc606b220_53de_4e6a_b364_1aed39334d8f
#define idc606b220_53de_4e6a_b364_1aed39334d8f

#include "dptr.h"
#include "global.h"
#include "serverapi/serverptr.h"

class EnginePlugin;

class MAIN_EXPORT Broadcast : public QObject
{
	Q_OBJECT

public:
	Broadcast(QObject *parent = nullptr);
	virtual ~Broadcast() override;

	/**
	 * @brief Is LAN broadcast capture enabled?
	 *
	 * Plugins need to decide themselves how to react to being
	 * disabled. Broadcast is always enabled by default.
	 *
	 * Setter emits enabledChanged() signal.
	 */
	bool isEnabled() const;
	void setEnabled(bool enabled);

	/**
	 * @brief Plugin this Broadcast belongs to.
	 *
	 * New instances of EnginePlugin shouldn't be created here. Instead
	 * each plugin should keep a global instance of EnginePlugin (singleton?)
	 * and a pointer to this instance should be returned.
	 */
	virtual EnginePlugin *plugin() const = 0;

signals:
	void enabledChanged(bool enabled);
	void serverDetected(ServerPtr server, bool needsRefresh);
	void serverLost(ServerPtr server);

private:
	DPtr<Broadcast> d;
};

#endif
