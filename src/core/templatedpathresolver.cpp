//------------------------------------------------------------------------------
// templatedpathresolver.cpp
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
#include "templatedpathresolver.h"

#include "configuration/doomseekerconfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>

#include <utility>

#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <pwd.h>
#endif

const QString TemplatedPathResolver::PROGDIR_TEMPLATE = "$PROGDIR";

DClass<TemplatedPathResolver>
{
public:
	bool progdirEnabled;
	bool envVarsEnabled;
	bool userHomeEnabled;
};

DPointered(TemplatedPathResolver)

TemplatedPathResolver::TemplatedPathResolver()
{
	d->progdirEnabled = false;
	d->envVarsEnabled = false;
	d->userHomeEnabled = true;
}

TemplatedPathResolver::~TemplatedPathResolver()
{
}

QString TemplatedPathResolver::resolve(const QString &templatedPath) const
{
	QString resolved = templatedPath;

	// Match ~ to home dir.
	if (d->userHomeEnabled && resolved.startsWith("~"))
	{
#ifdef Q_OS_WIN
		const QRegularExpression separatorRegex(R"([/\\])");
#else
		const QRegularExpression separatorRegex("/");
#endif
		int separatorIdx = resolved.indexOf(separatorRegex);

		QString username = resolved.mid(1, separatorIdx - 1);
		QString home;
		if (username.isEmpty())
		{
			home = QDir::homePath();
		}
		else
		{
			home = QString("~%1").arg(username);
#ifdef DOOMSEEKER_TEMPLATED_PATH_RESOLVER_TILDEUSER
			const struct passwd *pwent = getpwnam(username.toLatin1().constData());
			if (pwent != nullptr)
			{
				home = pwent->pw_dir;
			}
#endif
		}
		if (separatorIdx > 0)
		{
			resolved = home + resolved.mid(separatorIdx);
		}
		else
		{
			resolved = home;
		}
	}

	// Match arbitrary env. vars.
	QRegularExpression envVarRegex("\\$([a-zA-Z_][a-zA-Z0-9_]*)");
	int offset = 0;
	while (d->progdirEnabled || d->envVarsEnabled)
	{
		QRegularExpressionMatch match;
		int varIdx = resolved.indexOf(envVarRegex, offset, &match);
		if (varIdx < 0)
			break;
		QString name = match.captured(1);
		QString varValue = "";

		// $PROGDIR is a special placeholder that always resolves to the directory
		// where executable is stored, even if an env. var with the same name exists.
		if (d->progdirEnabled && name == PROGDIR_TEMPLATE.mid(1))
		{
			varValue = QCoreApplication::applicationDirPath();
		}
		else if (d->envVarsEnabled && QProcessEnvironment::systemEnvironment().contains(name))
		{
			varValue = qgetenv(name.toUtf8());
		}
		else if (!d->envVarsEnabled)
		{
			varValue = QString("$%1").arg(name);
		}
		resolved = resolved.left(varIdx) + varValue + resolved.mid(varIdx + match.capturedLength());
		offset = varIdx + varValue.length();
	}
	return resolved;
}

QStringList TemplatedPathResolver::resolve(const QStringList &templatedPaths) const
{
	QStringList transformed;
	for (const QString &path : templatedPaths)
		transformed << resolve(path);
	return transformed;
}

bool TemplatedPathResolver::progdirEnabled() const
{
	return d->progdirEnabled;
}

void TemplatedPathResolver::setProgdirEnabled(bool enabled)
{
	d->progdirEnabled = enabled;
}

bool TemplatedPathResolver::envVarsEnabled() const
{
	return d->envVarsEnabled;
}

void TemplatedPathResolver::setEnvVarsEnabled(bool enabled)
{
	d->envVarsEnabled = enabled;
}

bool TemplatedPathResolver::userHomeEnabled() const
{
	return d->userHomeEnabled;
}

void TemplatedPathResolver::setUserHomeEnabled(bool enabled)
{
	d->userHomeEnabled = enabled;
}

// Functions

TemplatedPathResolver gDoomseekerTemplatedPathResolver()
{
	TemplatedPathResolver resolver;
	resolver.setEnvVarsEnabled(gConfig.doomseeker.bResolveTemplatedPathsPlaceholders);
	resolver.setProgdirEnabled(gConfig.doomseeker.bResolveTemplatedPathsPlaceholders);
	resolver.setUserHomeEnabled(true);
	return resolver;
}
