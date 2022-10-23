//------------------------------------------------------------------------------
// gameexefactory.h
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id53E6D544_EC39_49F8_93CF518BBB413164
#define id53E6D544_EC39_49F8_93CF518BBB413164

#include "dptr.h"
#include "global.h"
#include "serverapi/polymorphism.h"
#include <QList>
#include <QObject>

class EnginePlugin;
class ExeFilePath;
class GameFileList;
class Server;


/**
 * @ingroup group_pluginapi
 * @brief Returns executable file retrievers from plugins to Doomseeker.
 *
 * Doomseeker will use this factory to retrieve file lists for operations that
 * are not directly tied into connecting to specific servers. It expects these
 * files to be configured by the user.
 *
 * When connecting to servers different executables may be required depending on
 * the Server itself (for example for different game versions). To handle this
 * case Server::clientExe() returns an ExeFile instance which allows more
 * flexibility in behavior implementation.
 *
 * @see EnginePlugin::Data
 */
class MAIN_EXPORT GameExeFactory : public QObject
{
	Q_OBJECT

public:
	GameExeFactory(EnginePlugin *plugin);
	virtual ~GameExeFactory() override;

	/**
	 * @brief @b [Virtual] Additional paths to any executable that matches
	 * the bit mask.
	 *
	 * Return any paths to any additional executables that may match the
	 * execType bitmask field. Executables should exist on the local
	 * machine. These executables will be used by Doomseeker to present
	 * users with additional choices, for example to pick a different
	 * executable when creating a new game.
	 *
	 * Default implementation does nothing and returns an empty list.
	 *
	 * @param execType
	 *     GameFile::ExecType bitmask field. Return only executables that
	 *     match this bitmask. Match criteria are to be decided by the
	 *     plugin.
	 */
	QList<ExeFilePath> additionalExecutables(int execType) const;

	/**
	 * @brief @b [Virtual] List of all game files associated with this
	 * game.
	 *
	 * This list is used to generate path configurators in plugin
	 * configuration box.
	 *
	 * Read doc for GameFile.
	 */
	GameFileList gameFiles() const;

	/**
	 * @brief Gets EnginePlugin associated with this object.
	 */
	EnginePlugin *plugin() const;

protected:
	POLYMORPHIC_SETTER_DECLARE_CONST(GameFileList, GameExeFactory, gameFiles, ())
	GameFileList gameFiles_default() const;

	POLYMORPHIC_SETTER_DECLARE_CONST(QList<ExeFilePath>, GameExeFactory, additionalExecutables, (int))
	QList<ExeFilePath> additionalExecutables_default(int execType) const;

private:
	DPtr<GameExeFactory> d;
};

#endif
