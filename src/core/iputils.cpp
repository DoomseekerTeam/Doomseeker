//------------------------------------------------------------------------------
// iputils.cpp
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
#include "iputils.h"

#include <algorithm>
#include <QHostAddress>

bool IpUtils::isIPv4Mapped(const QHostAddress &address)
{
	// toIPv6Address() maps native IPv4 addresses too, but those are not IPv6.
	if (address.protocol() != QAbstractSocket::IPv6Protocol)
		return false;

	// Check the mapped prefix explicitly using Qt 5.7 APIs:
	// toIPv4Address(&ok) also accepts :: as 0.0.0.0, but that is genuine IPv6.
	// The prefix also handles ::ffff:0.0.0.0, whose converted IPv4 value is
	// zero just like a failed conversion.
	const Q_IPV6ADDR ipv6 = address.toIPv6Address();
	const quint8 mappedPrefix[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
	return std::equal(mappedPrefix, mappedPrefix + sizeof(mappedPrefix), ipv6.c);
}
