//------------------------------------------------------------------------------
// serverpasswordsummary.h
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
#ifndef idA2EA6505_8AD6_40F7_AD19A3DFD8DBC961
#define idA2EA6505_8AD6_40F7_AD19A3DFD8DBC961

#include "dptr.h"
#include <QString>
#include <QVariant>

class Server;
class ServerSummary;

class ServerPasswordType
{
public:
	static const QString CONNECT;
	static const QString INGAME;
};

class ServerPasswordSummary
{
public:
	static ServerPasswordSummary deserializeQVariant(const QVariant &v);

	ServerPasswordSummary();
	/**
	 * @brief Builds ServerPasswordSummary out of provided data.
	 *
	 * @param server
	 *     Data such as game or address are extracted from this object.
	 * @param type
	 *     Password use-case type, either one of ServerPasswordType consts
	 *     or custom.
	 */
	ServerPasswordSummary(const Server *server, const QString &type);
	~ServerPasswordSummary();

	const QString &address() const;
	const QString &game() const;
	bool isValid() const;
	const QString &name() const;
	unsigned short port() const;

	QVariant serializeQVariant() const;

	const ServerSummary &serverSummary() const;
	void setServerSummary(const ServerSummary &val);

	/**
	 * @brief One of ServerPasswordType consts or custom.
	 */
	void setType(const QString &val);

	/**
	 * @brief Similarity to the 'other' server; between 0.0 and 1.0.
	 *
	 * Similarity is calculated basing on serverSummary() and type().
	 *
	 * Both servers must be valid.
	 */
	float similarity(const ServerPasswordSummary &other) const;

	QDateTime time() const;

	/**
	 * @brief One of ServerPasswordType consts or custom.
	 */
	const QString &type() const;
	/**
	 * @brief Returns type(), if type() is empty returns
	 *        ServerPasswordType::CONNECT .
	 *
	 * This provides compatibility with configuration stored by
	 * Doomseeker <=0.11.1
	 */
	const QString &typeWithCompatibility() const;

private:
	DPtr<ServerPasswordSummary> d;
};

#endif
