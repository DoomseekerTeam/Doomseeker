//------------------------------------------------------------------------------
// idgamesfile.cpp
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
#include "idgamesfile.h"

IdgamesFile::IdgamesFile()
{
}

IdgamesFile::IdgamesFile(const QVariant &rawData)
{
	this->data = rawData.toMap();
}

QList<IdgamesFile> IdgamesFile::parseSearchResult(const QVariant &rawData)
{
	QList<IdgamesFile> collection;
	// Value of 'file' key can be either a list of objects or a single
	// object. If list is empty we asasume that there's a single object.
	QVariant varFile = rawData.toMap()["file"];
	if (!varFile.toList().isEmpty())
	{
		for (const QVariant &element : varFile.toList())
		{
			collection << IdgamesFile(element);
		}
	}
	else
	{
		collection << IdgamesFile(varFile);
	}
	return collection;
}

QString IdgamesFile::filename() const
{
	return data["filename"].toString();
}

bool IdgamesFile::isNull() const
{
	return data.isEmpty();
}

QString IdgamesFile::url() const
{
	return data["url"].toString();
}
