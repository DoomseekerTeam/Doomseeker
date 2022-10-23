//------------------------------------------------------------------------------
// testmodreader.cpp
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------

#include "testmodreader.h"

#include "modreader.h"

#include <QResource>
#include <QSet>
#include <QSharedPointer>

bool TestModReader::executeTest()
{
	// If something fails, set to false;
	bool result = true;

	// Map searching
	QSet<QString> mapsToCheck =
	{
		"pk3map", // PK3 format
		"BASEPK3", // PK3 format, those can have wads on their base-directory.
		"HEXENMAP", // Hexen format
		"DOOMMAP", // Doom format
		"UDMFMAP" // Udmf format
	};
	QSharedPointer<ModReader> modReader(ModReader::create(QResource("7zedWadsTest.7z").absoluteFilePath()));
	if (!modReader.isNull())
	{
		modReader->load();
		QStringList maps = modReader->getAllMaps();
		for (const QString &map : mapsToCheck)
		{
			bool found = maps.contains(map);
			testLog << map + " - " + ((found) ? "Found" : "Not Found");
			if (!found)
				result = false;
		}
	}
	else
		result = false;

	return result;
}
