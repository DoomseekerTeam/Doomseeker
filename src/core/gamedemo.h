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
#include <QDir>
#include <QList>
#include <QString>
#include <QWidget>

class EnginePlugin;
class PWad;

/**
 * The extra metadata that describes a recorded game demo.
 *
 * Conveys the file path to the actual demo (`demopath`) plus other
 * information that help describe this demo: when it was recorded,
 * what game it was, what files were loaded.
 */
struct GameDemo
{
	/// Path to the demo file this metadata describes.
	QString demopath;
	/// Name of the game (port, engine) from which the demo was recorded.
	QString game;
	/// Date and time when the demo was recorded.
	QDateTime time;
	/// IWAD used to record this demo.
	QString iwad;
	/// PWADs and other game mod files used to record this demo.
	QList<PWad> wads;

	/**
	 * The name of the file when stored in Doomseeker's managed demos directory.
	 */
	QString managedName() const;

	/**
	 * True if this metadata relates to a file found somewhere on the disk.
	 */
	bool hasDemoFile() const
	{
		return !demopath.isEmpty();
	}
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

	/**
	 * Demo filename filters in format applicable for QFileDialog.
	 */
	static QStringList demoFileFilters();

	static bool ensureStorageExists(QWidget *parent);

	/**
	 * Import demo described by GameDemo to the managed demos store.
	 *
	 * GameDemo::filepath must point to a valid demo file and it's
	 * also expected that the other attributes of GameDemo will be
	 * filled with proper info about the demo.
	 *
	 * The demo file is copied to Doomseeker's store and the metadata
	 * is saved next to it. Such imported demo then becomes
	 * indistinguishable from a normally recorded demo.
	 *
	 * If it happens that a demo with the exactly same filename already
	 * exists in the store, then the existing demo is *REMOVED*.
	 */
	static bool importDemo(const GameDemo &demo);

	/**
	 * Given the path to a demo, return the path to the metadata file.
	 *
	 * Either path may not exist.
	 */
	static QString metafile(const QString &path);

	static QString mkDemoFullPath(Control control, const EnginePlugin &plugin, const bool isMultiplayer);

	/**
	 * List all the unique demo filename extensions known by the loaded engines.
	 */
	static QStringList listDemoExtensions();

	/**
	 * List all the demo files managed by Doomseeker.
	 */
	static QStringList listManagedDemos();

	static bool removeManagedDemo(const QString &name);

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
	 *     Direct path to either the demo file or its metadata file.
	 *
	 * @return GameDemo struct populated with info about the demo at path.
	 */
	static GameDemo loadGameDemo(const QString &path);

	/**
	 * Load the GameDemo metadata for a managed demo by its name.
	 *
	 * Managed demos can be listed with listManagedDemos(). If demo
	 * is not found, an empty GameDemo struct is returned -- the
	 * GameDemo::filepath attribute will also be empty.
	 *
	 * @param name
	 *     Name of the demo as returned by listManagedDemos().
	 *
	 * @return GameDemo struct populated with info about the demo.
	 */
	static GameDemo loadManagedGameDemo(const QString &name);

	operator Control() const;

private:
	class PrivData
	{
	public:
		DemoRecord::Control control;
	};
	PrivData d;

	static QDir demosDir();
	static QString mkDemoName(const EnginePlugin &plugin, const bool isMultiplayer);
	static void saveDemoMetaData(const GameDemo &demo, const QString &path);
};

#endif
