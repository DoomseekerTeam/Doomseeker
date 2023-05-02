//------------------------------------------------------------------------------
// filealias.cpp
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
#include "filealias.h"

#include <cassert>

FileAlias::FileAlias()
{
	d.matchType = FileAlias::LeftToRight;
}

FileAlias::FileAlias(const QString &name)
{
	d.matchType = FileAlias::LeftToRight;
	d.name = name;
}

void FileAlias::addAlias(const QString &val)
{
	if (!d.aliases.contains(val, Qt::CaseInsensitive))
	{
		d.aliases << val;
	}
}

void FileAlias::addAliases(const QStringList &val)
{
	for (const QString &element : val)
	{
		addAlias(element);
	}
}

const QStringList &FileAlias::aliases() const
{
	return d.aliases;
}

void FileAlias::setAliases(const QStringList &val)
{
	d.aliases = val;
}

FileAlias FileAlias::deserializeQVariant(const QVariant &var)
{
	QVariantMap m = var.toMap();
	FileAlias result;
	result.setAliases(m["aliases"].toStringList());
	result.setMatchType(deserializeMatchType(m["matchType"]));
	result.setName(m["name"].toString());
	return result;
}

QVariant FileAlias::serializeQVariant() const
{
	QVariantMap m;
	m["aliases"] = aliases();
	m["matchType"] = serializeMatchType(matchType());
	m["name"] = name();
	return m;
}

FileAlias::MatchType FileAlias::deserializeMatchType(const QVariant &variant)
{
	QString val = variant.toString();
	if (val == "LeftToRight")
		return LeftToRight;
	else if (val == "AllEqual")
		return AllEqual;
	// Default return value.
	return LeftToRight;
}

QVariant FileAlias::serializeMatchType(MatchType matchType)
{
	switch (matchType)
	{
	default:
		assert(false && "don't know how to serialize this FileAlias::MatchType"); // intentional fall-through
	case LeftToRight:
		return "LeftToRight";
	case AllEqual:
		return "AllEqual";
	}
}

FileAlias FileAlias::freeDoom1Aliases()
{
	FileAlias result;
	result.setName("doom.wad");
	QStringList aliases;
	aliases << "freedomu.wad" << "freedoom1.wad";
	result.setAliases(aliases);
	result.setMatchType(LeftToRight);
	return result;
}

QList<FileAlias> FileAlias::freeDoom2Aliases()
{
	QStringList bases;
	// Each of these are equivalent in terms of modern source-ports.
	// If custom levels require one of those WADs as a base, I'd assume
	// they will run just fine with any of them, and thus also with freedoom.
	bases << "doom2.wad" << "tnt.wad" << "plutonia.wad";
	QList<FileAlias> result;
	for (const QString &base : bases)
	{
		FileAlias alias;
		alias.setName(base);
		alias.setMatchType(LeftToRight);
		QStringList aliases;
		aliases << "freedoom.wad" << "freedoom2.wad";
		alias.setAliases(aliases);
		result << alias;
	}
	return result;
}

bool FileAlias::isSameName(const QString &otherName) const
{
	return d.name.compare(otherName, Qt::CaseInsensitive) == 0;
}

bool FileAlias::isValid() const
{
	return !name().isEmpty() && !aliases().isEmpty();
}

FileAlias::MatchType FileAlias::matchType() const
{
	return d.matchType;
}

void FileAlias::setMatchType(MatchType matchType)
{
	d.matchType = matchType;
}

const QString &FileAlias::name() const
{
	return d.name;
}

void FileAlias::setName(const QString &val)
{
	d.name = val;
}

QList<FileAlias> FileAlias::standardWadAliases()
{
	QList<FileAlias> result;
	result << freeDoom1Aliases();
	for (const FileAlias &alias : freeDoom2Aliases())
	{
		result << alias;
	}
	return result;
}
///////////////////////////////////////////////////////////////////////////////
QStringList FileAliasList::aliases(const QList<FileAlias> &candidates, const QString &name)
{
	QStringList allAliases;
	for (const FileAlias &candidate : candidates)
	{
		switch (candidate.matchType())
		{
		case FileAlias::LeftToRight:
			if (candidate.name().compare(name, Qt::CaseInsensitive) == 0)
				allAliases << candidate.aliases();
			break;
		case FileAlias::AllEqual:
		{
			QStringList allValidNames;
			allValidNames << candidate.name();
			allValidNames << candidate.aliases();
			if (allValidNames.contains(name, Qt::CaseInsensitive))
				allAliases << allValidNames;
			break;
		}
		default:
			assert(false && "unknown FileAlias::MatchType");
			break;
		}
	}
	// Normalize to remove duplicates and the source name.
	allAliases.removeDuplicates();
	allAliases.removeAll(name);
	return allAliases;
}

QList<FileAlias> FileAliasList::mergeDuplicates(const QList<FileAlias> &input)
{
	QList<FileAlias> result;
	for (const FileAlias &alias : input)
	{
		bool merged = false;
		for (auto &aliasOnList : result)
		{
			if (aliasOnList.isSameName(alias.name()) && aliasOnList.matchType() == alias.matchType())
			{
				aliasOnList.addAliases(alias.aliases());
				merged = true;
				break;
			}
		}
		if (!merged)
		{
			result << alias;
		}
	}
	return result;
}
