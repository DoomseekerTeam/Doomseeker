//------------------------------------------------------------------------------
// filesearchpath.h
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
#ifndef idD5CA37A7_5FD3_4151_8316AB0B1F9974E5
#define idD5CA37A7_5FD3_4151_8316AB0B1F9974E5

#include <QSet>
#include <QString>
#include <QVariant>

class FileSearchPath
{
public:
	static QList<FileSearchPath> fromStringList(const QStringList &collection);
	static FileSearchPath fromVariant(const QVariant &var);
	static QList<FileSearchPath> fromVariantList(const QVariantList &collection);
	static QVariantList toVariantList(const QList<FileSearchPath> &collection);

	FileSearchPath();
	FileSearchPath(const QString &path);

	bool isRecursive() const
	{
		return recursive_;
	}

	bool isValid() const;
	QVariant toVariant() const;

	const QString &path() const
	{
		return path_;
	}

	void setPath(const QString &path)
	{
		path_ = path;
	}

	void setRecursive(bool b)
	{
		recursive_ = b;
	}

	/**
	 * @brief Set the list of the names files located in the directory.
	 *
	 * The cache is a map where the key is the name in lowercase and the entry
	 * will be case sensitive or not depending on the implementation.
	 */
	void setCache(const QMap<QString, QString> &files);
	bool hasCache();
	const QMap<QString, QString> &getCache();
private:
	QString path_;
	bool recursive_;
	QMap<QString, QString> cacheFiles_;
};

#endif
