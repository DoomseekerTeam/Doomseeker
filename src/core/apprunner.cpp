//------------------------------------------------------------------------------
// apprunner.cpp
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
#include "apprunner.h"

#include "gui/standardserverconsole.h"
#include "log.h"
#include "serverapi/message.h"
#include "strings.hpp"
#include <QProcess>

bool CommandLineInfo::isValid() const
{
	return !executable.filePath().isEmpty();
}

void AppRunner::cleanArguments(QStringList &args)
{
	QStringList::iterator it;
	for (it = args.begin(); it != args.end(); ++it)
	{
		if (it->contains(" "))
			Strings::trim(*it, "\"");
	}
}

#ifdef Q_OS_DARWIN
QString AppRunner::findBundleBinary(const QFileInfo &file)
{
	// Scan the plist file for where the real binary is in a bundle.  We have
	// to do this because some bundles (ZDaemon) don't like the --args method
	// and I heard that only works on Mac OS X 10.6 anywyas.
	QFile pLists(file.canonicalFilePath() + "/Contents/Info.plist");
	if (!pLists.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		gLog << tr("Could not read bundle plist. (%1)").arg(file.canonicalFilePath() + "/Contents/Info.plist");
		return QString();
	}

	char line[128];
	bool keyFound = false;
	while (pLists.readLine(line, 128) != -1)
	{
		if (!keyFound)
		{
			if (QString(line).trimmed() == "<key>CFBundleExecutable</key>")
				keyFound = true;
		}
		else
		{
			QString binaryLine(line);
			binaryLine = binaryLine.trimmed();
			if (binaryLine.startsWith("<string>") && binaryLine.endsWith("</string>"))
				return QString("/Contents/MacOS/") + binaryLine.mid(8, binaryLine.indexOf("</string>") - 8);
			keyFound = false;
		}
	}
	return QString();
}
#endif

Message AppRunner::runExecutable(const CommandLineInfo &cmdInfo)
{
	gLog << tr("Starting (working dir %1): %2").arg(cmdInfo.applicationDir.absolutePath()).arg(cmdInfo.executable.absoluteFilePath());
	QStringList args = cmdInfo.args;
	cleanArguments(args);

	int result;

	#ifdef Q_OS_DARWIN
	if ( cmdInfo.executable.isBundle())
		result = QProcess::startDetached(cmdInfo.executable.absoluteFilePath() + AppRunner::findBundleBinary(cmdInfo.executable), args, cmdInfo.applicationDir.absolutePath());
	else
	#endif
	{
		result = QProcess::startDetached(cmdInfo.executable.absoluteFilePath(), args, cmdInfo.applicationDir.absolutePath());
	}

	Message message;

	if (!result)
	{
		QString error = tr("File: %1\ncannot be run").arg(cmdInfo.executable.absoluteFilePath());
		gLog << error;
		message = Message::customError(error);
		return message;
	}

	return message;
}

void AppRunner::runExecutableWrappedInStandardServerConsole(
	const QIcon &icon, const CommandLineInfo &cli)
{
	gLog << tr("Starting (working dir %1): %2").arg(cli.applicationDir.absolutePath())
		.arg(cli.executable.absoluteFilePath());
	QStringList args = cli.args;
	cleanArguments(args);
	new StandardServerConsole(icon, cli.executable.absoluteFilePath(), args);
}
