//------------------------------------------------------------------------------
// modset.h
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
#ifndef id78d5de3c_ac88_4972_86bf_667567e4e647
#define id78d5de3c_ac88_4972_86bf_667567e4e647

#include "../wadseekerexportinfo.h"
#include "../dptr.h"
#include "modfile.h"

/**
 * @brief A collection for ModFile objects.
 */
class WADSEEKER_API ModSet
{
public:
	ModSet();
	~ModSet();

	/**
	 * @brief Add a ModFile to the collection.
	 */
	void addModFile(const ModFile &file);
	/**
	 * @brief Clear the collection.
	 */
	void clear();
	/**
	 * @brief Get the ModFile basing on its filename.
	 */
	ModFile findFileName(const QString &fileName) const;
	/**
	 * @brief Get the first ModFile in the collection.
	 */
	ModFile first() const;
	/**
	 * @brief True if the collection is empty.
	 */
	bool isEmpty() const;
	/**
	 * @brief Converts the collection to a QList.
	 *
	 * The order of the ModFile objects is unspecified.
	 */
	QList<ModFile> modFiles() const;
	/**
	 * @brief Remove the ModFile from the collection basing on
	 * ModFile::fileName().
	 */
	void removeModFile(const ModFile &file);

private:
	DPtr<ModSet> d;
};

#endif
