//------------------------------------------------------------------------------
// ircsounds.cpp
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
#include "ircsounds.h"

#include "irc/configuration/ircconfig.h"

#include <QFileInfo>
#include <QSoundEffect>

void IRCSounds::loadFromConfig()
{
	unload();

	if (gIRCConfig.sounds.bUseNicknameUsedSound)
	{
		QString path = gIRCConfig.sounds.nicknameUsedSound;
		sounds.insert(NicknameUsed, loadIfExists(path));
	}

	if (gIRCConfig.sounds.bUsePrivateMessageReceivedSound)
	{
		QString path = gIRCConfig.sounds.privateMessageReceivedSound;
		sounds.insert(PrivateMessageReceived, loadIfExists(path));
	}
}

QSoundEffect *IRCSounds::loadIfExists(const QString &path)
{
	QFileInfo fileInfo(path);
	if (fileInfo.isFile())
	{
		auto sfx = new QSoundEffect();
		sfx->setSource(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
		return sfx;
	}
	return nullptr;
}

void IRCSounds::playIfAvailable(SoundType sound)
{
	auto pSound = sounds[sound];
	if (pSound != nullptr)
		pSound->play();
}

void IRCSounds::unload()
{
	for (auto sfx : sounds)
		delete sfx;
	sounds.clear();
}
