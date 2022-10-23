//------------------------------------------------------------------------------
// localization.h
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
#ifndef DOOMSEEKER_LOCALIZATION_H
#define DOOMSEEKER_LOCALIZATION_H

#include "localizationinfo.h"

#include <QIODevice>
#include <QList>
#include <QString>
#include <QTranslator>

class Localization
{
public:
	QList<LocalizationInfo> localizations;

	static Localization *get();

	/**
	 * Find best matching locale from currently loaded translations.
	 * Falls back to LocalizationInfo::PROGRAM_NATIVE if no match is found.
	 */
	LocalizationInfo coerceBestMatchingLocalization(const QString &localeName) const;
	const LocalizationInfo &currentLocalization() const;

	bool isCurrentlyLoaded(const QString &localeName) const;
	bool loadTranslation(const QString &localeName);
	QList<LocalizationInfo> loadLocalizationsList(const QStringList &definitionsFileSearchDirs);

private:
	class LocalizationLoader;

	static Localization *instance;

	Localization();

	QList<QTranslator *> currentlyLoadedTranslations;
	LocalizationInfo currentLocalization_;

	void installQtTranslations(const QString &localeName, QStringList searchPaths);
	bool installTranslation(const QString &translationName, const QStringList &searchPaths);
	QTranslator *loadTranslationFile(const QString &translationName, const QStringList &searchPaths);
};

#endif
