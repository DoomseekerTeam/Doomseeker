//------------------------------------------------------------------------------
// ini.cpp
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
#include "ini.h"

#include "ini/settingsprovider.h"
#include "ini/settingsproviderqt.h"
#include "log.h"
#include "strings.hpp"

#include <cassert>
#include <QRegularExpression>

DClass<Ini>
{
public:
	SettingsProvider *provider;
};

DPointered(Ini)

Ini::Ini(SettingsProvider *provider)
{
	d->provider = provider;
}

Ini::~Ini()
{
}

IniVariable Ini::createSetting(const QString &sectionName, const QString &name, const QVariant &data)
{
	IniSection s = section(sectionName);
	if (s.isNull())
		return IniVariable();

	return s.createSetting(name, data);
}

void Ini::deleteSection(const QString &sectionName)
{
	for (const QString &key : d->provider->allKeys())
	{
		if (key.startsWith(sectionName + "/", Qt::CaseInsensitive))
			removeKey(key);
	}
}

void Ini::deleteSetting(const QString &sectionName, const QString &settingName)
{
	removeKey(sectionName + "/" + settingName);
}

bool Ini::hasSetting(const QString &sectionname, const QString &settingname) const
{
	return d->provider->hasKey(sectionname + "/" + settingname);
}

void Ini::removeKey(const QString &key)
{
	d->provider->remove(key);
}

IniVariable Ini::retrieveSetting(const QString &sectionName, const QString &variableName)
{
	IniSection section = this->section(sectionName);
	if (section.isNull())
		return IniVariable();

	return section.retrieveSetting(variableName);
}

IniSection Ini::section(const QString &name)
{
	if (name.isEmpty())
		return IniSection();

	return IniSection(this, name);
}

QVector<IniSection> Ini::sectionsArray(const QString &regexPattern)
{
	QVector<IniSection> sectionsReferencesArray;

	QRegularExpression regExp(regexPattern, QRegularExpression::CaseInsensitiveOption);

	QStringList groups = d->provider->allSections();

	for (const QString &key : groups)
	{
		if (key.contains(regExp))
			sectionsReferencesArray << IniSection(this, key);
	}

	return sectionsReferencesArray;
}

IniVariable Ini::setting(const QString &sectionName, const QString &variableName)
{
	if (sectionName.isEmpty() || variableName.isEmpty())
		return IniVariable();

	IniVariable var = retrieveSetting(sectionName, variableName);
	if (var.isNull())
		return createSetting(sectionName, variableName, QVariant());

	return var;
}

void Ini::setValue(const QString &key, const QVariant &value)
{
	assert(d->provider != nullptr);

	d->provider->setValue(key, value);
}

QVariant Ini::value(const QString &key) const
{
	assert(d->provider != nullptr);

	return d->provider->value(key);
}
