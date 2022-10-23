//------------------------------------------------------------------------------
// filesearchpath.cpp
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
#include "filesearchpath.h"

#include <QStringList>

FileSearchPath::FileSearchPath()
{
	recursive_ = false;
}

FileSearchPath::FileSearchPath(const QString &path)
{
	path_ = path;
	recursive_ = false;
}

QList<FileSearchPath> FileSearchPath::fromStringList(const QStringList &collection)
{
	QList<FileSearchPath> result;
	for (QString path : collection)
	{
		result << path;
	}
	return result;
}

FileSearchPath FileSearchPath::fromVariant(const QVariant &var)
{
	FileSearchPath result;
	QVariantList list = var.toList();
	if (list.size() >= 1 && list.size() <= 2)
	{
		result.setPath(list[0].toString());
		if (list.size() == 2)
		{
			result.setRecursive(list[1].toBool());
		}
	}
	return result;
}

QList<FileSearchPath> FileSearchPath::fromVariantList(const QVariantList &collection)
{
	QList<FileSearchPath> result;
	for (const QVariant &variant : collection)
	{
		result << fromVariant(variant);
	}
	return result;
}

bool FileSearchPath::isValid() const
{
	return path_.isNull();
}

QVariant FileSearchPath::toVariant() const
{
	QVariantList var;
	var << path_;
	var << recursive_;
	return var;
}

QVariantList FileSearchPath::toVariantList(const QList<FileSearchPath> &collection)
{
	QVariantList result;
	for (const FileSearchPath &path : collection)
	{
		result << path.toVariant();
	}
	return result;
}

void FileSearchPath::setCache(const QMap<QString, QString> &files)
{
	cacheFiles_ = files;
}

bool FileSearchPath::hasCache()
{
	return !cacheFiles_.isEmpty();
}

const QMap<QString, QString> &FileSearchPath::getCache()
{
	return cacheFiles_;
}