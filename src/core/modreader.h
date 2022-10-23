//------------------------------------------------------------------------------
// modreader.h
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------
#ifndef __MODREADER_H_
#define __MODREADER_H_

#include "dptr.h"

#include <QSharedPointer>
#include <QString>

/**
 * @brief Contains an entry of a WAD directory.
 *
 * it consists of the name of the entry, the position in the file that the
 * related lump is located and the size of said lump.
 *
 */
struct DirectoryEntry
{
	QString name;
	int position;
	int size;
};

class ModReader
{
public:
	virtual ~ModReader() {}
	virtual bool load() = 0;
	virtual QStringList getAllMaps() = 0;
	virtual bool isIwad() = 0;
	static QSharedPointer<ModReader> create(const QString &path);
};

/**
 * @brief File reader for the WAD format.
 */
class WadReader : public ModReader
{
public:
	/**
	 * @brief Construct a new WAD reader to manage a WAD file.
	 *
	 * @param path absolute path to WAD file.
	 */
	WadReader(const QString &path);
	~WadReader() override;
	/**
	 * @brief loads the WAD set in the constructor.
	 *
	 * @return true if everything was successful.
	 */
	bool load() override;
	/**
	 * @brief returns whether or not this is a IWAD.
	 */
	bool isIwad() override;
	/**
	 * @brief Get all the directory of the WAD.
	 */
	QList<DirectoryEntry> getDirectory();
	/**
	 * @brief Get a list of the maps that this WAD contains.
	 *
	 * @return QStringList list of maps.
	 */
	QStringList getAllMaps() override;

private:
	/**
	 * @brief Get the Classic-formatted Maps.
	 */
	QStringList getClassicMaps(const QStringList &names);
	/**
	* @brief Get the UDMF-formatted Maps.
	*/
	QStringList getUdmfMaps(const QStringList &names);

private:
	DPtr<WadReader> d;
};

/**
 * @brief File reader for generic compression file formats.
 * Current formats: ZIP, 7Z
 */
class CompressedReader : public ModReader
{
public:
	/**
	 * @brief Construct a new reader to manage the compressed file.
	 *
	 * @param path absolute path to the compressed file.
	 */
	CompressedReader(const QString &path);
	~CompressedReader() override;
	/**
	 * @brief loads the compressed file set in the constructor.
	 *
	 * @return true if everything was successful.
	 */
	bool load() override;
	/**
	 * @brief returns whether or not this is a IWAD. It will always be false.
	 */
	bool isIwad() override
	{
		return false;
	}
	/**
	 * @brief Get a list of the maps that this compressed file contains.
	 *
	 * @return QStringList list of maps.
	 */
	QStringList getAllMaps() override;
protected:
	QStringList getAllMapsRootDir();
	DPtr<CompressedReader> d;
};

/**
 * @brief File reader for PK3/7 format.
 */
class PkReader : public CompressedReader
{
public:
	PkReader(const QString &path);
	/**
	 * @brief Get a list of the maps that this PK3/7 contains.
	 *
	 * @return QStringList list of maps.
	 */
	QStringList getAllMaps() override;
};

#endif
