//------------------------------------------------------------------------------
// engineplugincombobox.h
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
#ifndef id436d9d45_1fa2_4bc9_9b7e_1dacbffcd6ed
#define id436d9d45_1fa2_4bc9_9b7e_1dacbffcd6ed

#include <QComboBox>

class EnginePlugin;

class EnginePluginComboBox : public QComboBox
{
	Q_OBJECT

public:
	EnginePluginComboBox(QWidget *parent);

	EnginePlugin *currentPlugin() const;
	bool setPluginByName(const QString &name);

signals:
	void currentPluginChanged(EnginePlugin *plugin);

private:
	void loadPlugins();

private slots:
	void onIndexChanged();
};

#endif
