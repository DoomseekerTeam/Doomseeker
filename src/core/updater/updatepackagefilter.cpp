//------------------------------------------------------------------------------
// updatepackagefilter.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "updatepackagefilter.h"

#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "strings.hpp"
#include "updater/autoupdater.h"
#include "version.h"
#include <cassert>
#include <QCoreApplication>
#include <QFile>
#include <wadseeker/wadseekerversioninfo.h>

class UpdatePackageFilter::PluginInfo
{
public:
	QString name;
	QString revision;
};
//////////////////////////////////////////////////////////////////////////////
DClass<UpdatePackageFilter>
{
public:
	bool bWasAnyUpdatePackageIgnored;
	QMap<QString, QList<QString> > ignoredPackagesRevisions;
	QMap<QString, UpdatePackageFilter::PluginInfo> plugins;

	bool hasMainProgramPackage(const QList<UpdatePackage> &packages) const
	{
		for (const UpdatePackage &pkg : packages)
		{
			if (pkg.name == AutoUpdater::MAIN_PROGRAM_PACKAGE_NAME)
			{
				return true;
			}
		}
		return false;
	}
};

DPointered(UpdatePackageFilter)
//////////////////////////////////////////////////////////////////////////////
UpdatePackageFilter::UpdatePackageFilter()

{
	d->bWasAnyUpdatePackageIgnored = false;
}

UpdatePackageFilter::~UpdatePackageFilter()
{
}

QMap<QString, UpdatePackageFilter::PluginInfo> UpdatePackageFilter::collectPluginInfo()
{
	QMap<QString, PluginInfo> infos;
	const QList<PluginLoader::Plugin *> plugins = gPlugins->plugins();
	for (const PluginLoader::Plugin *plugin : plugins)
	{
		PluginInfo pluginInfo;
		pluginInfo.name = plugin->info()->data()->name;
		pluginInfo.revision = QString::number(plugin->info()->data()->version);
		QString prefixedName = AutoUpdater::PLUGIN_PREFIX + pluginInfo.name.toLower().replace(" ", "");
		infos.insert(prefixedName, pluginInfo);
	}
	return infos;
}

QList<UpdatePackage> UpdatePackageFilter::filter(const QList<UpdatePackage> &packages)
{
	QList<UpdatePackage> filtered;
	d->plugins = collectPluginInfo();
	QList<UpdatePackage> packagesOnIgnoredList;
	for (UpdatePackage pkg : packages)
	{
		if (isDifferentThanInstalled(pkg))
		{
			if (!isOnIgnoredList(pkg.name, pkg.revision))
			{
				filtered << pkg;
			}
			else
			{
				packagesOnIgnoredList << pkg;
			}
		}
	}
	if (!filtered.isEmpty())
	{
		// If we do an update of at least one package, we also need to update
		// all packages that were previously ignored.
		filtered.append(packagesOnIgnoredList);
		packagesOnIgnoredList.clear();
	}
	d->bWasAnyUpdatePackageIgnored = !packagesOnIgnoredList.isEmpty();
	return filtered;
}

bool UpdatePackageFilter::isDifferentThanInstalled(UpdatePackage &pkg) const
{
	if (pkg.name == AutoUpdater::MAIN_PROGRAM_PACKAGE_NAME)
	{
		QString localRevision = QString::number(Version::revisionNumber());
		if (localRevision != pkg.revision)
		{
			pkg.currentlyInstalledDisplayVersion = Version::versionRevision();
			return true;
		}
	}
	else if (pkg.name == AutoUpdater::WADSEEKER_PACKAGE_NAME)
	{
		QString localRevision = WadseekerVersionInfo::version();
		if (localRevision != pkg.revision)
		{
			pkg.currentlyInstalledDisplayVersion = localRevision;
			return true;
		}
	}
	else if (pkg.name == AutoUpdater::QT_PACKAGE_NAME)
	{
		// For OS X PowerPC is no longer getting Qt updates so it will
		// always be on 4.8.  Similarly i386 will be used as a legacy
		// platform (the number of 32-bit only Intel Macs is tiny and
		// the version of Qt5 we're launching with requires Lion or
		// higher.  Thus only 64-bit Intel Macs need to even bother
		// checking this package.
		#if !defined(Q_OS_DARWIN) || defined(__x86_64__)
		if (QString(Version::qtPackageVersion()) != pkg.revision)
		{
			pkg.currentlyInstalledDisplayVersion = Version::qtPackageVersion();
			return true;
		}
		// Workaround for auto-updater bug that made itself apparent
		// in Doomseeker 1.1~beta builds. Bug caused every even
		// numbered file in auxiliary packages not to be installed. As
		// Qt is such auxiliary package in 1.1~beta, some files
		// failed to install.
		bool checkBrokenQt = false;
		#ifdef Q_OS_WIN32
		checkBrokenQt = true;
		#endif
		if (checkBrokenQt && !isQtInstallOk())
		{
			pkg.currentlyInstalledDisplayVersion = Version::qtPackageVersion() + tr("-BROKEN");
			return true;
		}
		#endif
	}
	else
	{
		// Plugin node.
		if (d->plugins.contains(pkg.name))
		{
			PluginInfo pluginInfo = d->plugins[pkg.name];
			if (pluginInfo.revision != pkg.revision)
			{
				pkg.currentlyInstalledDisplayVersion = pluginInfo.revision;
				return true;
			}
		}
	}
	return false;
}

bool UpdatePackageFilter::isQtInstallOk() const
{
	QStringList files;
	files << "libeay32.dll" << "ssleay32.dll";
	for (const QString &filename : files)
	{
		QString fileLocation = Strings::combinePaths(
			QCoreApplication::applicationDirPath(), filename);
		QFile file(fileLocation);
		if (!file.exists())
		{
			return false;
		}
	}
	return true;
}

bool UpdatePackageFilter::isOnIgnoredList(const QString &package, const QString &revision) const
{
	const QList<QString> &list = d->ignoredPackagesRevisions[package];
	return list.contains(revision);
}

void UpdatePackageFilter::setIgnoreRevisions(const QMap<QString, QList<QString> > &packagesRevisions)
{
	d->ignoredPackagesRevisions = packagesRevisions;
}

bool UpdatePackageFilter::wasAnyUpdatePackageIgnored() const
{
	return d->bWasAnyUpdatePackageIgnored;
}
