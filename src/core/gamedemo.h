//------------------------------------------------------------------------------
// gamedemo.h
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
#ifndef ida153f497_2308_468a_8825_d9e06386db7a
#define ida153f497_2308_468a_8825_d9e06386db7a

#include "dptr.h"

#include <QList>
#include <QString>
#include <QWidget>

class EnginePlugin;
class PWad;

class GameDemo
{
public:
	enum Control
	{
		NoDemo,
		Managed,
		Unmanaged
	};

	GameDemo();
	GameDemo(Control control);

	static bool ensureStorageExists(QWidget *parent);
	static QString mkDemoFullPath(Control control, const EnginePlugin &plugin);
	static void saveDemoMetaData(const QString &demoName, const EnginePlugin &plugin,
		const QString &iwad, const QList<PWad> &pwads);

	operator Control() const;

private:
	class PrivData
	{
	public:
		GameDemo::Control control;
	};
	PrivData d;

	static QString mkDemoName(const EnginePlugin &plugin);
};

#endif
