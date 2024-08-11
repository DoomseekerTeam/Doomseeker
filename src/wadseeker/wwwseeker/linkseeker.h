//------------------------------------------------------------------------------
// linkseeker.h
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
#ifndef WADSEEKER_WWWSEEKER_LINKSEEKER_H
#define WADSEEKER_WWWSEEKER_LINKSEEKER_H

#include "dptr.h"

#include <QObject>

class Link;
class QIODevice;
class WebCrawl;

/**
 * @brief Processes the incoming QIODevices, looking for any links they may
 * provide.
 *
 * This object is asynchronous. It accepts a QIODevice and returns
 * immediately. Each QIODevice is identified by its source URL, which
 * can be anything really, but should be unique within a single LinkSeeker.
 *
 * Found links are emitted over time via the linkFound() signal, and
 * when the seeker is done with a given device, it emits finished().
 */
class LinkSeeker : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(LinkSeeker)

public:
	LinkSeeker();
	~LinkSeeker();

	void acceptHtml(const WebCrawl &src, QIODevice *io);
	bool isWorking() const;

public slots:
	void conclude(const QIODevice *io);

signals:
	void finished(const WebCrawl &src);
	void linkFound(const WebCrawl &src, const Link &link);

private:
	DPtr<LinkSeeker> d;
};

#endif
