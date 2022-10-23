//------------------------------------------------------------------------------
// zandronumengineplugin.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUM_ENGINEPLUGIN_H
#define DOOMSEEKER_PLUGIN_ZANDRONUM_ENGINEPLUGIN_H

#include <dptr.h>
#include <plugins/engineplugin.h>

class ZandronumEnginePlugin : public EnginePlugin
{
	DECLARE_PLUGIN(ZandronumEnginePlugin)
public:
	ZandronumEnginePlugin();

	QList<CreateServerDialogPage *> createServerDialogPages(CreateServerDialog *pDialog) override;

	void setupConfig(IniSection &config) override;

	ConfigPage *configuration(QWidget *parent) override;
	GameHost *gameHost() override;
	QList<GameMode> gameModes() const override;
	QList<GameCVar> gameModifiers() const override;
	QList<GameCVar> limits(const GameMode &gm) const override;

	MasterClient *masterClient() const;

	ServerPtr mkServer(const QHostAddress &address, unsigned short port) const override;

	void start() override;

private:
	DPtr<ZandronumEnginePlugin> d;
};

#endif
