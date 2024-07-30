//------------------------------------------------------------------------------
// fileseekinfo.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef WADSEEKER_WWWSEEKER_FILESEEKINFO_H
#define WADSEEKER_WWWSEEKER_FILESEEKINFO_H

#include <QString>
#include <QStringList>

/**
 * @brief Define the details of looking for a given file.
 *
 * The same seeked file can be listed under different filenames. This class
 * wraps itself around the seeked filename and provides the list of variant
 * filenames.
 *
 * For the sake of containers, the `operator==` and `operator!=` are implemented
 * but check for equality of the seeked filename only, case-insensitively.
 */
class FileSeekInfo
{
public:
	/**
	 * @brief Creates an invalid FileSeekInfo object.
	 */
	FileSeekInfo();

	/**
	 * @brief Creates a FileSeekInfo object with a file name and optional
	 *        possibleFilenames list.
	 *
	 * @param file
	 *     The name of the file that interests us. Two FileSeekInfo objects that
	 *     share the same file are considered equal. Comparison operators for
	 *     this value are case-insensitive.
	 * @param possibleFilenames
	 *     Optional. List of possible filenames for the file. If list is
	 *     empty, the object is treated as empty; isEmpty() returns true.
	 *     Empty and whitespace values are dropped from the list.
	 *     Duplicates are dropped.
	 */
	FileSeekInfo(const QString &file, const QStringList &possibleFilenames = QStringList());

	/**
	 * @brief Name under which this file is identified.
	 */
	const QString &file() const
	{
		return m_file;
	}

	/**
	 * @brief Compares passed string against possibleFilenames() list.
	 *
	 * Compare is case-insensitive. Doesn't compare against file().
	 */
	bool isFilenameListed(const QString &filename) const;

	/**
	 * @brief True if file() returns a non-empty non-whitespace string.
	 */
	bool isValid() const
	{
		return !file().trimmed().isEmpty();
	}

	/**
	 * @brief Compares passed param against value returned by file().
	 *
	 * Check is case-insensitive.
	 *
	 * @return True if file param string is equal to file().
	 */
	bool operator==(const QString &file) const;

	/**
	 * @brief Compares file() of both sides.
	 *
	 * Check is case-insensitive.
	 *
	 * @return True if values returned by file() are equal.
	 */
	bool operator==(const FileSeekInfo &other) const;

	/**
	 * @brief Compares passed param against value returned by file().
	 *
	 * Check is case-insensitive.
	 *
	 * @return True if file param string is different than file().
	 */
	bool operator!=(const QString &file) const;

	/**
	 * @brief List of possible filenames for this file.
	 */
	const QStringList &possibleFilenames() const
	{
		return m_possibleFilenames;
	}

private:
	/**
	 * @brief Name under which the file is identified.
	 */
	QString m_file;
	/**
	 * @brief List of possible filenames for this file.
	 */
	QStringList m_possibleFilenames;
};

#endif
