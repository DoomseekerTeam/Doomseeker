//------------------------------------------------------------------------------
// localizationinfo.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_LOCALIZATIONINFO_H
#define DOOMSEEKER_LOCALIZATIONINFO_H

#include <QIODevice>
#include <QList>
#include <QString>

class LocalizationInfo
{
public:
	/**
	 * Not a real localization, but a special value that tells the
	 * program to try to always follow the current language of the OS.
	 */
	static const LocalizationInfo SYSTEM_FOLLOW;
	/**
	 * The default localization - English - is hardcoded into
	 * the program and we don't need to have it in any .def file.
	 */
	static const LocalizationInfo PROGRAM_NATIVE;
	static LocalizationInfo findBestMatch(const QList<LocalizationInfo> &candidates, const QString &localeName);

	/**
	 * @brief The same as code used for country flags in IP2C.
	 */
	QString countryCodeName;
	/**
	 * @brief Compliant with language_country standard. See
	 *        QLocale::name()
	 */
	QString localeName;
	/**
	 * @brief Name that will be displayed to user.
	 */
	QString niceName;

	void fromString(const QString &str);
	QString toString() const;
	bool isValid() const;

	bool operator==(const LocalizationInfo &o2) const;
	bool operator!=(const LocalizationInfo &o2) const
	{
		return !(*this == o2);
	}
};

enum LocaleMatch
{
	LocaleMatchCompletely,
	LocaleMatchLanguage,
	LocaleNoMatch
};

/**
 * Compares two locale strings and returns match score for them.
 *
 * Comparisons are case-insensitive.
 *
 * @return Match score:
 *     # LocaleNoMatch - if locales mismatch completely
 *     # LocaleMatchLanguage - if 'language' matches
 *     # LocaleMatchCompletely - if 'language_country' matches
 */
LocaleMatch matchLocale(const QString &localeName1, const QString &localeName2);

#endif
