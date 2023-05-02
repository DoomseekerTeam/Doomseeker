//------------------------------------------------------------------------------
// filefind.h
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
// Copyright (C) 2017 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idb6512beb_74a5_44ab_b0d3_5a3ce1c0f476
#define idb6512beb_74a5_44ab_b0d3_5a3ce1c0f476

#include "dptr.h"

#include <QtContainerFwd>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// In Qt6 this becomes an alias in QtContainerFwd
class QStringList;
#endif

/**
 * @brief Case-insensitive filename seeker that seeks in a file tree listing.
 *
 * Finds indices under which files or paths are positioned in the listing.
 */
class FileFind
{
public:
	FileFind(const QStringList &fileTreeListing);

	QList<int> findExactPath(const QString &path);
	/**
	 * @brief Finds filename in the directory tree.
	 *
	 * Root directory of the tree is prioritized. If filename is found
	 * in the root, its index will be returned at the beginning of
	 * the index list. Files in subdirectories will follow.
	 *
	 * Returned list is empty if file is not found.
	 */
	QList<int> findFilenameAll(const QString &filename);
	/**
	 * @brief First element of findFilenameAll() or negative value if not found.
	 */
	int findFilename(const QString &filename);

private:
	DPtr<FileFind> d;
};

#endif
