//------------------------------------------------------------------------------
// gamefile.cpp
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
#include "gamefile.h"

#include "fileutils.h"

DClass<GameFile>
{
public:
	QString configName;
	int executable;
	QString fileName;
	QString niceName;
	QStringList searchSuffixes;
};

DPointered(GameFile)

GameFile::GameFile()
{
	d->executable = 0;
}

GameFile::GameFile(const GameFile &other)
{
	this->d = other.d;
}

GameFile::~GameFile()
{
}

GameFile &GameFile::operator=(const GameFile &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

const QString &GameFile::configName() const
{
	return d->configName;
}

GameFile &GameFile::setConfigName(const QString &name)
{
	d->configName = name;
	return *this;
}

const QString &GameFile::fileName() const
{
	return d->fileName;
}

GameFile &GameFile::setFileName(const QString &name)
{
	d->fileName = name;
	return *this;
}

int GameFile::executable() const
{
	return d->executable;
}

GameFile &GameFile::setExecutable(int flags)
{
	d->executable = flags;
	return *this;
}

bool GameFile::isSameFile(const QString &otherFileName)
{
	QString thisFile = fileName();
	#if defined(Q_OS_WIN32)
	if (executable() != 0)
	{
		if (!thisFile.endsWith(".exe"))
		{
			thisFile += ".exe";
		}
	}
	#endif
	return thisFile.compare(otherFileName, FileUtils::comparisonSensitivity()) == 0;
}

bool GameFile::isValid() const
{
	return !configName().isEmpty();
}

const QString &GameFile::niceName() const
{
	return d->niceName;
}

GameFile &GameFile::setNiceName(const QString &name)
{
	d->niceName = name;
	return *this;
}

QStringList &GameFile::searchSuffixes() const
{
	return d->searchSuffixes;
}

GameFile &GameFile::setSearchSuffixes(const QStringList &suffixes)
{
	d->searchSuffixes = suffixes;
	return *this;
}

///////////////////////////////////////////////////////////////////////////

DClass<GameFileList>
{
public:
	QList<GameFile> list;
};

DPointered(GameFileList)


GameFileList::GameFileList()
{
}

GameFileList::~GameFileList()
{
}

GameFileList &GameFileList::append(const GameFile &gameFile)
{
	d->list << gameFile;
	return *this;
}

GameFileList &GameFileList::append(const GameFileList &list)
{
	for (const GameFile &other : list.asQList())
	{
		append(other);
	}
	return *this;
}

QList<GameFile> GameFileList::asQList() const
{
	return d->list;
}

void GameFileList::clear()
{
	d->list.clear();
}

GameFile GameFileList::first() const
{
	for (const GameFile &file : d->list)
	{
		if (file.isValid())
		{
			return file;
		}
	}
	return GameFile();
}

GameFile GameFileList::findByConfigName(const QString &configName)
{
	for (const GameFile &file : d->list)
	{
		if (file.configName() == configName)
		{
			return file;
		}
	}
	return GameFile();
}

bool GameFileList::isEmpty() const
{
	return d->list.isEmpty();
}

GameFileList &GameFileList::prepend(const GameFile &gameFile)
{
	d->list.prepend(gameFile);
	return *this;
}

GameFileList &operator<<(GameFileList &list, const GameFile &gameFile)
{
	list.append(gameFile);
	return list;
}

GameFileList &operator<<(GameFileList &list, const GameFileList &other)
{
	list.append(other);
	return list;
}

///////////////////////////////////////////////////////////////////////////

GameFileList GameFiles::allCreateGameExecutables(const GameFileList &list)
{
	return allFlagMatchExecutables(list, GameFile::CreateGame);
}

GameFileList GameFiles::allClientExecutables(const GameFileList &list)
{
	return allFlagMatchExecutables(list, GameFile::Client);
}

GameFileList GameFiles::allServerExecutables(const GameFileList &list)
{
	return allFlagMatchExecutables(list, GameFile::Server);
}

GameFileList GameFiles::allFlagMatchExecutables(const GameFileList &list, int execs)
{
	GameFileList result;
	for (const GameFile &file : list.asQList())
	{
		if (file.isValid() && (execs & file.executable()))
		{
			result << file;
		}
	}
	return result;
}

GameFile GameFiles::defaultClientExecutable(const GameFileList &list)
{
	for (const GameFile &file : list.asQList())
	{
		if (file.isValid() && (file.executable() & GameFile::Client)
			&& file.configName() == "BinaryPath")
		{
			return file;
		}
	}
	return GameFile();
}

GameFile GameFiles::defaultOfflineExecutable(const GameFileList &list)
{
	for (const GameFile &file : list.asQList())
	{
		if (file.isValid() && (file.executable() & GameFile::Offline)
			&& file.configName() == "BinaryPath")
		{
			return file;
		}
	}
	return GameFile();
}

GameFile GameFiles::defaultServerExecutable(const GameFileList &list)
{
	for (const GameFile &file : list.asQList())
	{
		if (file.isValid() && (file.executable() & GameFile::Server)
			&& (file.configName() == "ServerBinaryPath" || file.configName() == "BinaryPath"))
		{
			return file;
		}
	}
	return GameFile();
}

GameFile GameFiles::preferredOfflineExecutable(const GameFileList &list)
{
	GameFile candidate = defaultOfflineExecutable(list);
	if (candidate.isValid())
	{
		return candidate;
	}
	for (const GameFile &file : list.asQList())
	{
		if (file.isValid() && (file.executable() & GameFile::Offline))
		{
			return file;
		}
	}
	return GameFile();
}
