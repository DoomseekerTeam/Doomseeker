//------------------------------------------------------------------------------
// filealias.h
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
#ifndef id8E04D971_DF78_4DB4_97E84A8D859F3694
#define id8E04D971_DF78_4DB4_97E84A8D859F3694

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

class FileAlias
{
public:
	enum MatchType
	{
		LeftToRight,
		AllEqual
	};

	static FileAlias freeDoom1Aliases();
	static QList<FileAlias> freeDoom2Aliases();
	/**
	 * @brief Standard/default aliases for configuration init.
	 */
	static QList<FileAlias> standardWadAliases();

	FileAlias();
	FileAlias(const QString &name);

	/**
	 * @brief Will ensure unique values.
	 */
	void addAlias(const QString &val);
	/**
	 * @brief Will ensure unique values.
	 */
	void addAliases(const QStringList &val);

	const QStringList &aliases() const;
	void setAliases(const QStringList &val);

	static FileAlias deserializeQVariant(const QVariant &var);
	QVariant serializeQVariant() const;

	/**
	 * @brief Compares passed string to local name and checks if they're
	 *        same.
	 */
	bool isSameName(const QString &otherName) const;

	/**
	 * @brief Valid FileAlias has a name and at least one alias.
	 */
	bool isValid() const;

	MatchType matchType() const;
	void setMatchType(MatchType matchType);

	const QString &name() const;
	void setName(const QString &val);

private:
	class PrivData
	{
	public:
		QStringList aliases;
		MatchType matchType;
		QString name;
	};

	PrivData d;

	static MatchType deserializeMatchType(const QVariant &variant);
	static QVariant serializeMatchType(MatchType matchType);
};

class FileAliasList
{
public:
	/**
	 * Match `name` to all `candidates` in accordance to their FileAlias::matchType();
	 * return list of all other names for this file.
	 */
	static QStringList aliases(const QList<FileAlias> &candidates, const QString &name);
	static QList<FileAlias> mergeDuplicates(const QList<FileAlias> &input);
};

#endif // header
