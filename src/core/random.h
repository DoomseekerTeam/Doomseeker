//------------------------------------------------------------------------------
// random.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "global.h"

#include <QSharedPointer>
#include <random>

/**
 * @ingroup group_pluginapi
 * @brief Random values generator.
 *
 * Each call to a method that generates a value checks for a properly seeded
 * generator. If this check fails a builtInInit() is called. This ensures that a
 * Random number generator is always initialized. Internally this uses the
 * random C++ Standard Library, using the Mersenne Twister algorithm.
 */
class MAIN_EXPORT Random
{
public:
	/**
	 * @brief Inits or re-inits the Random class with specified seed.
	 */
	static void init(int seed);

	/**
	 * @brief Generates a new random unsigned short.
	 *
	 * @param max
	 *     Maximum value for the generated number. This is exclusive,
	 *     ie. generated value is in range 0 <= x < max
	 */
	static unsigned short nextUShort(unsigned short max);

private:
	static QSharedPointer<std::mt19937> generator;

	static void builtInInit();
};

#endif
