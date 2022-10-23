//------------------------------------------------------------------------------
// joinerror.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id0375B082_FEE9_4270_B314893A57DB3840
#define id0375B082_FEE9_4270_B314893A57DB3840

#include "dptr.h"
#include "global.h"
#include <QString>
#include <QtContainerFwd>

class PWad;

/**
 * @ingroup group_pluginapi
 * @brief Indicator of error for the server join process.
 *
 * This structure contains information about whether an error occured and
 * if it did - what type of error it is. Based on this GUI can make a
 * decision on how to handle the error and whether to try again.
 */
class MAIN_EXPORT JoinError
{
public:
	enum JoinErrorType
	{
		NoError,
		MissingWads,
		ConfigurationError,
		Critical,
		/**
		 * @brief Aborts without printing error.
		 *
		 * Useful if plugin wants to do its own error handling.
		 */
		Terminate,
		/**
		 * @brief Game executable was not found but it can be automatically
		 *        installed by the plugin.
		 */
		CanAutomaticallyInstallGame
	};

	JoinError();
	JoinError(JoinErrorType errorType);
	JoinError(const JoinError &other);
	JoinError &operator=(const JoinError &other);
	virtual ~JoinError();

	void addMissingWad(const PWad &wad);
	void clearMissingWads();

	const QString &error() const;
	bool isError() const;
	bool isMissingIwadOnly() const;
	bool isMissingWadsError() const;

	/**
	 * This is valid only if type == MissingWads.
	 */
	const QString &missingIwad() const;

	/**
	 * This is valid only if type == MissingWads.
	 */
	const QList<PWad> &missingWads() const;
	/**
	 * This is valid only if type == MissingWads.
	 */
	const QList<PWad> &incompatibleWads() const;

	void setError(const QString &error);
	void setMissingIwad(const QString &iwad);
	void setMissingWads(const QList<PWad> &wads);
	void setIncompatibleWads(const QList<PWad> &wads);
	void setType(JoinErrorType type);

	JoinErrorType type() const;

private:
	DPtr<JoinError> d;
};

#endif
