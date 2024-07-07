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
	/// Who recorded this demo.
	QString author;
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

/**
 * Control how the demo should be recorded.
 */
class DemoRecord
{
public:
	enum Control
	{
		/// Don't record any demo at all.
		NoDemo,
		/// Record Doomseeker's managed demo to DemoStore.
		Managed,
		/// Record an unmanaged (normal) demo.
		Unmanaged,
	};

	DemoRecord();
	DemoRecord(Control control);

	operator Control() const;

private:
	class PrivData
	{
	public:
		DemoRecord::Control control;
	};
	PrivData d;
};

/**
 * Managed demo storage, organizes the demos and decorates them with metadata.
 */
class DemoStore
{
public:
	/**
	 * Create a DemoStore pointing to the program's default managed demo storage.
	 */
	DemoStore();

	/**
	 * Create a DemoStore where demos are stored in the passed-in directory.
	 */
	DemoStore(QDir root);

	/**
	 * Demo filename filters in format applicable for QFileDialog.
	 */
	static QStringList demoFileFilters();

	/**
	 * Create the managed store for the DemoStore.
	 *
	 * This method shows UI error pop-ups if something fails that informs the
	 * user about what was the cause of the failure. That's why it accepts
	 * a `QWidget` to use as a parent for the `QMessageBox`.
	 *
	 * @return `true` if it was created, `false` if creation failed.
	 */
	bool ensureStorageExists(QWidget *parent);

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
	bool importDemo(const GameDemo &demo);

	/**
	 * Given the path to a demo, return the path to the metadata file.
	 *
	 * Either path may not exist.
	 */
	static QString metafile(const QString &path);

	/**
	 * Create a demo name, with optional full path, basing on the passed-in
	 * parameters.
	 *
	 * @param control
	 *     Decides if the demo is to be stored in DemoStore and affects if
	 *     DemoStore's root path is prepended to the returned filename. This
	 *     happens when control is DemoRecord::Control::Managed.
	 * @param plugin
	 *     The game with which this demo is recorded.
	 * @param isMultiplayer
	 *     Is the game running in the multiplayer mode (`true`), or
	 *     singleplayer (`false`).
	 *
	 * @return The path (or just the filename) to where the recorded demo
	 * should be saved.
	 *
	 * @see saveDemoMetaData()
	 */
	QString mkDemoFullPath(DemoRecord::Control control, const EnginePlugin &plugin,
		const bool isMultiplayer);

	/**
	 * List all the demo files managed by Doomseeker.
	 *
	 * The returned elements should be treated as names that exist and are
	 * unique within the DemoStore. If the managed demos need to be removed
	 * or otherwise manipulated, they should be referred to by the names
	 * returned here.
	 *
	 * @return All names/paths to the demos within the DemoStore.
	 */
	QStringList listManagedDemos();

	/**
	 * Remove the managed demo by its name.
	 *
	 * It also removes the metadata file if it exists.
	 *
	 * @param name
	 *     Name of the demo as returned by listManagedDemos().
	 *
	 * @return `true` if the demo was removed successfully (or if it
	 * didn't exist in the first place). `false` if removal failed.
	 */
	bool removeManagedDemo(const QString &name);

	/**
	 * Save the demo's metadata in the store basing on the passed-in parameters.
	 *
	 * @param demoName
	 *     Name of the demo, previously generated with mkDemoFullPath().
	 * @param plugin
	 *     The game with which this demo is recorded.
	 * @param iwad
	 *     The IWAD used in the demo recording.
	 * @param pwads
	 *     List of PWADs and other mod files used in the demo recording,
	 *     provided in the game-load order.
	 * @param isMultiplayer
	 *     Is the game running in the multiplayer mode (`true`), or
	 *     singleplayer (`false`).
	 */
	void saveDemoMetaData(const QString &demoName, const EnginePlugin &plugin,
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
	 * The demo doesn't need to be in the managed demo storage (hence
	 * the method is `static`).
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
	GameDemo loadManagedGameDemo(const QString &name);

private:
	DPtr<DemoStore> d;

	/**
	 * List all the unique demo filename extensions known by the loaded engines.
	 */
	static QStringList listDemoExtensions();

	static QString mkDemoName(const EnginePlugin &plugin, const bool isMultiplayer);
	void saveDemoMetaData(const GameDemo &demo, const QString &path);
};

#endif
