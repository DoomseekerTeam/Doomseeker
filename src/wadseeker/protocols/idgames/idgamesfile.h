//------------------------------------------------------------------------------
// idgamesfile.h
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
#ifndef id6153B628_4703_4117_8E9690A6527F8835
#define id6153B628_4703_4117_8E9690A6527F8835

#include <QString>
#include <QVariant>

/**
 * @brief File entry in idgames database.
 */
class IdgamesFile
{
public:
	static QList<IdgamesFile> parseSearchResult(const QVariant &rawData);

	IdgamesFile();
	IdgamesFile(const QVariant &rawData);

	QString filename() const;
	bool isNull() const;
	QString url() const;

private:
	QVariantMap data;
};

#endif
