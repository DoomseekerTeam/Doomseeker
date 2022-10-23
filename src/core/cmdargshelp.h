//------------------------------------------------------------------------------
// cmdargshelp.h
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
#ifndef id68df3b4b_d3e4_43df_8456_bb074b01d1b9
#define id68df3b4b_d3e4_43df_8456_bb074b01d1b9

#include <QObject>

class CmdArgsHelp : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Returns all the options that Doomseeker accepts,
	 *        excluding the ones for internal use.
	 */
	static QString argsHelp();

	/**
	 * @brief Prepends "Available command line parameters" to
	 *        argsHelp().
	 */
	static QString availableCommands();

	/**
	 * @brief Returns a string informing about the lack of arguments,
	 *        followed by availableCommands().
	 *
	 * @param expectedArguments Number of expected arguments.
	 * @param option Option that doesn't have enough arguments.
	 */
	static QString missingArgs(int expectedArguments, QString option);

	/**
	 * @brief Returns a string informing about the use of an unknown
	 *        option, followed by availableCommands().
	 *
	 * @param option Unknown option.
	 */
	static QString unrecognizedOption(QString option);
};

#endif
