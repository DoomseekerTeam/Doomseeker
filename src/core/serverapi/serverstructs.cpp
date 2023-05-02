//------------------------------------------------------------------------------
// serverstructs.cpp
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
#include "serverstructs.h"
#include "wadseeker/entities/checksum.h"
#include "wadseeker/entities/hash.h"
#include "wadseeker/entities/modfile.h"
#include <QRegExp>
#include <QVector>


static QString coerceInternalName(const QString &name)
{
	return name.toLower().remove(QRegExp("[^a-z0-9]"));
}


DClass<DMFlag>
{
public:
	QString name;
	unsigned value;
	QString internalName;

	void setInternalName(const QString &name)
	{
		internalName = coerceInternalName(name);
	}
};

DPointered(DMFlag)

DMFlag::DMFlag()
{
	d->value = 0;
}

DMFlag::DMFlag(const QString &internalName, unsigned value)
{
	d->name = internalName;
	d->setInternalName(internalName);
	d->value = value;
}

DMFlag::DMFlag(const QString &internalName, unsigned value, const QString &name)
{
	d->name = name;
	d->setInternalName(internalName);
	d->value = value;
}

DMFlag::DMFlag(const DMFlag &other)
{
	this->d = other.d;
}

DMFlag::~DMFlag()
{
}

