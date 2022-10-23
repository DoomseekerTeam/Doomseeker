//------------------------------------------------------------------------------
// version.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __VERSION_H__
#define __VERSION_H__

#include "global.h"
#include <QString>

/**
 * @ingroup group_pluginapi
 * @brief Provides interface for plugins and for Doomseeker itself
 * to obtain version information on the program.
 */
class MAIN_EXPORT Version
{
public:
	/**
	 * Returns Git changeset or an empty string if not available.
	 */
	static QString changeset();

	/**
	 * @brief Combines program's name and versionRevision().
	 */
	static QString fullVersionInfo() { return name() + " " + versionRevision(); }
	/**
	 * @brief Combines program's name, versionRevision()
	 *        and operatingSystem().
	 */
	static QString fullVersionInfoWithOs()
	{
		return name() + " " + versionRevision() + " " + operatingSystem();
	}
	static QString name();
	/**
	 * @brief Name of operating system, content length and detail may vary.
	 */
	static QString operatingSystem();
	static QString qtPackageVersion();
	static QString revision();
	static unsigned long long revisionNumber();

	/**
	 * @brief WWW User Agent used for HTTP communications.
	 */
	static QString userAgent();

	static QString version();

	/**
	 * @brief Combines version and revision strings.
	 *
	 * If revision is not available only version is returned.
	 */
	static QString versionRevision();

	static QString yearSpan();
};

#endif
