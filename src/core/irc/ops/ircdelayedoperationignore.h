//------------------------------------------------------------------------------
// ircdelayedoperationignore.h
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
#ifndef id6d18a0bf_8b5a_44d2_be14_a0de3c5175fe
#define id6d18a0bf_8b5a_44d2_be14_a0de3c5175fe

#include "dptr.h"
#include "irc/ops/ircdelayedoperation.h"

class IRCNetworkAdapter;

class IRCDelayedOperationIgnore : public IRCDelayedOperation
{
	Q_OBJECT

public:
	IRCDelayedOperationIgnore(QWidget *parent, IRCNetworkAdapter *network, const QString &nickname);
	~IRCDelayedOperationIgnore() override;

	/**
	 * If disabled, a '*!*@host' pattern is assumed. Disabled by default.
	 */
	void setShowPatternPopup(bool b);
	void start();

private:
	DPtr<IRCDelayedOperationIgnore> d;

private slots:
	void onWhoIsUser(const QString &nickname, const QString &user,
		const QString &hostName);
};

#endif
