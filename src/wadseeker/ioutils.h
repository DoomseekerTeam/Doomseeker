//------------------------------------------------------------------------------
// ioutils.h
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
#ifndef __IOUTILS_H__
#define __IOUTILS_H__

#include <climits>
#include <QIODevice>

class IOUtils
{
public:
	/**
	 * @brief Copies contents of one QIODevice to the other.
	 *
	 * @b Note:
	 * IODevices must be open before the method is called.
	 * IODevices are not closed when copy operation finishes.
	 * There are no seek operations performed. Data is read from src
	 * current position and written to dst current position.
	 *
	 * @param src
	 *      Source QIODevice. Must be open for reading.
	 * @param dst
	 *      Target QIODevice. Must be open for writing.
	 * @param maxCount
	 *      Maximum number of bytes copied from src to dst.
	 *      Default value: ULLONG_MAX
	 * @param bufferSize
	 *      Memory buffer size. How many bytes will be copied in one burst.
	 *      Default value: 4MB
	 *
	 * @return true on success.
	 */
	static bool copy(QIODevice &src, QIODevice &dst, unsigned long long maxCount = ULLONG_MAX, unsigned bufferSize = 4 * 1024 * 1024);
};

#endif
