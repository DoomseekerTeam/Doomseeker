//------------------------------------------------------------------------------
// pluginloader.h
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
// Copyright (C) 2011 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef __PLUGINLOADER_H__
#define __PLUGINLOADER_H__

#include <QList>
#include <QMap>
#include <QString>

#include "dptr.h"
#include "global.h"

#define gPlugins (PluginLoader::instance())

class EnginePlugin;

class PluginLoader
{
public:
	/**
	 * This class handles one specific plugin.  It allows for cross-platform access
	 * to the plugins.
	 */
	class Plugin
	{
	public:
		/**
		 * @brief Inits a plugin.
		 *
		 * @param type
		 *     Magic number which is compared with any possible
		 *     plugins to confirm it is the right type.
		 * @param file
		 *     Path to plugin file.
		 */
		Plugin(unsigned int type, QString file);
		virtual ~Plugin();

		/**
		 * @brief Returns a pointer to the requested function or nullptr.
		 */
		void *function(const char *func) const;
		void initConfig();
		bool isValid() const;
		/**
		 * @brief Main plugin interface.
		 */
		EnginePlugin *info() const;

	private:
		DPtr<Plugin> d;

		void unload();
	};

	/**
	 * @brief Destroys the init() instance.
	 */
	static void deinit();
	/**
	 * @brief Attempts to load plugins from given set of directories.
	 *
	 * PluginLoader will step through directory in listed order. If
	 * a directory with at least one valid plugin is encountered, the
	 * loading process stops and skips all further directories.
	 *
	 * This behavior may need to be revised and changed in the future.
	 *
	 * @param directories
	 *     The list of directories from which to load the plugins.
	 * @param pluginsEnabled
	 *     Force these plugins to be either enabled or disabled, disregarding
	 *     the default approach. The plugin names should be specified without
	 *     the 'lib' prefix or the filename extension.
	 */
	static void init(const QStringList &directories,
		const QMap<QString, bool> &pluginsEnabled);
	/**
	 * @brief Accesses instance of the class after init().
	 *
	 * This isn't a real singleton. Call init() before calling this.
	 * gpPluginLoader and gPluginLoader macros can also be used to
	 * access the instance.
	 */
	static PluginLoader *instance();

	virtual ~PluginLoader();

	/**
	 * @brief Clears the plugins list
	 */
	void clearPlugins();

	/**
	 * @brief Convenience method - calls Plugin::info() for specified
	 *        plugin.
	 *
	 * @return nullptr can be returned if plugin with given index is not
	 * present.
	 */
	EnginePlugin *info(int pluginIndex) const;

	/**
	 * @brief Get info() by pluginIndexFromName().
	 *
	 * Even bigger convenience method: find a plugin by its name,
	 * and return its EnginePlugin, or nullptr if not found.
	 *
	 * @return nullptr can be returned if plugin with given name was not
	 * found.
	 */
	EnginePlugin *info(const QString &name) const
	{
		auto idx = pluginIndexFromName(name);
		return idx >= 0 ? info(idx) : nullptr;
	}

	/**
	 * @brief Inits configuration for plugins.
	 */
	void initConfig();

	/**
	 * @brief Gets the number of loaded plugins.
	 *
	 * It will return 0 in safe mode.
	 */
	unsigned int numPlugins() const;

	/**
	 * @brief Looks for a plugin which name equals to parameter.
	 *
	 * This tries to match the plugin name by several variants:
	 *
	 * 1. Match to EnginePlugin::nameCanonical() as-is.
	 * 2. Match to a mangled canonical name, by removing spaces,
	 *    underscores, and comparing case-insensitively.
	 * 3. Match to plugin's displayed name, applying the same
	 *    mangling as before.
	 *
	 * @param name
	 *     Name to look for.
	 * @return index of found plugin in the plugin array, or -1
	 *         if not found.
	 */
	int pluginIndexFromName(const QString &name) const;

	const QList<Plugin *> &plugins() const;
	/**
	 * @brief Returns the requested plugin or nullptr.
	 */
	const Plugin *plugin(unsigned int index) const;

	/**
	 * @brief Resets the plugins directory, clearing the loaded plugins and
	 *        getting new loaded plugins in the process.
	 */
	void resetPluginsDirectory(const QString &pluginsDirectory);

	/**
	 * @brief Returns the requested plugin or nullptr.
	 */
	const Plugin *operator[] (unsigned int index) const;

private:
	DPtr<PluginLoader> d;

	static PluginLoader *staticInstance;

	PluginLoader(unsigned int type, const QStringList &directories,
		const QMap<QString, bool> &pluginsEnabled);

	bool filesInDir();
};

#endif /* __PLUGINLOADER_HPP__ */
