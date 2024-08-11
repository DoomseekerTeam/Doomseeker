//------------------------------------------------------------------------------
// modfile.h
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
#ifndef ided18842d_ed12_42b5_9c38_c21abea3480f
#define ided18842d_ed12_42b5_9c38_c21abea3480f

#include "../wadseekerexportinfo.h"
#include "../dptr.h"

#include <QString>
#include <QUrl>

class Checksum;

/**
 * @brief A file belonging to a mod.
 *
 * This class describes a file that is part of a mod. It's centered
 * around the name of the file and its integrity checksums, but also can
 * provide info on the mod itself.
 */
class WADSEEKER_API ModFile
{
public:
	ModFile();
	ModFile(const QString &name);
	ModFile(const ModFile &other);
	virtual ~ModFile();
	ModFile &operator=(const ModFile &other);

	/**
	 * @brief The name of the mod file.
	 */
	const QString &fileName() const;
	/**
	 * @brief Set the name of the mod file.
	 */
	void setFileName(const QString &) const;

	/**
	 * @brief The title, or *pretty name*, of the mod.
	 */
	const QString &name() const;
	/**
	 * @brief Set the title of the mod.
	 */
	void setName(const QString &) const;

	/**
	 * @brief Description of the mod.
	 */
	const QString &description() const;
	/**
	 * @brief Set a description of the mod.
	 */
	void setDescription(const QString &) const;

	/**
	 * @brief List of checksums that verify the correctness of this file.
	 *
	 * This is a list because more than one hashing algorithm can be
	 * supported. There should not be two Checksum structs on the list
	 * with the same algorithm.
	 */
	const QList<Checksum> &checksums() const;
	/**
	 * @brief Set a list of checksums that verify the correctness of this file.
	 */
	void setChecksums(const QList<Checksum> &) const;

	/**
	 * @brief The known URL to download the mod.
	 */
	const QUrl &url() const;
	/**
	 * @brief Set the known URL to download the mod.
	 */
	void setUrl(const QUrl &) const;

	/**
	 * @brief The version of the mod.
	 */
	const QString &version() const;
	/**
	 * @brief Set the version of the mod.
	 */
	void setVersion(const QString &) const;

private:
	DPtr<ModFile> d;
};

#endif
