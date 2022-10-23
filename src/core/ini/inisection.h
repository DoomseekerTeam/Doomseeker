//------------------------------------------------------------------------------
// inisection.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __INISECTION_H__
#define __INISECTION_H__

#include "dptr.h"
#include "global.h"
#include <QVariant>
#include <QVector>

class Ini;
class IniVariable;

/**
 * @ingroup group_pluginapi
 * @brief INI section representation.
 *
 * Provides access to variables in specified INI file section.
 */
class MAIN_EXPORT IniSection
{
public:
	/**
	 * @brief Creates an invalid IniSection object. Such object should not
	 *        be used for read/write operations.
	 */
	IniSection();

	/**
	 * @brief Creates a valid IniSection object.
	 *
	 * IniSection object will operate on specified Ini object and provide
	 * access to variables in specified section.
	 * @param pIni
	 *      Pointer to Ini object on which this object will operate on.
	 *      pIni is not deleted when this is deleted. pIni pointer is copied
	 *      over when this is copied over.
	 * @param sectionName
	 *      Name of the section for which the variables will be accessed.
	 */
	IniSection(Ini *pIni, const QString &sectionName);

	virtual ~IniSection();

	/**
	 * @brief Inits specified variable with specified data.
	 *
	 * This method serves specifically for init purposes.
	 * If variable exists already exists, data will not be modified.
	 *
	 * @param name
	 *      Name of the variable to init. If '/' character is used,
	 *      a variable in subsection will be accessed.
	 * @param data
	 *      Init data. This data will be assigned to variable if specified
	 *      variable doesn't yet exist.
	 *
	 * @return IniVariable object that wraps around specified variable.
	 *         A valid IniVariable object is returned both if variable was
	 *         just created or already existed when createSetting() was
	 *         called. IniVariable object is created by a call to
	 *         retrieveSetting(). Invalid IniVariable is created if name
	 *         is empty.
	 */
	IniVariable createSetting(const QString &name, const QVariant &data);

	/**
	 * @brief Deletes specified variable.
	 *
	 * @param name
	 *      Name of the variable to init. If '/' character is used,
	 *      a variable in subsection will be accessed.
	 */
	void deleteSetting(const QString &name);

	/**
	 * @brief true if setting of given name exists within the section.
	 *
	 * Setting contents may be empty and this will still return true.
	 * false is returned only if setting doesn't exist at all.
	 */
	bool hasSetting(const QString &name) const;

	/**
	 * @brief If true, IniSection object is not valid and should not be
	 *        used to perform any actions on the Ini file.
	 */
	bool isNull() const;

	/**
	 * @brief A name (or path) of this section with lettercase preserved.
	 */
	const QString &name() const;

	/**
	 * @brief Gets a variable but only if it already exists
	 *
	 * @param name
	 *      Name of the variable to init. If '/' character is used,
	 *      a variable in subsection will be accessed.
	 * @return IniVariable object that wraps around the variable. May be
	 *         invalid if name is empty.
	 */
	IniVariable retrieveSetting(const QString &name);

	/**
	 * @brief const version of retrieveSetting()
	 */
	const IniVariable retrieveSetting(const QString &name) const;

	/**
	 * @brief Gets a variable. Creates it first if it doesn't exist yet.
	 *
	 * @param name
	 *      Name of the variable to init. If '/' character is used,
	 *      a variable in subsection will be accessed.
	 * @return IniVariable object that wraps around the variable. May be
	 *         invalid if name is empty.
	 */
	IniVariable setting(const QString &name);

	/**
	 * @brief Sets a variable directly. Omits the IniVariable system.
	 *
	 * @param key
	 *      Path to the variable name. May include subsections by using '/'
	 *      character.
	 * @param value
	 *      New value for the variable.
	 */
	void setValue(const QString &key, const QVariant &value);

	/**
	 * @brief Calls setting().
	 */
	IniVariable operator[](const QString &name);

	/**
	 * @brief Calls const retrieveSetting().
	 */
	const IniVariable operator[](const QString &name) const;

	/**
	 * @brief Retrieves a variable directly; omits the IniVariable system.
	 *
	 * @return QVariant object that denotes the variable. May be invalid
	 *         if variable is not defined.
	 */
	QVariant value(const QString &key) const;

	/**
	 * @brief Retrieves a variable directly; omits the IniVariable system.
	 *
	 * Overload that returns defaultValue if requested value is invalid.
	 *
	 * @return QVariant object that denotes the variable. If variable is
	 *         not valid, the defaultValue is returned.
	 */
	QVariant value(const QString &key, QVariant defaultValue) const;

private:
	DPtr<IniSection> d;

	/**
	 * @brief Removes specified key in this section.
	 */
	void remove(const QString &key);
};

#endif
