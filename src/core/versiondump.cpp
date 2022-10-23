//------------------------------------------------------------------------------
// versiondump.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "versiondump.h"

#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "version.h"
#include <QIODevice>
#include <QJsonDocument>
#include <QString>
#include <wadseeker/wadseekerversioninfo.h>

class VersionDump::Module
{
public:
	Module(const QString &displayName, const QString &revision,
		const QString &displayVersion = QString())
	{
		_displayName = displayName;
		_revision = revision;
		_displayVersion = displayVersion;
	}

	QVariantMap toVariantMap()
	{
		QVariantMap result;
		result["display-name"] = _displayName;
		result["revision"] = _revision;

		if (!_displayVersion.isNull())
		{
			result["display-version"] = _displayVersion;
		}
		return result;
	}

private:
	QString _displayName;
	QString _revision;
	QString _displayVersion;
};

void VersionDump::dumpJsonToIO(QIODevice &io)
{
	QVariantMap root;
	Module doomseeker(Version::name(), QString::number(Version::revisionNumber()),
		Version::versionRevision());
	// "doomseeker" is legacy package and hardcoded to download version 1.1.
	root["doomseeker"] = Module("Doomseeker", "1496802976", "1.1-170607-0236").toVariantMap();
	root["doomseeker-core"] = doomseeker.toVariantMap();
	root["wadseeker"] = Module("Wadseeker", WadseekerVersionInfo::version(),
		WadseekerVersionInfo::version()).toVariantMap();
	root["qt"] = Module("Qt", Version::qtPackageVersion()).toVariantMap();
	for (unsigned int i = 0; i < gPlugins->numPlugins(); ++i)
	{
		const PluginLoader::Plugin *plugin = gPlugins->plugin(i);
		QString name = plugin->info()->data()->name;
		QString keyword = "p-" + name.toLower().replace(" ", "");
		root[keyword] = Module(name, QString::number(plugin->info()->data()->version)).toVariantMap();
	}

	io.write(QJsonDocument::fromVariant(root).toJson());
	io.write("\n");
}
