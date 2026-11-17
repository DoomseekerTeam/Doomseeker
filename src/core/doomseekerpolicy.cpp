//------------------------------------------------------------------------------
// doomseekerpolicy.cpp
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
#include "doomseekerpolicy.h"

#include <QtGlobal>

namespace
{
const DoomseekerPolicy *installedPolicy = nullptr;
}

DoomseekerPolicy::DoomseekerPolicy(bool autoUpdate, bool ip2cUpdate, bool ip2cAutoUpdate)
	: autoUpdateAllowed(autoUpdate), ip2cUpdateAllowed(ip2cUpdate),
	  ip2cAutoUpdateAllowed(ip2cAutoUpdate)
{
}

const DoomseekerPolicy &DoomseekerPolicy::policy()
{
	if (installedPolicy == nullptr)
		qFatal("Doomseeker policy accessed without an installed ScopedDoomseekerPolicy");
	return *installedPolicy;
}

ScopedDoomseekerPolicy::ScopedDoomseekerPolicy(DoomseekerPolicy policy)
	: value(policy), previous(installedPolicy)
{
	installedPolicy = &value;
}

ScopedDoomseekerPolicy::~ScopedDoomseekerPolicy()
{
	installedPolicy = previous;
}

bool DoomseekerPolicy::isAutoUpdateAllowed() const
{
	return autoUpdateAllowed;
}

bool DoomseekerPolicy::isIP2CUpdateAllowed() const
{
	return ip2cUpdateAllowed;
}

bool DoomseekerPolicy::isIP2CAutoUpdateAllowed() const
{
	return ip2cAutoUpdateAllowed;
}

DoomseekerPolicy::Builder::Builder()
	: autoUpdateAllowed(false), ip2cUpdateAllowed(false), ip2cAutoUpdateAllowed(false)
{
}

DoomseekerPolicy::Builder &DoomseekerPolicy::Builder::setAutoUpdateAllowed(bool allowed)
{
	autoUpdateAllowed = allowed;
	return *this;
}

DoomseekerPolicy::Builder &DoomseekerPolicy::Builder::setIP2CUpdateAllowed(bool allowed)
{
	ip2cUpdateAllowed = allowed;
	return *this;
}

DoomseekerPolicy::Builder &DoomseekerPolicy::Builder::setIP2CAutoUpdateAllowed(bool allowed)
{
	ip2cAutoUpdateAllowed = allowed;
	return *this;
}

DoomseekerPolicy DoomseekerPolicy::Builder::build() const
{
	return DoomseekerPolicy(autoUpdateAllowed, ip2cUpdateAllowed,
		ip2cUpdateAllowed && ip2cAutoUpdateAllowed);
}
