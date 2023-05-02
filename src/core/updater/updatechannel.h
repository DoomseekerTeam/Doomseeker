//------------------------------------------------------------------------------
// updatechannel.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_UPDATER_UPDATECHANNEL_H
#define DOOMSEEKER_UPDATER_UPDATECHANNEL_H

#include <QObject>
#include <QString>

/**
 * This class is for translation purposes so the
 * strings won't appear as assigned to QObject in the
 * Qt Linguist.
 *
 * UpdateChannel is designed to be copied and copying
 * a QObject leads to a disaster.
 */
class UpdateChannelTr : public QObject
{
	Q_OBJECT

private:
	UpdateChannelTr() {}
};

class UpdateChannel
{
public:
	/**
	 * @brief List of all available channels.
	 */
	static QList<UpdateChannel> allChannels();
	/**
	 * @brief Creates object from its internal name.
	 *
	 * @param name
	 *     Name which must be equal to value returned by the name()
	 *     method. This name must be the same as one of the names
	 *     that are hard-coded in objects returned by allChannels().
	 * @return
	 *     UpdateChannel object which has the exact same name as specified.
	 *     If name isn't equal to any known ones then a null
	 *     object is returned.
	 */
	static UpdateChannel fromName(const QString &name);
	/**
	 * @brief Creates "beta" channel object.
	 */
	static UpdateChannel mkBeta();
	/**
	 * @brief Creates "stable" channel object.
	 */
	static UpdateChannel mkStable();

	/**
	 * @brief Creates a null object.
	 */
	UpdateChannel() {}
	UpdateChannel(const UpdateChannel &other);

	bool operator==(const UpdateChannel &other) const;
	bool operator!=(const UpdateChannel &other) const
	{
		return !(*this == other);
	}
	UpdateChannel &operator=(const UpdateChannel &other);

	bool isNull() const;

	QString name() const;
	QString translatedDescription() const;
	QString translatedName() const;
	/**
	 * @brief Full name of "update-info*.js" file for given channel
	 * and platform.
	 */
	QString versionDataFileName() const;

private:
	UpdateChannel(const QString &name);

	QString channelName;
};

#endif
