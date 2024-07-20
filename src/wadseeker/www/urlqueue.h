//------------------------------------------------------------------------------
// urlqueue.h
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
#ifndef WADSEEKER_WWW_URLQUEUE_H
#define WADSEEKER_WWW_URLQUEUE_H

#include "www/urlmirror.h"

#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QUrl>

/**
 * @brief Keep UrlMirror and allow to get URLs one-by-one.
 *
 * This is a priority queue because:
 *
 * - a) this class is designed to remember all URLs for its lifetime and never
 *      allow any duplicates,
 * - b) the URLs are taken out in FIFO order with an optional priority.
 *
 * The priority is in reverse: lower number is higher priority.
 *
 * Each URL that was added to the queue will not be allowed to appear on
 * this queue again, even if one of the remove methods were used.
 *
 * The URLs are organized by resource, because that's what Wadseeker is
 * ultimately interested in. A resource is just a name, so Wadseeker has
 * the flexibility of putting various items here. Resource can also be
 * unnamed -- use an empty string for a resource name in such case.
 *
 * All URLs are ultimately organized into UrlMirror structures. Mirror
 * URLs are supported in a way that allows URLs to be associated with
 * each other as pointing to the same remote resource, only located at a
 * different site. User may decide to remove the URL and all its mirrors
 * at once, for example when they decide that the mirrors do not contain
 * the requested file.
 *
 * Organizing URLs into mirrors is explicit and not directly related to
 * organizing the URLs by resource. While all URLs in a mirror must
 * point to the same resource, one resource may have many URLs that are
 * not recognized as mirrors. This is done this way because:
 *
 * - a) URLs to unnamed resources can be *anything*, e.g. URLs to unrelated
 *      websites, so they're not really a mirrors by definition,
 * - b) different files (WADs) may have the same name, so if two URLs are
 *      believed to point to a resource **"example.wad"**, they may not
 *      necessarilly point to the **same** **"example.wad"**.
 */
class UrlQueue
{
public:
	struct PrioUrl
	{
		int priority;
		QUrl url;
	};

	static const QString UNNAMED_RESOURCE;

	UrlQueue();
	~UrlQueue();

	/**
	 * @brief Add a previously created UrlMirror to the list.
	 *
	 * URLs that have already been used in the queue will be pruned
	 * from the mirror list. If the list becomes empty afterwards, it's
	 * not added.
	 *
	 * @param mirror The mirror list to add; it's already expected
	 *     to have a resource name.
	 * @param priority Importance of this mirror (lower number is higher priority).
	 */
	void add(UrlMirror mirror, int priority = 0);

	/**
	 * @brief Add all URLs on the list and treats them as mirrors
	 *        to the same resource.
	 *
	 * URLs that have already been used in the queue will be pruned
	 * from the mirror list. If the list becomes empty afterwards, it's
	 * not added.
	 *
	 * @param urls The mirror list to add.
	 * @param resource Name of the resource this mirror is supposed to point to.
	 * @param priority Importance of this mirror (lower number is higher priority).
	 */
	void addMirrorUrls(const QList<QUrl> &urls,
		const QString &resource = UNNAMED_RESOURCE,
		int priority = 0);

	/**
	 * @brief Add a single URL to the list, no mirrors.
	 *
	 * URL may not be actually added if it was already known to
	 * the queue before.
	 *
	 * @param url To add.
	 * @param resource Name of the resource this URL is supposed to point to.
	 * @param priority Importance of this URL (lower number is higher priority).
	 */
	void addUrl(const QUrl &url,
		const QString &resource = UNNAMED_RESOURCE,
		int priority = 0);

	/**
	 * @brief Get a list of URLs available for given resource without
	 * taking them out.
	 *
	 * The URLs remain available. The URLs are returned in the order
	 * of their priority and FIFO. Mark spent URLs with takenOut().
	 */
	QList<PrioUrl> peek(const QString &resource = UNNAMED_RESOURCE);

	/**
	 * @brief Checks if URL was known to the object during its entire
	 *        lifetime.
	 *
	 * This URL might've already been taken out and will not appear
	 * anymore on the available URLs list. Such URLs will return false
	 * on hasUrl().
	 */
	bool isUrlKnown(const QUrl &url) const;

	/**
	 * @brief Checks if URL is put on the queue but has not been taken out, yet.
	 */
	bool hasUrl(const QUrl &url) const;

	/**
	 * @brief True if there are no available URLs.
	 */
	bool isEmpty() const;

	/**
	 * @brief Removes single URL from the available URLs list.
	 *
	 * This will not make the object forget the URL. isUrlKnown() for this
	 * URL will still return true and re-adding the same URL will not be
	 * possible. Moreover, if URL was not previously known to the queue,
	 * it will now be known as already taken out.
	 */
	void removeUrl(const QUrl &url);

	/**
	 * @brief Removes URL and all its mirrors from the available URLs list.
	 *
	 * @see removeUrl().
	 */
	void removeUrlAndMirrors(const QUrl &url);

	/**
	 * @brief Number of currently available URLs.
	 */
	unsigned size() const;

	/**
	 * @brief Set the URL as taken out; UrlQueue won't produce it anymore.
	 */
	void takenOut(const QUrl &url);

private:
	// Can't be a QMultiMap because QMultiMap provides
	// its elements in a LIFO stack instead of a FIFO queue. This
	// results in idgames providing last (worst) mirrors first,
	// for example.
	QMap<int, QList<UrlMirror> > mirrors;
	QSet<QUrl> knownUrls;
	QSet<QUrl> takenOutUrls;

	/**
	 * Compact the internal storage of the queue.
	 */
	void compact();

	QList<UrlMirror> mirrorsWithUrl(const QUrl &url);
	void removeAllUrls(const UrlMirror &mirror);
};

#endif
