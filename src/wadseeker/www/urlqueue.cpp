//------------------------------------------------------------------------------
// urlqueue.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "urlqueue.h"

const QString UrlQueue::UNNAMED_RESOURCE;

UrlQueue::UrlQueue()
{
}

UrlQueue::~UrlQueue()
{
}

void UrlQueue::add(UrlMirror mirror, int priority)
{
	for (auto it = mirror.urls.begin(); it != mirror.urls.end();)
	{
		if (isUrlKnown(*it))
			it = mirror.urls.erase(it);
		else
			++it;
	}
	if (!mirror.isEmpty())
	{
		for (QUrl url : mirror.urls)
			this->knownUrls.insert(url);
		if (!this->mirrors.contains(priority))
			this->mirrors.insert(priority, QList<UrlMirror>());
		this->mirrors[priority] << mirror;
	}
}

void UrlQueue::addMirrorUrls(const QList<QUrl> &urls, const QString &resource, int priority)
{
	UrlMirror mirror;
	mirror.resource = resource;
	mirror.urls = urls;
	add(mirror, priority);
}

void UrlQueue::addUrl(const QUrl &url, const QString &resource, int priority)
{
	QList<QUrl> urls;
	urls << url;
	addMirrorUrls(urls, resource, priority);
}

void UrlQueue::compact()
{
	for (auto priorityIt = this->mirrors.begin(); priorityIt != this->mirrors.end();)
	{
		QList<UrlMirror> &mirrors = *priorityIt;
		for (auto mirrorIt = mirrors.begin(); mirrorIt != mirrors.end();)
		{
			// Check if this mirror still has unused URLs.
			auto urlIt = mirrorIt->urls.begin();
			for (; urlIt != mirrorIt->urls.end(); ++urlIt)
			{
				if (!this->takenOutUrls.contains(*urlIt))
					break;
			}
			if (urlIt == mirrorIt->urls.end())
			{
				// If we reached this point, the entire mirror is spent and
				// we should remove it.
				mirrorIt = mirrors.erase(mirrorIt);
			}
			else
			{
				++mirrorIt;
			}
		}

		// Remove priority level if there are no more mirrors on it.
		if (mirrors.isEmpty())
			priorityIt = this->mirrors.erase(priorityIt);
		else
			++priorityIt;
	}
}

bool UrlQueue::isEmpty() const
{
	return size() == 0;
}

bool UrlQueue::isUrlKnown(const QUrl &url) const
{
	return this->knownUrls.contains(url);
}

bool UrlQueue::hasUrl(const QUrl &url) const
{
	return this->knownUrls.contains(url) && !this->takenOutUrls.contains(url);
}

QList<UrlMirror> UrlQueue::mirrorsWithUrl(const QUrl &url)
{
	QList<UrlMirror> mirrorsWithUrls;
	for (auto priorityIt = this->mirrors.begin(); priorityIt != this->mirrors.end(); ++priorityIt)
	{
		for (UrlMirror mirror : *priorityIt)
		{
			if (mirror.hasUrl(url))
			{
				mirrorsWithUrls << mirror;
			}
		}
	}

	return mirrorsWithUrls;
}

QList<UrlQueue::PrioUrl> UrlQueue::peek(const QString &resource)
{
	QList<PrioUrl> urls;
	for (auto priorityIt = this->mirrors.begin(); priorityIt != this->mirrors.end(); ++priorityIt)
	{
		QList<UrlMirror> &mirrors = *priorityIt;
		for (const UrlMirror &mirror : mirrors)
		{
			if (mirror.resource == resource)
			{
				for (const QUrl &url : mirror.urls)
				{
					if (!this->takenOutUrls.contains(url))
						urls << PrioUrl { priorityIt.key(), url };
				}
			}
		}
	}
	return urls;
}

void UrlQueue::removeAllUrls(const UrlMirror &mirror)
{
	for (const QUrl &url : mirror.urls)
	{
		removeUrl(url);
	}
}

void UrlQueue::removeUrl(const QUrl &url)
{
	this->knownUrls.insert(url);
	this->takenOutUrls.insert(url);
}

void UrlQueue::removeUrlAndMirrors(const QUrl &url)
{
	QList<UrlMirror> mirrors = mirrorsWithUrl(url);
	for (const UrlMirror &mirror : mirrors)
	{
		removeAllUrls(mirror);
	}
}

unsigned UrlQueue::size() const
{
	return this->knownUrls.size() - this->takenOutUrls.size();
}

void UrlQueue::takenOut(const QUrl &url)
{
	this->knownUrls.insert(url);
	this->takenOutUrls.insert(url);
	compact();
}
