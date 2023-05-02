//------------------------------------------------------------------------------
// updatepackagefilter.h
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
#ifndef DOOMSEEKER_UPDATER_UPDATERPACKAGEFILTER_H
#define DOOMSEEKER_UPDATER_UPDATERPACKAGEFILTER_H

#include "dptr.h"
#include "updater/updatechannel.h"
#include "updater/updatepackage.h"
#include <QList>
#include <QMap>
#include <QString>

/**
 * @brief Filters UpdatePackage information basing on what is requested
 *        by the program.
 *
 * Firstly packages are filtered by specified channel name: setChannel().
 * All packages which do not fit this criteria are discarded.
 *
 * Next these filters are applied:
 * - Packages which are told to be ignored through setIgnoreRevisions()
 *   method. @b However if at least one package goes through the filters
 *   then this list is itself ignored. The ignore list by itself is designed
 *   to prevent nagging users for updates which they already discarded.
 *   But if an update for one of the package appears it's probable
 *   that it may require update for other packages as well due to incompatible
 *   interfaces between the main program and the plugins.<br>
 *   @b TL;DR: If we update one of them, then we'll be only safe if we update
 *   all of them.
 * - Packages which have the same revision number as the ones already
 *   installed are discarded. This is hardcoded into the class.
 */
class UpdatePackageFilter : public QObject
{
	Q_OBJECT

public:
	UpdatePackageFilter();
	~UpdatePackageFilter() override;

	QList<UpdatePackage> filter(const QList<UpdatePackage> &packages);
	void setIgnoreRevisions(const QMap<QString, QList<QString> > &packagesRevisions);
	/**
	 * @brief After filter() flag which says if any package was ignored.
	 *
	 * This flag is set to correct value after a call to filter().
	 * If true then at least one package was ignored due to being on
	 * setIgnoreRevisions() list.
	 *
	 * Note that if package was on ignore list but the list itself was
	 * ignored because of other valid update packages which weren't on
	 * it then this will return false. This is consistent with
	 * the description of this class.
	 *
	 * @return true if at least one package was discarded.
	 *         false if no package was discarded.
	 */
	bool wasAnyUpdatePackageIgnored() const;

private:
	Q_DISABLE_COPY(UpdatePackageFilter)

	class PluginInfo;

	DPtr<UpdatePackageFilter> d;
	friend class PrivData<UpdatePackageFilter>;

	QMap<QString, PluginInfo> collectPluginInfo();
	bool isDifferentThanInstalled(UpdatePackage &pkg) const;
	bool isOnIgnoredList(const QString &package, const QString &revision) const;
	bool isQtInstallOk() const;
};

#endif
