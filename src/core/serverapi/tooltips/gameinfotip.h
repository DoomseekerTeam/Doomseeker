//------------------------------------------------------------------------------
// gameinfotip.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __GAME_INFO_TIP_H_
#define __GAME_INFO_TIP_H_

#include "dptr.h"
#include "serverapi/serverptr.h"
#include <QObject>
#include <QString>

class GameInfoTip : public QObject
{
	Q_OBJECT

public:
	GameInfoTip(const ServerCPtr &server);
	~GameInfoTip() override;

	QString generateHTML();

private:
	DPtr<GameInfoTip> d;

	static const QString UNLIMITED;

	/**
	 * @brief Generic method for spawning HTML row describing a game limit.
	 *
	 * Format is:
	 * @code
	 *<limitName>: <valueArgsTemplate>
	 * @endcode
	 *
	 * @param limitName - Name of the limit.
	 * @param valueArgsTemplate - Placeholder for values. Must be in format
	 *     understood by QString.arg() AND @b MUST have at least one
	 *     placeholder for value argument.
	 * @param value - This will replace %1 argument. If equal
	 *     to zero, UNLIMITED string will be replaced instead.
	 * @return HTML row ready to be put inside a table.
	 */
	QString limitHTML(QString limitName, QString valueArgsTemplate, int value);

	QString playersHTML();
	QString scorelimitHTML();
	QString teamScoresHTML();
	QString timelimitHTML();
};

#endif
