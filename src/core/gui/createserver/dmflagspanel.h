//------------------------------------------------------------------------------
// dmflagspanel.h
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
#ifndef id76bed8d4_95b3_443b_9d90_4f95b6310cf8
#define id76bed8d4_95b3_443b_9d90_4f95b6310cf8

#include "dptr.h"

#include <QtContainerFwd>
#include <QWidget>

class DMFlagsSection;
class EnginePlugin;
class GameCreateParams;
class Ini;

class DMFlagsPanel : public QWidget
{
public:
	DMFlagsPanel(QWidget *parent);
	~DMFlagsPanel() override;

	void fillInParams(GameCreateParams &params);
	bool initDMFlagsTabs(const EnginePlugin *engine);
	void loadConfig(Ini &config);
	void saveConfig(Ini &config);

private:
	DPtr<DMFlagsPanel> d;

	QList<DMFlagsSection> dmFlags() const;
	void removeDMFlagsTabs();
};

#endif
