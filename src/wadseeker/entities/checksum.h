//------------------------------------------------------------------------------
// checksum.h
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

#ifndef WADSEEKER_CHECKSUM_H
#define WADSEEKER_CHECKSUM_H

#include "../wadseekerexportinfo.h"
#include "../dptr.h"
#include <QByteArray>
#include <QCryptographicHash>

/**
 * @brief Class to store a hash and the algorithm used.
 */
class WADSEEKER_API Checksum
{
public:
	Checksum(const QByteArray &hash, const QCryptographicHash::Algorithm &algorithm);
	Checksum(const Checksum &other);
	virtual ~Checksum();
	Checksum &operator=(const Checksum &other);

	/**
	 * @brief Hash of a file generated with the algorithm.
	 */
	const QByteArray &hash() const;
	/**
	 * @brief Algorithm used to generate the hash.
	 */
	const QCryptographicHash::Algorithm &algorithm() const;

private:
	DPtr<Checksum> d;
};

#endif
