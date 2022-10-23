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
 * @brief Class to store all necessary (and optional) information of a mod.
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
	 * @brief returns the name of the file that contains the mod.
	 */
	const QString &fileName() const;
	/**
	 * @brief Sets the name of the file containing the mod.
	 */
	void setFileName(const QString &) const;

	/**
	 * @brief returns the pretty name of the mod.
	 */
	const QString &name() const;
	/**
	 * @brief Sets a pretty name of the mod.
	 */
	void setName(const QString &) const;

	/**
	 * @brief returns the description for the mod.
	 */
	const QString &description() const;
	/**
	 * @brief Sets a description for the mod.
	 */
	void setDescription(const QString &) const;

	/**
	 * @brief returns the list of checksums accepted for the mod.
	 */
	const QList<Checksum> &checksums() const;
	/**
	 * @brief Sets a list of checksums accepted for the mod.
	 */
	void setChecksums(const QList<Checksum> &) const;

	/**
	 * @brief returns the known url to download the mod.
	 */
	const QUrl &url() const;
	/**
	 * @brief Sets a known url to download the mod.
	 */
	void setUrl(const QUrl &) const;

	/**
	 * @brief returns the version for the mod.
	 */
	const QString &version() const;
	/**
	 * @brief Sets a version for the mod.
	 */
	void setVersion(const QString &) const;

private:
	DPtr<ModFile> d;
};

#endif
