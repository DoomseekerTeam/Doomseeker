//------------------------------------------------------------------------------
// fileseekinfo.cpp
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "fileseekinfo.h"

FileSeekInfo::FileSeekInfo()
{
}

FileSeekInfo::FileSeekInfo(const QString &file, const QStringList &possibleFilenames)
	: m_file(file)
{
	for (QString filename : possibleFilenames)
	{
		filename = filename.trimmed();
		if (!filename.isEmpty() && !isFilenameListed(filename))
		{
			m_possibleFilenames << filename;
		}
	}
}

bool FileSeekInfo::isFilenameListed(const QString &filename) const
{
	for (const QString &listFilename : m_possibleFilenames)
	{
		if (filename.compare(listFilename, Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}

	return false;
}

bool FileSeekInfo::operator==(const QString &file) const
{
	return this->file().compare(file, Qt::CaseInsensitive) == 0;
}

bool FileSeekInfo::operator==(const FileSeekInfo &other) const
{
	return *this == other.file();
}

bool FileSeekInfo::operator!=(const QString &file) const
{
	return !(*this == file);
}
