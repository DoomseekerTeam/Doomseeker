//------------------------------------------------------------------------------
// caseinsensitivefsfileseeker.h
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
#ifndef id5DAEAB3C_C201_4B17_958E43B77B42E2FA
#define id5DAEAB3C_C201_4B17_958E43B77B42E2FA

#include "pathfinder/fileseeker.h"

/**
 * @brief For case-insensitive file systems (like NTFS).
 */
class CaseInsensitiveFSFileSeeker : public FileSeeker
{
public:
	CaseInsensitiveFSFileSeeker(QSharedPointer <QList<FileSearchPath> > paths);
	QString findFileInPath(const QString &fileName, FileSearchPath &path) override;
};

#endif