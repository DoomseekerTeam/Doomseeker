//------------------------------------------------------------------------------
// commandline.h
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
#ifndef DOOMSEEKER_COMMANDLINE_H
#define DOOMSEEKER_COMMANDLINE_H

#include <QString>
#include <QStringList>
#include <Qt>

/**
 * @brief Class used for manipulating command line as it should appear in
 *        shell or cmd prompt.
 *
 * This can be used to escape command line arguments in a way that they are
 * ready to be copied by the user and pasted into the cmd line prompt.
 */
class CommandLine
{
public:
	/**
	 * @brief Escapes all characters in all strings on the list.
	 *
	 * The passed list is modified directly. No elements are added or
	 * removed but the existing elements may change.
	 *
	 * The function behaves differently depending on the OS the program
	 * was compiled for.
	 */
	static void escapeArgs(QStringList &args);

	/**
	 * @brief Escapes all characters in the passed string.
	 *
	 * The string is modified directly.
	 *
	 * The function behaves differently depending on the OS the program
	 * was compiled for.
	 */
	static void escapeArg(QString &arg);

	/**
	 * @brief Escapes the executable path and handles OS X bundles.
	 */
	static void escapeExecutable(QString &arg);
};

#endif
