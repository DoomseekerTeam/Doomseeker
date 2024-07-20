//------------------------------------------------------------------------------
// linkseeker.cpp
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
#include "linkseeker.h"
#include "dptr.h"

#include "www/webcrawl.h"
#include "wwwseeker/link.h"
#include "wwwseeker/linksieve.h"

#include <QIODevice>
#include <QSharedPointer>
#include <QUrl>

namespace
{
struct SieveJob
{
	WebCrawl src;
	QIODevice *io;
	QSharedPointer<LinkSieveHtml> sieve;
};

class SieveJobList : public QList<SieveJob>
{
public:
	SieveJobList::iterator findBySieve(LinkSieveHtml *sieve)
	{
		auto it = this->begin();
		for (; it != this->end() && it->sieve.data() != sieve; ++it);
		return it;
	}
};
}

DClass<LinkSeeker>
{
public:
	SieveJobList sieves;
};
DPointeredNoCopy(LinkSeeker);

LinkSeeker::LinkSeeker()
{
}

LinkSeeker::~LinkSeeker()
{
}

void LinkSeeker::acceptHtml(const WebCrawl &src, QIODevice *io)
{
	auto sieve = QSharedPointer<LinkSieveHtml>::create(io);
	auto sievePtr = sieve.data();
	d->sieves << SieveJob {src, io, sieve};

	connect(sieve.data(), &LinkSieveHtml::link,
		this, [this, src](const Link &link)
		{
			emit this->linkFound(src, link);
		});
	connect(sieve.data(), &LinkSieveHtml::finished,
		this, [this, src, sievePtr]()
		{
			auto jobIt = d->sieves.findBySieve(sievePtr);
			if (jobIt != d->sieves.end())
				d->sieves.erase(jobIt);
			emit this->finished(src);
		});
}

void LinkSeeker::conclude(const QIODevice *io)
{
	for (SieveJob &job : d->sieves)
	{
		if (job.io == io)
		{
			job.sieve->conclude();
			break;
		}
	}
}

bool LinkSeeker::isWorking() const
{
	return !d->sieves.isEmpty();
}
