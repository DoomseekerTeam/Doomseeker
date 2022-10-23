//------------------------------------------------------------------------------
// ip2capi.h
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
#ifndef DOOMSEEKER_IP2C_IP2CAPI_H
#define DOOMSEEKER_IP2C_IP2CAPI_H

#include "global.h"

/**
 * @ingroup group_pluginapi
 * @brief The list of ISO 3166-1 alpha-3 codes plus extras known to Doomseeker.
 *
 * The "extras" currently include only the custom `EU` code which
 * indicates a place located somewhere in the European Union.
 */
MAIN_EXPORT const QStringList &countryCodes();

#endif
