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

/*
  MirrorStorage
*/
bool MirrorStorage::hasUrl(const QUrl &url) const
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

/*
  UrlProvider
*/
UrlProvider::UrlProvider()
{
	allUrlsPrioritized.insert(PRIORITY_ARCHIVES, QList<QUrl>());
	allUrlsPrioritized.insert(PRIORITY_OTHER, QList<QUrl>());
}

UrlProvider::~UrlProvider()
{
}

void UrlProvider::addMirror(const QUrl &originalUrl, const QUrl &mirrorUrl)
{
	QList<MirrorStorage> mirrors = mirrorsWithUrl(originalUrl);
	if (!mirrors.isEmpty())
	{
		if (!hasOrHadUrl(mirrorUrl))
		{
			MirrorStorage &firstMirror = mirrors.first();
			firstMirror.urls << mirrorUrl;
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

void UrlProvider::addMirrorUrls(const QList<QUrl> &urls)
{
	MirrorStorage mirrorStorage;

	for (const QUrl &url : urls)
	{
		if (!hasOrHadUrl(url))
		{
			mirrorStorage.urls << url;
			insertUrlPrioritized(url);
		}
	}

	if (!mirrorStorage.isEmpty())
	{
		mirrors << mirrorStorage;
	}
}

void UrlProvider::addUrl(const QUrl &url)
{
	if (!hasOrHadUrl(url))
	{
		MirrorStorage mirrorStorage;
		mirrorStorage.urls << url;
		insertUrlPrioritized(url);
		mirrors << mirrorStorage;
	}
}

QList<QUrl> UrlProvider::allAvailableUrls() const
{
	QList<QUrl> urls;
	for (const QList<QUrl> &partial : allUrlsPrioritized.values())
	{
		urls += partial;
	}
	return urls;
}

QUrl UrlProvider::first() const
{
	return allAvailableUrls().first();
}

void UrlProvider::insertUrlPrioritized(const QUrl &url)
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

bool UrlProvider::isEmpty() const
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

bool UrlProvider::hasOrHadUrl(const QUrl &url) const
{
	for (MirrorStorage mirror : mirrors)
	{
		if (mirror.hasUrl(url))
		{
			return true;
		}
	}

	return false;
}

bool UrlProvider::hasUrl(const QUrl &url) const
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

QList<MirrorStorage> UrlProvider::mirrorsWithUrl(const QUrl &url)
{
	QList<MirrorStorage> mirrorsWithUrls;
	for (MirrorStorage mirror : mirrors)
	{
		if (mirror,hasUrl(url))
		{
			mirrorsWithUrls << mirror;
		}
	}

	return mirrorsWithUrls;
}

unsigned UrlProvider::numUrls() const
{
	return allAvailableUrls().size();
}

UrlProvider &UrlProvider::operator<<(const QUrl &url)
{
	addUrl(url);
	return *this;
}

void UrlProvider::removeAllUrls(const MirrorStorage &mirror)
{
	for (const QUrl &url : mirror.urls)
	{
		removeUrl(url);
	}
}

void UrlProvider::removeUrl(const QUrl &url)
{
	for (int key : allUrlsPrioritized.keys())
	{
		allUrlsPrioritized[key].removeAll(url);
	}
}

void UrlProvider::removeUrlAndMirrors(const QUrl &url)
{
	QList<MirrorStorage> mirrors = mirrorsWithUrl(url);
	for (const MirrorStorage &mirror : mirrors)
	{
		removeAllUrls(mirror);
	}
}

QUrl UrlProvider::takeFirst()
{
	QUrl url = first();
	removeUrl(url);
	return url;
}
