//------------------------------------------------------------------------------
// freedoominfoparser.cpp
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
#include "freedoominfoparser.h"

#include "entities/checksum.h"
#include "entities/modset.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QVariant>

DClass<FreedoomInfoParser>
{
public:
	QByteArray contents;
	QVariantMap root;
	ModSet modSet;

	void readRoot()
	{
		QString str = QString::fromUtf8(contents.data(), contents.size());
		root = QJsonDocument::fromJson(str.toUtf8()).toVariant().toMap();
	}

	void parseAndAppendModFile(const QString &filename, const QVariantMap &var)
	{
		ModFile file = parseModFile(filename, var);
		modSet.addModFile(file);
	}

	ModFile parseModFile(const QString &filename, const QVariantMap &var)
	{
		ModFile file;

		QList<Checksum> checksums;
		checksums << Checksum(QByteArray::fromHex(var["md5"].toByteArray()), QCryptographicHash::Md5);

		file.setFileName(filename);
		file.setName(var["name"].toString());
		file.setDescription(var["description"].toString());
		file.setChecksums(checksums);
		file.setUrl(var["url"].toString());
		file.setVersion(var["version"].toString());
		return file;
	}
};
DPointered(FreedoomInfoParser)

FreedoomInfoParser::FreedoomInfoParser(const QByteArray &contents)
{
	d->contents = contents;
}

FreedoomInfoParser::~FreedoomInfoParser()
{
}

ModSet FreedoomInfoParser::parse()
{
	d->modSet.clear();
	d->readRoot();
	for (const QString &filename : d->root.keys())
	{
		d->parseAndAppendModFile(filename, d->root[filename].toMap());
	}
	return d->modSet;
}
