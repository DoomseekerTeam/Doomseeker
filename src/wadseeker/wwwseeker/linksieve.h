//------------------------------------------------------------------------------
// linksieve.h
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef WADSEEKER_WWWSEEKER_LINKSIEVE_H
#define WADSEEKER_WWWSEEKER_LINKSIEVE_H

#include "dptr.h"

#include <QObject>

class Link;
class QIODevice;

/**
 * Seek for links to files in a HTML QIODevice.
 *
 * This object is asynchronous. It accepts a QIODevice and connects to
 * its signals. Each encountered Link is emitted via link(). When it's
 * done with a QIODevice, it emits the finished() signal.
 */
class LinkSieveHtml : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(LinkSieveHtml)

public:
	LinkSieveHtml(QIODevice *device);
	~LinkSieveHtml();

public:
	void conclude();

signals:
	void finished();
	void link(const Link &link);

private:
	DPtr<LinkSieveHtml> d;

private slots:
	void slurp();
};

#endif
