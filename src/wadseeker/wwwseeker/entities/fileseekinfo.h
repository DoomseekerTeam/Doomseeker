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
#ifndef __FILESEEKINFO_H__
#define __FILESEEKINFO_H__

#include <QString>
#include <QStringList>

/**
 * @brief Seek information for WWWSeeker
 *
 * The same seeked file can be listed under different filenames. Objects of
 * this class group a list of filenames under a single key value.
 */
class FileSeekInfo
{
public:
	/**
	 * @brief Creates invalid FileSeekInfo object.
	 */
	FileSeekInfo();

	/**
	 * @brief Creates a FileSeekInfo object with file name and optional
	 *        possibleFilenames list.
	 *
	 * @param file
	 *      Unique name of the file. Comparison operators for this value are
	 *      case-insensitive.
	 * @param possibleFilenames
	 *      Optional. List of possible filenames for the file. If list is
	 *      empty, the object is treated as empty - isEmpty() returns true.
	 *      Empty and whitespace values are dropped from the list.
	 *      Duplicates are dropped.
	 */
	FileSeekInfo(const QString &file, const QStringList &possibleFilenames = QStringList());

	/**
	 * @brief Adds a new filename to possibleFilenames() list.
	 *
	 * Duplicate values are dropped. Duplicate check is case-insensitive.
	 * A no-op if isValid() returns false.
	 *
	 * @param filename
	 *      Possible name of the file. If empty or whitespaced this is
	 *      a no-op.
	 */
	void addPossibleFilename(const QString &filename);

	/**
	 * @brief Name under which this file will be identified.
	 */
	const QString &file() const
	{
		return d.file;
	}

	/**
	 * @brief True if possibleFilenames() is empty.
	 */
	bool isEmpty() const
	{
		return possibleFilenames().isEmpty();
	}

	/**
	 * @brief Compares passed string against possibleFilenames() list.
	 *
	 * Compare is case-insensitive. Will @b NOT compare against file().
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
		return d.possibleFilenames;
	}

private:
	class PrivData
	{
	public:
		/**
		 * @brief Name under which the file will be identified.
		 */
		QString file;

		/**
		 * @brief List of possible filenames for this file.
		 */
		QStringList possibleFilenames;
	};

	PrivData d;
};

#endif
