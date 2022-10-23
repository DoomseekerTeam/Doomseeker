//------------------------------------------------------------------------------
// hash.h
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
// Copyright (C) 2018 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------

#ifndef WADSEEKER_HASH_H
#define WADSEEKER_HASH_H

#include "../wadseekerexportinfo.h"
#include "../dptr.h"
#include <QCryptographicHash>
#include <QFile>
#include <QString>
/**
 * @brief Handles the required algorithms to hash files
 */
class WADSEEKER_API Hash
{
public:
	/**
	 * @brief Hashes files
	 *
	 * @param path - Path to the desired file to hash
	 * @param algorithm - Algorithm to use
	 * @return hash of file
	 */
	static QByteArray hashFile(const QString &path, const QCryptographicHash::Algorithm algorithm);
};

#endif
