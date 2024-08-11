//------------------------------------------------------------------------------
// webcrawl.h
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
#ifndef WADSEEKER_WWW_WEBCRAWL_H
#define WADSEEKER_WWW_WEBCRAWL_H

#include <QList>
#include <QSharedPointer>
#include <QUrl>

class WWW;
class WebQuery;

/**
 * @brief Keep the history of Web crawling.
 */
class WebCrawl
{
public:
	static const QString UNNAMED_RESOURCE;

	WebCrawl(const QUrl &url,
		const QString &resource = UNNAMED_RESOURCE,
		int priority = 0,
		QList<QUrl> history = QList<QUrl>());

	void addUrl(const QUrl &url) { m_history << url; }
	const QList<QUrl> &history() const { return m_history; }
	unsigned historySize() const { return m_history.size(); }
	int priority() const { return m_priority; }
	const QString &resource() const { return m_resource; }
	QUrl url() const { return m_history.last(); }

private:
	QString m_resource;
	int m_priority;
	QList<QUrl> m_history;
};

class WebCrawlList : public QList<WebCrawl>
{
public:
	WebCrawlList::iterator findByUrl(const QUrl &url)
	{
		auto it = this->begin();
		for (; it != this->end() && it->url() != url; ++it);
		return it;
	}
};

#endif
