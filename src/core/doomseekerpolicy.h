//------------------------------------------------------------------------------
// doomseekerpolicy.h
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
#ifndef DOOMSEEKER_DOOMSEEKERPOLICY_H
#define DOOMSEEKER_DOOMSEEKERPOLICY_H

#define gPolicy DoomseekerPolicy::policy()

/**
 * Doomseeker permissions policy.
 */
class DoomseekerPolicy
{
public:
	class Builder;

	/// Requires a live ScopedDoomseekerPolicy; fails fatally if none is installed.
	static const DoomseekerPolicy &policy();
	/// Resolve from build options.
	static DoomseekerPolicy fromBuildOptions();

	/// Application update checks, downloads, and pending installations.
	bool isAutoUpdateAllowed() const;
	/// IP2C update checks and downloads, including manual updates.
	bool isIP2CUpdateAllowed() const;
	/// Automatic IP2C updates.
	bool isIP2CAutoUpdateAllowed() const;

private:
	DoomseekerPolicy(bool autoUpdate, bool ip2cUpdate, bool ip2cAutoUpdate);

	bool autoUpdateAllowed;
	bool ip2cUpdateAllowed;
	bool ip2cAutoUpdateAllowed;
};

/** All permissions default to false, independently of build options. */
class DoomseekerPolicy::Builder
{
public:
	Builder();
	Builder &setAutoUpdateAllowed(bool allowed);
	Builder &setIP2CUpdateAllowed(bool allowed);
	Builder &setIP2CAutoUpdateAllowed(bool allowed);
	/// Automatic IP2C permission requires general IP2C permission, in any setter order.
	DoomseekerPolicy build() const;

private:
	bool autoUpdateAllowed;
	bool ip2cUpdateAllowed;
	bool ip2cAutoUpdateAllowed;
};

/**
 * Own and install a policy until scope exit, restoring the previous binding.
 */
class ScopedDoomseekerPolicy
{
public:
	explicit ScopedDoomseekerPolicy(DoomseekerPolicy policy);
	~ScopedDoomseekerPolicy();

	ScopedDoomseekerPolicy(const ScopedDoomseekerPolicy &) = delete;
	ScopedDoomseekerPolicy &operator=(const ScopedDoomseekerPolicy &) = delete;
	ScopedDoomseekerPolicy(ScopedDoomseekerPolicy &&) = delete;
	ScopedDoomseekerPolicy &operator=(ScopedDoomseekerPolicy &&) = delete;

private:
	DoomseekerPolicy value;
	const DoomseekerPolicy *previous;
};

#endif
