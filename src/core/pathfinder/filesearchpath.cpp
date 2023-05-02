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

#include "strings.hpp"
#include "templatedpathresolver.h"

#include <QDir>
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
	return !path_.isNull();
}

FileSearchPath FileSearchPath::resolveTemplated(TemplatedPathResolver &resolver,
	const FileSearchPath &path)
{
	FileSearchPath resolved = path;
	resolved.path_ = resolver.resolve(resolved.path_);
	return resolved;
}

QList<FileSearchPath> FileSearchPath::resolveTemplated(TemplatedPathResolver &resolver,
	const QList<FileSearchPath> &paths)
{
	QList<FileSearchPath> resolved;
	for (const FileSearchPath &path : paths)
		resolved << resolveTemplated(resolver, path);
	return resolved;
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

bool FileSearchPath::contains(const QString &other_) const
{
	QString self = QDir::cleanPath(this->path_);
	QString other = QDir::cleanPath(other_);
	self = Strings::trimr(self, "/");
	other = Strings::trimr(other, "/");

	#ifdef Q_OS_WIN
	self = self.toLower();
	other = other.toLower();
	#endif

	return self == other || (this->recursive_ && other.startsWith(self + '/'));
}

void FileSearchPath::merge(QList<FileSearchPath> &paths)
{
	for (int idx1 = 0; idx1 < paths.length(); ++idx1)
	{
		for (int idx2 = idx1 + 1; idx2 < paths.length(); ++idx2)
		{
			FileSearchPath &path1 = paths[idx1];
			FileSearchPath &path2 = paths[idx2];

			if (path2.contains(path1.path()))
			{
				std::swap(path1, path2);
			}

			if (path1.contains(path2.path()))
			{
				path1.setRecursive(path1.isRecursive() || path2.isRecursive());
				paths.removeAt(idx2);
				--idx2;
			}
		}
	}
}
