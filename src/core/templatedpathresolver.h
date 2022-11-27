//------------------------------------------------------------------------------
// templatedpathresolver.h
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
// Copyright (C) 2022 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_TEMPLATED_PATH_RESOLVER_H
#define DOOMSEEKER_TEMPLATED_PATH_RESOLVER_H

#include "dptr.h"
#include "global.h"

#include <QString>
#include <QStringList>

/**
 * @ingroup group_pluginapi
 * @brief Resolver for paths that contain placeholders.
 *
 * The transformation of each placeholder type can be toggled by
 * appropriate setters.
 *
 * The allowed placeholders are:
 *
 * * `$PROGDIR` (capitals important) resolves to the directory where
 *   the application's executable resides.
 * * `$` character followed by a letter or underscore followed optionally
 *   by a string of letters, digits or underscores is treated as an env.
 *   variable and a resolution using such variable is attempted. If there's
 *   no matching env. variable, the placeholder resolves to an empty string.
 *   Regex: `$[a-zA-Z_][a-zA-Z0-9_]*`.
 * * `~` character at the beginning, followed by a directory delimiter
 *   or end of the string is replaced with current user's home directory.
 *   The `~someone` pattern will resolve to the home directory of "someone"
 *   but only on a UNIX system. On MS Windows, the `~` pattern is resolved
 *   but `~someone` is not.
 *
 * While Windows platform quotes the env. variables with `%`, this resolver
 * doesn't follow this pattern and uses the `$` prefix universally for all
 * platforms.
 */
class MAIN_EXPORT TemplatedPathResolver
{
public:
	TemplatedPathResolver();
	virtual ~TemplatedPathResolver();

	/**
	 * @brief Resolve a templated path.
	 *
	 * @return Path with all placeholders resolved as best as possible.
	 * The returned path is not normalized.
	 */
	QString resolve(const QString &templatedPath) const;

	/**
	 * @brief Convenience method, calls resolve(QString) for each element.
	 *
	 * @return List transformed by resolve(QString).
	 */
	QStringList resolve(const QStringList &templatedPaths) const;

	/**
	 * @brief Control the `$PROGDIR` resolution.
	 *
	 * If this is disabled, but the env. variable resolution is enabled,
	 * `$PROGDIR` will be treated like a common env. variable!
	 *
	 * Default: `false`
	 */
	bool progdirEnabled() const;
	void setProgdirEnabled(bool enabled);

	/**
	 * @brief Control the `$` env. variable resolution.
	 *
	 * Default: `false`
	 */
	bool envVarsEnabled() const;
	void setEnvVarsEnabled(bool enabled);

	/**
	 * @brief Control the resolution of both `~` and `~username`.
	 *
	 * Default: `true`
	 */
	bool userHomeEnabled() const;
	void setUserHomeEnabled(bool enabled);

private:
	DPtr<TemplatedPathResolver> d;
};

/**
 * @ingroup group_pluginapi
 * @brief Create the TemplatedPathResolver in accordance to Doomseeker's config.
 *
 * Construct the TemplatedPathResolver object internally considering the
 * current user configuration and resolve the passed path.
 */
MAIN_EXPORT TemplatedPathResolver gDoomseekerTemplatedPathResolver();

#endif
