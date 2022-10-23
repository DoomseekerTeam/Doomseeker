//------------------------------------------------------------------------------
// utf8splitter.h
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
#ifndef id8078ABFA_1AE4_4D52_A117FE6ABC7067E9
#define id8078ABFA_1AE4_4D52_A117FE6ABC7067E9

#include <QByteArray>
#include <QList>

/**
 * @brief Splits valid arrays of UTF-8 characters into smaller arrays without
 *        splitting in-between chars.
 */
class Utf8Splitter
{
public:
	QList<QByteArray> split(const QByteArray &in, int maxChunkLength);
};

#endif
