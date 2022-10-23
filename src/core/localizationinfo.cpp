//-----------------------------------------------------------------------------
// localizationinfo.cpp
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//-----------------------------------------------------------------------------
#include "localizationinfo.h"

#include <QLocale>
#include <QStringList>

const LocalizationInfo LocalizationInfo::SYSTEM_FOLLOW = {
	// Unlike other translations, the "display name" in this one needs
	// to be displayed in the currently active language. We cannot use
	// tr() here because that would be init exactly once and also most
	// likely before the translations are loaded.
	"", "system", "system default"
};

const LocalizationInfo LocalizationInfo::PROGRAM_NATIVE = {
	"GBR", "en", "English"
};

LocalizationInfo LocalizationInfo::findBestMatch(
	const QList<LocalizationInfo> &candidates,
	const QString &localeName)
{
	LocalizationInfo matchLanguage;
	for (const LocalizationInfo &candidate : candidates)
	{
		LocaleMatch matchScore = matchLocale(candidate.localeName, localeName);
		switch (matchScore)
		{
		case LocaleMatchCompletely:
			return candidate;
		case LocaleMatchLanguage:
			matchLanguage = candidate;
			break;
		default:
			// do nothing
			break;
		}
	}
	return matchLanguage;
}

bool LocalizationInfo::isValid() const
{
	return !localeName.isEmpty();
}

bool LocalizationInfo::operator==(const LocalizationInfo &o2) const
{
	return countryCodeName == o2.countryCodeName
		&& QLocale(localeName) == QLocale(o2.localeName)
		&& niceName == o2.niceName;
}

QString LocalizationInfo::toString() const
{
	return QString("%1;%2;%3").arg(countryCodeName, localeName, niceName);
}

////////////////////////////////////////

LocaleMatch matchLocale(const QString &localeName1, const QString &localeName2)
{
	if (localeName1.compare(localeName2, Qt::CaseInsensitive) == 0)
		return LocaleMatchCompletely;
	else if (localeName1.split("_")[0].compare(localeName2.split("_")[0], Qt::CaseInsensitive) == 0)
		return LocaleMatchLanguage;
	else
		return LocaleNoMatch;
}
