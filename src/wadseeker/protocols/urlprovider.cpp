//------------------------------------------------------------------------------
// urlprovider.cpp
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
#include "urlprovider.h"

#include "wadseekerversioninfo.h"
#include "wwwseeker/urlparser.h"
#include <QFileInfo>

class MirrorStorage
{
public:
	QList<QUrl> urls;

	bool hasUrl(const QUrl &url) const
	{
		for (const QUrl &urlOnList : urls)
		{
			if (UrlParser::urlEqualsCaseInsensitive(urlOnList, url))
			{
				return true;
			}
		}

		return false;
	}

	bool isEmpty() const
	{
		return urls.isEmpty();
	}
};
///////////////////////////////////////////////////////////////////////////////
URLProvider::URLProvider()
{
	allUrlsPrioritized.insert(PRIORITY_ARCHIVES, QList<QUrl>());
	allUrlsPrioritized.insert(PRIORITY_OTHER, QList<QUrl>());
}

URLProvider::~URLProvider()
{
	for (MirrorStorage *pMirror : mirrors)
	{
		delete pMirror;
	}
}

void URLProvider::addMirror(const QUrl &originalUrl, const QUrl &mirrorUrl)
{
	QList<MirrorStorage *> mirrors = mirrorsWithUrl(originalUrl);
	if (!mirrors.isEmpty())
	{
		if (!hasOrHadUrl(mirrorUrl))
		{
			MirrorStorage *pFirstMirror = mirrors.first();
			pFirstMirror->urls << mirrorUrl;
			insertUrlPrioritized(mirrorUrl);
		}
	}
	else
	{
		QList<QUrl> newUrls;
		newUrls << originalUrl;

		if (!hasOrHadUrl(mirrorUrl))
		{
			newUrls << mirrorUrl;
		}

		addMirrorUrls(newUrls);
	}
}

void URLProvider::addMirrorUrls(const QList<QUrl> &urls)
{
	auto pMirrorStorage = new MirrorStorage();

	for (const QUrl &url : urls)
	{
		if (!hasOrHadUrl(url))
		{
			pMirrorStorage->urls << url;
			insertUrlPrioritized(url);
		}
	}

	if (!pMirrorStorage->isEmpty())
	{
		mirrors << pMirrorStorage;
	}
	else
	{
		delete pMirrorStorage;
	}
}

void URLProvider::addUrl(const QUrl &url)
{
	if (!hasOrHadUrl(url))
	{
		auto pMirrorStorage = new MirrorStorage();
		pMirrorStorage->urls << url;
		insertUrlPrioritized(url);
		mirrors << pMirrorStorage;
	}
}

QList<QUrl> URLProvider::allAvailableUrls() const
{
	QList<QUrl> urls;
	for (const QList<QUrl> &partial : allUrlsPrioritized.values())
	{
		urls += partial;
	}
	return urls;
}

QUrl URLProvider::first() const
{
	return allAvailableUrls().first();
}

void URLProvider::insertUrlPrioritized(const QUrl &url)
{
	QFileInfo fileInfo(url.path());
	if (WadseekerVersionInfo::supportedArchiveExtensions().contains(fileInfo.suffix()))
	{
		allUrlsPrioritized[PRIORITY_ARCHIVES].append(url);
	}
	else
	{
		allUrlsPrioritized[PRIORITY_OTHER].append(url);
	}
}

bool URLProvider::isEmpty() const
{
	for (const QList<QUrl> &chunk : allUrlsPrioritized.values())
	{
		if (!chunk.isEmpty())
		{
			return false;
		}
	}
	return true;
}

bool URLProvider::hasOrHadUrl(const QUrl &url) const
{
	for (MirrorStorage *pMirror : mirrors)
	{
		if (pMirror->hasUrl(url))
		{
			return true;
		}
	}

	return false;
}

bool URLProvider::hasUrl(const QUrl &url) const
{
	for (const QUrl &urlOnList : allAvailableUrls())
	{
		if (UrlParser::urlEqualsCaseInsensitive(urlOnList, url))
		{
			return true;
		}
	}
	return false;
}

QList<MirrorStorage *> URLProvider::mirrorsWithUrl(const QUrl &url)
{
	QList<MirrorStorage *> mirrorsWithUrls;
	for (MirrorStorage *pMirror : mirrors)
	{
		if (pMirror->hasUrl(url))
		{
			mirrorsWithUrls << pMirror;
		}
	}

	return mirrorsWithUrls;
}

unsigned URLProvider::numUrls() const
{
	return allAvailableUrls().size();
}

URLProvider &URLProvider::operator<<(const QUrl &url)
{
	addUrl(url);
	return *this;
}

void URLProvider::removeAllUrls(const MirrorStorage *pMirror)
{
	for (const QUrl &url : pMirror->urls)
	{
		removeUrl(url);
	}
}

void URLProvider::removeUrl(const QUrl &url)
{
	for (int key : allUrlsPrioritized.uniqueKeys())
	{
		allUrlsPrioritized[key].removeAll(url);
	}
}

void URLProvider::removeUrlAndMirrors(const QUrl &url)
{
	QList<MirrorStorage *> mirrors = mirrorsWithUrl(url);
	for (MirrorStorage *pMirror : mirrors)
	{
		removeAllUrls(pMirror);
	}
}

QUrl URLProvider::takeFirst()
{
	QUrl url = first();
	removeUrl(url);
	return url;
}
