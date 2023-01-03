//------------------------------------------------------------------------------
// cmdargshelp.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "cmdargshelp.h"

QString CmdArgsHelp::argsHelp()
{
#ifdef Q_OS_WIN
	const QString mountedWhere = tr("being assigned a different letter");
#else
	const QString mountedWhere = tr("being mounted on a different path");
#endif
	QString help = "";
	help += tr(
		"--basedir <directory>\n"
		"    Changes the default storage directory to the specified\n"
		"    one (also in --portable mode). Doomseeker stores all of its\n"
		"    settings, cache and managed files in this directory.\n");
	help += tr(
		"--connect <protocol://ip[:port]>\n"
		"    Attempts to connect to the specified server.\n");
	help += tr(
		"--create-game\n"
		"    Launches Doomseeker in \"Create Game\" mode.\n");
	help += tr(
		"--datadir <directory>\n"
		"    Sets an explicit search location for\n"
		"    IP2C data along with plugins.\n"
		"    Can be specified multiple times.\n");
	help += tr(
		"--help\n"
		"    Prints this list of command line arguments.\n");
	help += tr(
		"--rcon [plugin] [ip]\n"
		"    Launch the rcon client for the specified ip.\n");
	help += tr(
		"--portable\n"
		"    Starts the application in the portable mode:\n"
		"    - Doomseeker saves all the configuration files to the directory\n"
		"      where its executable resides. Normally, configuration is saved to\n"
		"      user's home directory. This directory can be changed with --basedir.\n"
		"    - The current working directory is forced to the directory where\n"
		"      Doomseeker's executable resides.\n"
		"    - Doomseeker will save in the configuration all paths as relative\n"
		"      in anticipation that they may change between the runs, for ex.\n"
		"      due to the portable device %1.\n").arg(mountedWhere);
	help += tr(
		"--version-json [file|-]\n"
		"    Prints version info on Doomseeker and all\n"
		"    plugins in JSON format to specified file,\n"
		"    then closes the program. If file is not\n"
		"    specified or specified as '-', version info\n"
		"    is printed to stdout.\n");
	help += tr(
		"--verbose\n"
		"    Forces verbose logging to stderr.\n"
		"    This is the default in most cases.\n");
	help += tr(
		"--quiet\n"
		"    Forces no logging to stderr.\n"
		"    This is the default when dumping versions.\n");
	return help;
}

QString CmdArgsHelp::availableCommands()
{
	QString help = tr("Available command line parameters:\n");
	help += argsHelp();
	return help;
}

QString CmdArgsHelp::missingArgs(int expectedArguments, QString option)
{
	QString help = tr("doomseeker: expected %n argument(s) in option %1\n\n", nullptr, expectedArguments).arg(option);
	help += availableCommands();
	return help;
}

QString CmdArgsHelp::unrecognizedOption(QString option)
{
	QString help = tr("doomseeker: unrecognized option '%1'\n\n").arg(option);
	help += availableCommands();
	return help;
}