DMFlag &DMFlag::operator=(const DMFlag &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

const QString &DMFlag::internalName() const
{
	return d->internalName;
}

bool DMFlag::isValid() const
{
	return value() > 0;
}

const QString &DMFlag::name() const
{
	return d->name;
}

unsigned DMFlag::value() const
{
	return d->value;
}

////////////////////////////////////////////////////////////////////////////////

DClass<DMFlagsSection>
{
public:
	QString name;
	QString internalName;
	QVector<DMFlag> flags;

	void setInternalName(const QString &name)
	{
		internalName = coerceInternalName(name);
	}
};

DPointered(DMFlagsSection)

DMFlagsSection::DMFlagsSection()
{
}

DMFlagsSection::DMFlagsSection(const QString &internalName)
{
	d->name = internalName;
	d->setInternalName(internalName);
}

DMFlagsSection::DMFlagsSection(const QString &internalName, const QString &name)
{
	d->name = name;
	d->setInternalName(internalName);
}

DMFlagsSection::DMFlagsSection(const DMFlagsSection &other)
{
	this->d = other.d;
}

DMFlagsSection::~DMFlagsSection()
{
}

DMFlagsSection &DMFlagsSection::operator=(const DMFlagsSection &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

void DMFlagsSection::add(const DMFlag &flag)
{
	d->flags << flag;
}

unsigned DMFlagsSection::combineValues() const
{
	unsigned result = 0;
	for (const DMFlag &flag : d->flags)
	{
		result |= flag.value();
	}
	return result;
}

DMFlagsSection DMFlagsSection::copyEmpty() const
{
	DMFlagsSection copy = *this;
	copy.d->flags.clear();
	return copy;
}

int DMFlagsSection::count() const
{
	return d->flags.count();
}

const QString &DMFlagsSection::internalName() const
{
	return d->internalName;
}

bool DMFlagsSection::isEmpty() const
{
	return count() == 0;
}

const QString &DMFlagsSection::name() const
{
	return d->name;
}

const DMFlag &DMFlagsSection::operator[](int index) const
{
	return d->flags[index];
}

DMFlag &DMFlagsSection::operator[](int index)
{
	return d->flags[index];
}

QList<DMFlagsSection> DMFlagsSection::removedBySection(
	const QList<DMFlagsSection> &original,
	const QList<DMFlagsSection> &removals)
{
	QList<DMFlagsSection> copy;
	for (const DMFlagsSection &section : original)
	{
		bool removed = false;
		for (const DMFlagsSection &removal : removals)
		{
			if (section.internalName() == removal.internalName())
			{
				copy << section.removed(removal);
				removed = true;
				break;
			}
		}
		if (!removed)
		{
			copy << section;
		}
	}
	return copy;
}

DMFlagsSection DMFlagsSection::removed(const DMFlagsSection &removals) const
{
	DMFlagsSection copy = *this;
	for (const DMFlag &removal : removals.d->flags)
	{
		QMutableVectorIterator<DMFlag> i(copy.d->flags);
		while (i.hasNext())
		{
			DMFlag &flag = i.next();
			if (flag.value() == removal.value())
			{
				i.remove();
				break;
			}
		}
	}
	return copy;
}

////////////////////////////////////////////////////////////////////////////////

DClass<GameCVar>
{
public:
	QString command;
	QString name;
	QVariant value;
};

DPointered(GameCVar)

GameCVar::GameCVar()
{
}

GameCVar::GameCVar(const QString &name, const QString &command)
{
	d->name = name;
	d->command = command;
}

GameCVar::GameCVar(const QString &name, const QString &command, const QVariant &value)
{
	d->name = name;
	d->command = command;
	setValue(value);
}

GameCVar::GameCVar(const GameCVar &other)
{
	this->d = other.d;
}

GameCVar::~GameCVar()
{
}

GameCVar &GameCVar::operator=(const GameCVar &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

const QString &GameCVar::command() const
{
	return d->command;
}

bool GameCVar::hasValue() const
{
	return value().isValid();
}

bool GameCVar::isValid() const
{
	return !command().isEmpty();
}

const QString &GameCVar::name() const
{
	return d->name;
}

void GameCVar::setValue(const QVariant &value)
{
	d->value = value;
}

const QVariant &GameCVar::value() const
{
	return d->value;
}

////////////////////////////////////////////////////////////////////////////////

DClass<GameCVarProvider>
{
};

DPointeredNoCopy(GameCVarProvider)

GameCVarProvider::GameCVarProvider()
{
}

GameCVarProvider::~GameCVarProvider()
{
}

QList<GameCVar> GameCVarProvider::get(const QVariant &context)
{
	Q_UNUSED(context);
	return QList<GameCVar>();
}

////////////////////////////////////////////////////////////////////////////////

DClass<GameMode>
{
public:
	gamemode_id index;
	QString name;
	bool teamgame;
};

DPointered(GameMode)

GameMode::GameMode()
{
	d->index = SGM_Unknown;
	d->teamgame = false;
}

GameMode::GameMode(gamemode_id index, const QString &name)
{
	d->index = index;
	d->name = name;
	d->teamgame = false;
}

GameMode::GameMode(const GameMode &other)
{
	this->d = other.d;
}

GameMode::~GameMode()
{
}

GameMode &GameMode::operator=(const GameMode &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

GameMode GameMode::ffaGame(int index, const QString &name)
{
	GameMode result(index, name);
	result.setTeamGame(false);
	return result;
}

gamemode_id GameMode::index() const
{
	return d->index;
}

GameMode GameMode::mkCooperative()
{
	return ffaGame(SGM_Cooperative, QObject::tr("Cooperative"));
}

GameMode GameMode::mkDeathmatch()
{
	return ffaGame(SGM_Deathmatch, QObject::tr("Deathmatch"));
}

GameMode GameMode::mkTeamDeathmatch()
{
	return teamGame(SGM_TeamDeathmatch, QObject::tr("Team DM"));
}

GameMode GameMode::mkCaptureTheFlag()
{
	return teamGame(SGM_CTF, QObject::tr("CTF"));
}

GameMode GameMode::mkUnknown()
{
	return ffaGame(SGM_Unknown, QObject::tr("Unknown"));
}

const QString &GameMode::name() const
{
	return d->name;
}

bool GameMode::isTeamGame() const
{
	return d->teamgame;
}

bool GameMode::isValid() const
{
	return !d->name.isEmpty();
}

void GameMode::setTeamGame(bool b)
{
	d->teamgame = b;
}

GameMode GameMode::teamGame(int index, const QString &name)
{
	GameMode result(index, name);
	result.setTeamGame(true);
	return result;
}

////////////////////////////////////////////////////////////////////////////////

DClass<PWad>
{
public:
	QString name;
	bool optional;
	QList<Checksum> checksums;
};

DPointered(PWad)

PWad::PWad(const QString &name, bool optional, const QList<Checksum> &checksums)
{
	d->name = name;
	d->optional = optional;
	d->checksums = checksums;
}

PWad::PWad(const QString &name, bool optional)
{
	d->name = name;
	d->optional = optional;
}

PWad::PWad(const ModFile &modFile)
{
	d->name = modFile.fileName();
	d->optional = false;
	d->checksums = modFile.checksums();
}

PWad::PWad(const PWad &other)
{
	this->d = other.d;
}

PWad::~PWad()
{
}

PWad &PWad::operator=(const PWad &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

PWad::operator ModFile()
{
	ModFile modFile(d->name);
	modFile.setChecksums(d->checksums);
	return modFile;
}

bool PWad::isOptional() const
{
	return d->optional;
}

const QString &PWad::name() const
{
	return d->name;
}

const QList<Checksum> PWad::checksums() const
{
	return d->checksums;
}

void PWad::addChecksum(const QByteArray &hash, const QCryptographicHash::Algorithm &algorithm)
{
	d->checksums.append(Checksum(hash, algorithm));
}

bool PWad::validFile(const QString &path) const
{
	for (const Checksum checksum : d->checksums)
	{
		if (Hash::hashFile(path, checksum.algorithm()) != checksum.hash())
		{
			return false;
		}
	}
	return true;
}
