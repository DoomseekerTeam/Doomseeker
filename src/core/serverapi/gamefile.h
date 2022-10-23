//------------------------------------------------------------------------------
// gamefile.h
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idc66bde0f_a415_4f5a_8dcb_34697293515c
#define idc66bde0f_a415_4f5a_8dcb_34697293515c

#include "dptr.h"
#include "global.h"
#include <QList>
#include <QString>

/**
 * @ingroup group_pluginapi
 * @brief Game file definition allows to browse this file in configuration box.
 *
 * There are some standard values for configName() property:
 *
 * - BinaryPath is the general game executable path. It's always used as the
 *   client executable when joining servers and as offline game executable.
 *   It can also be used to host servers if EnginePlugin::Data::clientOnly
 *   is true.
 * - ServerBinaryPath is path to server executable. It's used when
 *   EnginePlugin::Data::clientOnly is false.
 *
 * Apart from these, the configName can be anything you want and anything your
 * plugin needs. Remember however that choosing a distinct name is important to
 * avoid collisions with any future standard names.
 *
 * There are three types of executables recognised by Doomseeker:
 * isClientExecutable(), isServerExecutable() and isOfflineExecutable().
 * One executable can be all of these at the same time or the game might have
 * separate executables for each mode. Setting one of these to true also implies
 * isExecutable(). If plugin doesn't specify its own list of GameFile then
 * Doomseeker will assume some defaults depending on plugin's EP_ClientOnly
 * flag. The defaults are as follows:
 *
 * - If plugin defines EP_ClientOnly flag then there's only one executable that
 *   is all: client, server and offline.
 * - If plugin doesn't define EP_ClientOnly then there are two executables:
 *   client executable that is both for client and offline modes and server
 *   executable that is only for hosting (possibly just a console).
 *
 * This class follows Builder pattern. All setters return reference to
 * the called object so that you can chain them together.
 *
 * @code
 * GameFile file;
 * return file.setNiceName(tr("client")).setConfigName("BinaryPath")
 *     .setFileName("rungame").setExecutable(true);
 * @endcode
 *
 */
class MAIN_EXPORT GameFile
{
public:
	/**
	 * @ingroup group_pluginapi
	 * @brief Executable types recognised by Doomseeker.
	 */
	enum ExecType
	{
		Client = 0x1,
		Server = 0x2,
		Offline = 0x4,
		Cso = Client | Server | Offline,
		CreateGame = Server | Offline,
	};

	GameFile();
	GameFile(const GameFile &other);
	virtual ~GameFile();
	GameFile &operator=(const GameFile &other);

	/**
	 * @brief Guesses by file name if this is the same file.
	 */
	bool isSameFile(const QString &otherFileName);

	/**
	 * @brief A valid file has configName().
	 */
	bool isValid() const;

	/**
	 * @brief Executable bit flags mod that compares to ExecType.
	 */
	int executable() const;
	GameFile &setExecutable(int flags);

	/**
	 * @brief Setting name where path will be stored in plugin's IniSection.
	 */
	const QString &configName() const;
	GameFile &setConfigName(const QString &name);

	/**
	 * @brief Name of the file on disk.
	 *
	 * Platform specific extensions (.exe) are unnecessary.
	 */
	const QString &fileName() const;
	GameFile &setFileName(const QString &name);

	/**
	 * @brief Descriptive name, ie. "client executable", "server executable",
	 * etc.
	 */
	const QString &niceName() const;
	GameFile &setNiceName(const QString &name);

	/**
	 * @brief Path suffixes that help in automatically finding this file.
	 *
	 * @see EnginePlugin::InitFeatures::EP_GameFileSearchSuffixes.
	 */
	QStringList &searchSuffixes() const;
	GameFile &setSearchSuffixes(const QStringList &suffixes);

private:
	DPtr<GameFile> d;
};


/**
 * @ingroup group_pluginapi
 * @brief GameFile collection.
 */
class MAIN_EXPORT GameFileList
{
	friend MAIN_EXPORT GameFileList &operator<<(GameFileList &list, const GameFile &gameFile);
	friend MAIN_EXPORT GameFileList &operator<<(GameFileList &list, const GameFileList &other);

public:
	GameFileList();
	virtual ~GameFileList();

	GameFileList &append(const GameFile &gameFile);
	GameFileList &append(const GameFileList &list);
	QList<GameFile> asQList() const;
	void clear();
	GameFile findByConfigName(const QString &configName);
	GameFile first() const;
	bool isEmpty() const;
	GameFileList &prepend(const GameFile &gameFile);

private:
	DPtr<GameFileList> d;
};

namespace GameFiles
{
GameFileList allCreateGameExecutables(const GameFileList &list);
GameFileList allClientExecutables(const GameFileList &list);
GameFileList allServerExecutables(const GameFileList &list);

/**
 * @param execs
 *     Binary OR of GameFile::ExecType.
 */
GameFileList allFlagMatchExecutables(const GameFileList &list, int execs);

GameFile defaultClientExecutable(const GameFileList &list);
GameFile defaultOfflineExecutable(const GameFileList &list);
GameFile defaultServerExecutable(const GameFileList &list);

GameFile preferredOfflineExecutable(const GameFileList &list);
}

#endif
