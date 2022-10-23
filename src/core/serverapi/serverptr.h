//------------------------------------------------------------------------------
// serverptr.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id85D9CDF0_8CF4_4753_A2F0F8C00FCCD0B8
#define id85D9CDF0_8CF4_4753_A2F0F8C00FCCD0B8

#include <QSharedPointer>

/**
 * @ingroup group_pluginapi
 * @file
 * ServerPtr typedefs declarations.
 *
 * This file contains forward declarations of ServerPtr types and is meant
 * to be included wherever no real access to the underlying Server
 * class is needed (which means "in other headers files", basically). Including
 * server.h automatically includes serverptr.h, but should only be done
 * where access to Server instances is actually needed. This is to reduce
 * recompilation times whenever server.h changes. This file should always
 * remain fairly static.
 */

class Server;
using ServerPtr = QSharedPointer<Server>;
using ServerCPtr = QSharedPointer<const Server>;

#endif
