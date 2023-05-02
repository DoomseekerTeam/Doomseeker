//------------------------------------------------------------------------------
// pluginloader.cpp
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
#include "pluginloader.h"

#include "configuration/doomseekerconfig.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "log.h"
#include "plugins/engineplugin.h"
#include "serverapi/masterclient.h"
#include "strings.hpp"
#include <cassert>
#include <QDir>
#include <QLibrary>
#include <QScopedPointer>

////////////////////////////////////////////////////////////////////////////////
DClass<PluginLoader::Plugin>
{
public:
	EnginePlugin *info;
	QString file;
	QScopedPointer<QLibrary> library;

	static bool isForbiddenPlugin(QString file)
	{
		return QFileInfo(file).fileName().toLower().contains("vavoom");
	}
};

DPointeredNoCopy(PluginLoader::Plugin)

PluginLoader::Plugin::Plugin(unsigned int type, QString file)
{
	Q_UNUSED(type); // THIS SHOULD BE REVIEWED. TYPE SEEMS USELESS.
	d->file = file;
	d->info = nullptr;
	if (PrivData<PluginLoader::Plugin>::isForbiddenPlugin(file))
	{
		gLog << QObject::tr("Skipping loading of forbidden plugin: %1").arg(file);
		return;
	}

	// Load the library
	d->library.reset(new QLibrary(file));
	if (d->library->load())
	{
		auto doomSeekerABI = (unsigned int (*)())(d->library->resolve("doomSeekerABI"));
		if (!doomSeekerABI || doomSeekerABI() != DOOMSEEKER_ABI_VERSION)
		{
			// Unsupported version
			QString reason;
			if (doomSeekerABI != nullptr)
			{
				reason = QObject::tr(
					"plugin ABI version mismatch; plugin: %1, Doomseeker: %2").arg(
					doomSeekerABI()).arg(DOOMSEEKER_ABI_VERSION);
			}
			else
			{
				reason = QObject::tr("plugin doesn't report its ABI version");
			}
			gLog << QObject::tr("Cannot load plugin %1, reason: %2.").arg(file, reason);
			unload();
			return;
		}

		auto doomSeekerInit = (EnginePlugin * (*)())(d->library->resolve("doomSeekerInit"));
		if (doomSeekerInit == nullptr)
		{ // This is not a valid plugin.
			unload();
			return;
		}

		d->info = doomSeekerInit();
		if (!info()->data()->valid)
		{
			unload();
			return;
		}

		gLog << QObject::tr("Loaded plugin: \"%1\"!").arg(info()->data()->name);
		d->info->start();
	}
	else
	{
		gLog << QObject::tr("Failed to open plugin: %1").arg(file);
		gLog << QString("Last error was: %1").arg(d->library->errorString());
	}
}

PluginLoader::Plugin::~Plugin()
{
	unload();
}

void *PluginLoader::Plugin::function(const char *func) const
{
	return (void *) d->library->resolve(func);
}

EnginePlugin *PluginLoader::Plugin::info() const
{
	return d->info;
}

void PluginLoader::Plugin::initConfig()
{
	if (isValid())
	{
		IniSection cfgSection = gConfig.iniSectionForPlugin(info()->data()->name);
		info()->setConfig(cfgSection);
	}
}

bool PluginLoader::Plugin::isValid() const
{
	return d->library != nullptr && d->library->isLoaded();
}

void PluginLoader::Plugin::unload()
{
	d->library.reset();
}

////////////////////////////////////////////////////////////////////////////////
DClass<PluginLoader>
{
public:
	unsigned int type;
	QString pluginsDirectory;
	QList<PluginLoader::Plugin *> plugins;
};

DPointeredNoCopy(PluginLoader)

PluginLoader *PluginLoader::staticInstance = nullptr;

PluginLoader::PluginLoader(unsigned int type, const QStringList &directories)
{
	d->type = type;
	for (const QString &dir : directories)
	{
		d->pluginsDirectory = dir;
		if (filesInDir())
		{
			break;
		}
	}
	if (numPlugins() == 0) // No plugins?!
	{
		gLog << QObject::tr("Failed to locate plugins.");
	}
}

PluginLoader::~PluginLoader()
{
	qDeleteAll(d->plugins);
}

void PluginLoader::clearPlugins()
{
	qDeleteAll(d->plugins);
	d->plugins.clear();
}

void PluginLoader::deinit()
{
	if (staticInstance != nullptr)
	{
		delete staticInstance;
		staticInstance = nullptr;
	}
}

bool PluginLoader::filesInDir()
{
	gLog << QString("Attempting to load plugins from directory: %1").arg(d->pluginsDirectory);
	QDir dir(d->pluginsDirectory);
	if (!dir.exists())
	{
		return false;
	}
	#ifdef Q_OS_WIN32
	QStringList windowsNamesFilter;
	windowsNamesFilter << "*.dll";
	dir.setNameFilters(windowsNamesFilter);
	#endif
	for (const QString &entry : dir.entryList(QDir::Files))
	{
		QString pluginFilePath = Strings::combinePaths(d->pluginsDirectory, entry);
		Plugin *plugin = new Plugin(d->type, pluginFilePath);
		if (plugin->isValid())
			d->plugins << plugin;
		else
			delete plugin;
	}
	return numPlugins() != 0;
}

EnginePlugin *PluginLoader::info(int pluginIndex) const
{
	const Plugin *p = plugin(pluginIndex);
	if (p != nullptr)
	{
		return p->info();
	}
	return nullptr;
}

void PluginLoader::init(const QStringList &directories)
{
	if (staticInstance != nullptr)
	{
		qDebug() << "Attempting to re-init PluginLoader";
		assert(false);
		return;
	}
	staticInstance = new PluginLoader(MAKEID('E', 'N', 'G', 'N'), directories);
}

void PluginLoader::initConfig()
{
	for (Plugin *plugin : d->plugins)
	{
		plugin->initConfig();
	}
}

PluginLoader *PluginLoader::instance()
{
	assert(staticInstance != nullptr);
	return staticInstance;
}

unsigned int PluginLoader::numPlugins() const
{
	return d->plugins.size();
}

const QList<PluginLoader::Plugin *> &PluginLoader::plugins() const
{
	return d->plugins;
}

const PluginLoader::Plugin *PluginLoader::plugin(unsigned int index) const
{
	return d->plugins[index];
}

int PluginLoader::pluginIndexFromName(const QString &name) const
{
	// Why the mangling?
	// Ever since version 0.8.1b there was a bug that removed all spacebars
	// from plugin names. This bug is fixed in a commit made on 2013-11-01,
	// but the fix breaks at least some parts of configuration for plugins
	// that have spacebars in their names. For example, all server
	// configurations for Chocolate Doom won't load anymore. To prevent that,
	// we need to treat spacebars as non-existent here. Simply put:
	// "Chocolate Doom" == "ChocolateDoom"
	QString mangledName = QString(name).replace(" ", "");
	for (int i = 0; i < d->plugins.size(); ++i)
	{
		QString mangledCandidate = QString(d->plugins[i]->info()->data()->name).replace(" ", "");
		if (mangledName.compare(mangledCandidate) == 0)
		{
			return i;
		}
	}

	return -1;
}

void PluginLoader::resetPluginsDirectory(const QString &pluginsDirectory)
{
	d->pluginsDirectory = pluginsDirectory;
	clearPlugins();
	filesInDir();
}

const PluginLoader::Plugin *PluginLoader::operator[] (unsigned int index) const
{
	return d->plugins[index];
}
