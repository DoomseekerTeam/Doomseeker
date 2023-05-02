//------------------------------------------------------------------------------
// addressfilter.cpp
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
#include "addressfilter.h"

bool AddressFilter::contains(const QPair<QHostAddress, int> &subnet) const
{
	return subnets.contains(subnet);
}

bool AddressFilter::isEmpty() const
{
	return subnets.isEmpty();
}

bool AddressFilter::matches(const QHostAddress &address) const
{
	if (subnets.isEmpty())
		return true;

	for (const auto &subnet : subnets)
	{
		if (address.isInSubnet(subnet))
			return true;
	}
	return false;
}

AddressFilter &AddressFilter::operator<<(const QPair<QHostAddress, int> &subnet)
{
	subnets.append(subnet);
	return *this;
}

bool AddressFilter::operator==(const AddressFilter &other) const
{
	return subnets == other.subnets;
}

bool AddressFilter::operator!=(const AddressFilter &other) const
{
	return subnets != other.subnets;
}

AddressFilter AddressFilter::deserialize(const QVariant &variant)
{
	AddressFilter out;

	QVariantList subnetsVariantList = variant.toList();
	for (const QVariant &subnetVariant : subnetsVariantList)
	{
		QVariantMap subnetVariantMap = subnetVariant.toMap();
		QPair<QHostAddress, int> subnet;
		subnet.first = QHostAddress(subnetVariantMap["address"].toString());
		subnet.second = subnetVariantMap["mask"].toInt();
		if (!subnet.first.isNull())
			out << subnet;
	}

	return out;
}

QVariant AddressFilter::serialize() const
{
	QVariantList result;
	for (const auto &subnet : subnets)
	{
		QVariantMap pair;
		pair["address"] = subnet.first.toString();
		pair["mask"] = subnet.second;
		result << pair;
	}
	return result;
}

QString AddressFilter::toString() const
{
	QStringList tokens;
	for (const auto &subnet : subnets)
	{
		tokens << QString("%1/%2").arg(subnet.first.toString()).arg(subnet.second);
	}
	return tokens.join(",");
}
