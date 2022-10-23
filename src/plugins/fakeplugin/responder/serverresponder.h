//------------------------------------------------------------------------------
// serverresponder.h
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef PLUGIN_RESPONDER_SERVERRESPONDER_H
#define PLUGIN_RESPONDER_SERVERRESPONDER_H

#include <QObject>

class ServerResponder : public QObject
{
	Q_OBJECT

public:
	static const int MAX_CHANCE = 65535;

	ServerResponder(QObject *parent = nullptr);
	~ServerResponder();

	bool bind(unsigned short port);
	/**
	 * @brief Port this responder is bound to, or zero if
	 *       not bound.
	 */
	unsigned short port() const;

	/**
	 * @brief Set chance for this server to not respond to a query.
	 *
	 * @param chance
	 *     A value between 0 and MAX_CHANCE, inclusive.
	 *     A value of zero means that no fails will occur,
	 *     max value means that fails will always occur.
	 */
	void setResponseFailChance(unsigned chance);

private:
	class PrivData;

	PrivData *d;

	bool shouldRespond() const;

private slots:
	void readPendingDatagrams();
	void readPendingDatagram();
	void respond();
};

#endif
