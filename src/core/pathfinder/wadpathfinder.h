//------------------------------------------------------------------------------
// wadpathfinder.h
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
#ifndef idCA954A5A_EF00_4F51_9C9782634F191143
#define idCA954A5A_EF00_4F51_9C9782634F191143

#include "dptr.h"
#include "serverapi/serverptr.h"
#include <QString>
#include <QStringList>

class PathFinder;

class WadFindResult
{
public:
	WadFindResult();
	virtual ~WadFindResult();

	const QString &alias() const;
	void setAlias(const QString &val);

	bool isAlias() const;
	bool isValid() const;

	const QString &path() const;
	void setPath(const QString &val);

private:
	DPtr<WadFindResult> d;
};

/**
 * @brief Wrapper for PathFinder that specializes in findings WADs.
 *
 * By WAD we understand any extension file that can be loaded by the
 * game, so not just .wad but also .pk3, .7z, .deh, etc.
 *
 * WadPathFinder accepts an already configured PathFinder in its
 * constructor. Usually, a default-constructed PathFinder already loads
 * the paths from user configuration, however some callers, including
 * plugins, can also add other paths to PathFinder. This allows
 * customization of what paths will be searched depending on the
 * context.
 *
 * WadPathFinder considers WAD aliases configured by the user, such
 * as freedoom2.wad being a valid replacement for doom2.wad. It also
 * considers additional OS paths for WAD finding, if given OS has
 * reasonable defaults for WAD files. As WadPathFinder is not
 * MAIN_EXPORT, these features are outside of plugin control.
 *
 * Additional paths appended on Linux:
 *
 * 1. `/usr/local/share/games/doom/`
 * 2. `/usr/share/games/doom/`
 */
class WadPathFinder
{
public:
	WadPathFinder(PathFinder pathFinder);
	~WadPathFinder();

	WadFindResult find(const QString &name);

	/**
	 * @brief Can disable WAD aliasing for contexts where only
	 * specific WADs should be found.
	 */
	void setAllowAliases(bool allowed);

private:
	DPtr<WadPathFinder> d;

	QStringList aliases(const QString &name) const;
};

WadFindResult findWad(ServerPtr server, const QString &wadName);

#endif
