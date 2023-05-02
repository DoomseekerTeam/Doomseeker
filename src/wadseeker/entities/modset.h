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
 * @brief Stores a list of ModFile classes. Adds convenience functions to
 * operate on the whole list.
 */
class WADSEEKER_API ModSet
{
public:
	ModSet();
	~ModSet();

	/**
	 * @brief Adds a ModFile class to the list.
	 */
	void addModFile(const ModFile &file);
	/**
	 * @brief clears the list.
	 */
	void clear();
	/**
	 * @brief find file providing its filename.
	 */
	ModFile findFileName(const QString &fileName) const;
	/**
	 * @brief returns the first ModFile on the list.
	 */
	ModFile first() const;
	/**
	 * @brief returns true if list is empty.
	 */
	bool isEmpty() const;
	/**
	 * @brief returns a QList with all the modFiles stored.
	 */
	QList<ModFile> modFiles() const;
	/**
	 * @brief removes the ModFile that is provided to the function.
	 * Note that only the FileName has to mach.
	 */
	void removeModFile(const ModFile &file);

private:
	DPtr<ModSet> d;
};

#endif
