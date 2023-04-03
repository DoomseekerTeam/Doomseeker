//------------------------------------------------------------------------------
// addressfilter.h
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
// Copyright (C) 2023 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef GUI_ENTITY_ADDRESSFILTER_H
#define GUI_ENTITY_ADDRESSFILTER_H

#include <QHostAddress>
#include <QVariant>

class AddressFilter final
{
public:
	static AddressFilter deserialize(const QVariant &variant);
	QVariant serialize() const;

	bool isEmpty() const;

	/**
	 * Check if the given address matches any subnet in the filter.
	 *
	 * If the filter is empty, all addresses are matched.
	 */
	bool matches(const QHostAddress &address) const;

	/**
	 * Add a subnet to the address filter.
	 */
	AddressFilter &operator<<(const QPair<QHostAddress, int> &subnet);

	bool operator==(const AddressFilter &other) const;
	bool operator!=(const AddressFilter &other) const;

	/**
	 * Comma separated list of subnets.
	 */
	QString toString() const;

private:
	QList<QPair<QHostAddress, int>> subnets;
};

#endif
