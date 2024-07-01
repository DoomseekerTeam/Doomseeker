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

#include <QDateTime>
#include <QList>
#include <QString>
#include <QWidget>

class EnginePlugin;
class PWad;

struct GameDemo
{
	QString filepath;
	QString port;
	QDateTime time;
	QString iwad;
	QList<PWad> wads;
};

class DemoRecord
{
public:
	enum Control
	{
		NoDemo,
		Managed,
		Unmanaged
	};

	DemoRecord();
	DemoRecord(Control control);

	static bool ensureStorageExists(QWidget *parent);
	static QString mkDemoFullPath(Control control, const EnginePlugin &plugin, const bool isMultiplayer);
	static void saveDemoMetaData(const QString &demoName, const EnginePlugin &plugin,
		const QString &iwad, const QList<PWad> &pwads, const bool isMultiplayer);

	/**
	 * Populate GameDemo struct with the demo metadata at path.
	 *
	 * Try to parse some of the metadata from the filename first. Then,
	 * look for the .ini file with the same name and try to read more
	 * metadata from that. If these fail, the returned GameDemo may be
	 * partially populated only.
	 *
	 * If GameDemo::port in the returned struct is empty, the path may
	 * not be pointing to a demo file at all, or it's a demo but lacks
	 * the Doomseeker metadata. The GameDemo::filepath is guaranteed to
	 * be populated at least (it's set to the path param).
	 *
	 * @param path
	 *     Direct path to demo file.
	 *
	 * @return GameDemo struct populated with info about the demo at path.
	 */
	static GameDemo loadGameDemo(const QString &path);

	operator Control() const;

private:
	class PrivData
	{
	public:
		DemoRecord::Control control;
	};
	PrivData d;

	static QString mkDemoName(const EnginePlugin &plugin, const bool isMultiplayer);
};

#endif
