//------------------------------------------------------------------------------
// ini.h
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
#ifndef __INI_H_
#define __INI_H_

#include "dptr.h"
#include "global.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include <QString>
#include <QStringList>
#include <QVector>

class SettingsProvider;

/**
 * @ingroup group_pluginapi
 * @brief Configuration handler.
 *
 * This class is an extension to simple interface provided by SettingsProvider.
 *
 * This is wrapper provides a hierarchical access
 * to the configuration. The hierarchy is resolved through the use of IniSection
 * and IniVariable classes. The Ini class provides methods necessary to obtain
 * valid instances of these objects.
 *
 * Objects of IniSection and IniVariable classes can be copied over as they
 * hold no actual data. They are used to generate valid paths and internally
 * use methods provided by the Ini class objects that are associated with them.
 * However, Ini object @b must remain valid when associated IniSection and
 * IniVariable objects are in use or segmentation fault will occur.
 *
 * Names of sections and variables are case sensitive.
 *
 * The currently supported file format is:
 * @code
 *
 * [ section1 ]
 * var1=1
 * var2="Exit game"
 *
 * [ section2 ]
 * var1=10
 * var2="New Game"
 *
 * @endcode
 *
 */
class MAIN_EXPORT Ini : public QObject
{
	Q_OBJECT

public:
	Ini(SettingsProvider *provider);
	virtual ~Ini() override;

	/**
	 * Value of data parameter will be ignored and no changes will be
	 * performed if setting already exists.
	 * @return Newly created or existing setting.
	 */
	IniVariable createSetting(const QString &sectionname, const QString &name, const QVariant &data);

	/**
	 * Completely removes a given section.
	 */
	void deleteSection(const QString &sectionname);

	/**
	 * Completely removes a given setting. This will not erase the section
	 * even if it's completely empty.
	 */
	void deleteSetting(const QString &sectionname, const QString &settingname);

	/**
	 * @brief true if setting of given name exists within given section.
	 *
	 * Setting contents may be empty and this will still return true.
	 * false is returned only if setting doesn't exist at all.
	 */
	bool hasSetting(const QString &sectionname, const QString &settingname) const;

	void removeKey(const QString &key);

	/**
	 * This won't create a variable if it doesn't exist and return nullptr
	 * in such case.
	 * @return A reference to the internally stored IniVariable if it
	 * does. Be sure to check if it isNull.
	 */
	IniVariable retrieveSetting(const QString &sectionname, const QString &variablename);

	/**
	 * @brief Access configuration file section.
	 *
	 * Creates and returns an accessor object to a section of
	 * specified name.  No actual modification to underlying data
	 * is done. The IniSection object is merely a convenience
	 * proxy for the Ini class.
	 *
	 * If section name is empty then a 'null object' of IniSection
	 * type is returned.
	 */
	IniSection section(const QString &name);

	/**
	 * Retrieves references to all sections whose names fit a certain
	 * pattern. The regex pattern will be instructed to
	 * ignore the case size.
	 */
	QVector<IniSection> sectionsArray(const QString &regexPattern);

	/**
	 * This method will create a variable if it doesn't exist. To avoid
	 * this behavior see: @see retrieveSetting().
	 * @return Returns a pointer to a IniVariable object. Do not delete
	 * this object.
	 */
	IniVariable setting(const QString &sectionname, const QString &variablename);

	void setValue(const QString &key, const QVariant &value);
	QVariant value(const QString &key) const;

private:
	DPtr<Ini> d;
};

#endif
