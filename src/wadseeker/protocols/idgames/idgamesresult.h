//------------------------------------------------------------------------------
// idgamesresult.h
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
#ifndef id483B475A_D9FB_4D7B_948EB4BC7878EF18
#define id483B475A_D9FB_4D7B_948EB4BC7878EF18

#include <QVariant>

/**
 * @brief General result of idgames archive public API query.
 */
class IdgamesResult
{
public:
	static IdgamesResult mkError(const QString &type, const QString &message);

	IdgamesResult() {}
	IdgamesResult(const QVariant &rawData);

	QVariant content() const;
	QString errorType() const;
	QString errorMessage() const;
	/**
	 * @brief true for either warning and error.
	 */
	bool hasErrorMessage() const;
	bool isError() const;
	bool isNull() const;
	bool isOk() const;
	bool isWarning() const;

private:
	QVariantMap data;

	QString errorKeyword() const;
};

#endif
