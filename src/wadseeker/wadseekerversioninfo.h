//------------------------------------------------------------------------------
// wadseekerversioninfo.h
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
#ifndef __WADSEEKERVERSIONINFO_H__
#define __WADSEEKERVERSIONINFO_H__

#include "wadseekerexportinfo.h"
#include <QString>
#include <QStringList>

class WADSEEKER_API WadseekerVersionInfo
{
public:
	/**
	 * @brief Author of the library.
	 */
	static QString author();

	/**
	 * @brief Description of the library.
	 */
	static QString description();

	/**
	 * @brief List of known extensions for "WAD" files (pk3, wad).
	 */
	static QStringList knownWadExtensions();

	/**
	 * @brief Checks if passed argument is on
	 *        the supportedArchiveExtensions() list.
	 *
	 * Check is case-insensitive.
	 *
	 * @return True if suffix is on the list.
	 */
	static bool isSupportedArchiveExtension(const QString &suffix);

	/**
	 * @brief List of extractable archive extensions.
	 */
	static QStringList supportedArchiveExtensions();

	/**
	 * @brief WWW User Agent used for HTTP communications.
	 */
	static QString userAgent();

	/**
	 * @brief Version string of the library.
	 */
	static QString version();

	/**
	 * @brief Development dates.
	 */
	static QString yearSpan();
};

#endif
