//------------------------------------------------------------------------------
// maplistpanel.h
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
#ifndef id73b8c7bf_fc0d_416b_bda8_e1252e0763c0
#define id73b8c7bf_fc0d_416b_bda8_e1252e0763c0

#include "dptr.h"

#include <QWidget>

class CreateServerDialog;
class EnginePlugin;
class GameCreateParams;
class Ini;

class MapListPanel : public QWidget
{
	Q_OBJECT

public:
	MapListPanel(QWidget *parent);
	~MapListPanel() override;

	void fillInParams(GameCreateParams &params);
	bool hasMaps() const;
	bool isMapOnList(const QString &mapName) const;
	void loadConfig(Ini &config);
	void saveConfig(Ini &config);
	void setCreateServerDialog(CreateServerDialog *dialog);
	void setupForEngine(const EnginePlugin *engine);

protected:
	void showEvent(QShowEvent *event) override;

private:
	void addMapToMaplist(const QString &map);

	DPtr<MapListPanel> d;
private slots:
	void addMapFromEditBoxToList();
	void addMapsFromLoadedWads();
	void onFocusChanged(QWidget *old, QWidget *now);
	void removeSelectedFromList();
	void updateMapWarningVisibility();
};

#endif
