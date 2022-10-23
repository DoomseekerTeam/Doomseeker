//------------------------------------------------------------------------------
// ircsounds.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __IRCSOUNDS_H__
#define __IRCSOUNDS_H__

#include <QMap>

class QSoundEffect;

/**
 * @brief Part of IRC UI package. Plays IRC related sounds.
 *
 * Loads, stores and plays IRC sounds if they are available and
 * if Doomseeker is configured to use them.
 */
class IRCSounds
{
public:
	enum SoundType
	{
		NicknameUsed,
		PrivateMessageReceived
	};

	IRCSounds() {}
	~IRCSounds()
	{
		unload();
	}

	/**
	 * @brief Reloads all sounds using paths stored in config.
	 *
	 * All previously loaded sounds are unloaded.
	 */
	void loadFromConfig();

	/**
	 * @brief Plays given sound.
	 */
	void playIfAvailable(SoundType sound);

private:
	QMap<SoundType, QSoundEffect *> sounds;

	QSoundEffect *loadIfExists(const QString &path);
	void unload();
};

#endif
