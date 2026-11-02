//------------------------------------------------------------------------------
// iputils.h
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
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_IPUTILS_H
#define DOOMSEEKER_IPUTILS_H

class QHostAddress;

/**
 * IP address utilities.
 */
class IpUtils
{
public:
	/**
	 * Check whether an IPv6 address has the IPv4-mapped prefix `::ffff:0:0/96`.
	 *
	 * Native IPv4 addresses, null addresses, and genuine IPv6 addresses
	 * (including `::` and `::1`) return false. `::ffff:0.0.0.0` returns true.
	 */
	static bool isIPv4Mapped(const QHostAddress &address);
};

#endif
