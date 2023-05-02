//------------------------------------------------------------------------------
// ip2crange.h
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
// Copyright (C) 2021 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_IP2C_IP2CRANGE_H
#define DOOMSEEKER_IP2C_IP2CRANGE_H

#include <QString>
#include <cstdint>

struct IP2CRange
{
	uint32_t ipStart;
	uint32_t ipEnd;
	QString country;

	IP2CRange()
	{
		ipStart = ipEnd = 0;
	}

	/**
	 * IP2CRange is valid when ipStart is different from
	 * ipEnd.
	 */
	bool isValid() const
	{
		return ipStart != ipEnd;
	}
};

#endif
