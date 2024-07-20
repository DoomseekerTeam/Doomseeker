//------------------------------------------------------------------------------
// linksieve.cpp
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
#include "linksieve.h"
#include "dptr.h"

#include "wwwseeker/htmlparser.h"
#include "wwwseeker/link.h"
#include <QByteArray>
#include <QIODevice>

DClass<LinkSieveHtml>
{
public:
	bool concluded;
	QIODevice *io;
	QByteArray buffer;
};
DPointeredNoCopy(LinkSieveHtml);

LinkSieveHtml::LinkSieveHtml(QIODevice *io)
{
	d->concluded = false;
	d->io = io;

	connect(io, &QIODevice::readyRead,
		this, &LinkSieveHtml::slurp);
	if (io->bytesAvailable() > 0)
		emit io->readyRead();
}

LinkSieveHtml::~LinkSieveHtml()
{
}

void LinkSieveHtml::conclude()
{
	if (d->concluded)
		return;

	d->concluded = true;
	this->disconnect(d->io);
	this->slurp();

	HtmlParser html(d->buffer);
	QList<Link> links = html.linksFromHtml();
	for (Link link : links)
		emit this->link(link);

	emit finished();
}

void LinkSieveHtml::slurp()
{
	if (d->io->bytesAvailable() > 0)
		d->buffer += d->io->readAll();
}
