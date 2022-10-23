//------------------------------------------------------------------------------
// commandlinetokenizer.h
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
#ifndef idFC7A2485_BC0B_42CB_A9A3892937F60D8D
#define idFC7A2485_BC0B_42CB_A9A3892937F60D8D

#include "dptr.h"

#include <QString>
#include <QStringList>

/**
 * @brief Splits command line into separate arguments in a manner appropriate
 *        for current OS.
 *
 * - On Windows WinAPI CommandLineToArgvW() is used.
 * - On other systems Scanner is used. Only double quotation mark (") is
 *   supported. Single quotation mark is treated like a normal character.
 */
class CommandLineTokenizer
{
public:
	QStringList tokenize(const QString &cmdLine);

private:
	class PrivData;
};

#endif
