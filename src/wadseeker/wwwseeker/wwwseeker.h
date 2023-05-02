//------------------------------------------------------------------------------
// wwwseeker.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __WWWSEEKER_H__
#define __WWWSEEKER_H__

#include "wadseekermessagetype.h"

#include <QHash>
#include <QList>
#include <QMultiMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QUrl>

class FileSeekInfo;
class NetworkReplySignalWrapper;
class NetworkReply;

/**
 *	@brief Search and protocol wrapper class.
 *
 *	WWWSeeker class seeks specified files by iterating through
 *	previously defined set of sites and parsing their HTML code
 *	if necessary.
 *
 *	This class handles URLs by selecting proper protocols for them.
 *	It hides how protocols work and provides simple interface:
 *	request a file, inform about download progress and
 *	send a proper signal when download is finished.
 *
 *	URLs with wildcards can be used, see: primaryFile
 */
class WWWSeeker : public QObject
{
	Q_OBJECT

public:
	WWWSeeker();
	~WWWSeeker() override;

	/**
	 * @brief Adds a URL to a site where a specified filename may reside
	 *        with default priority.
	 *
	 * Such site will be searched in a different order than global site
	 * URLs. Since it is suspected that these sites may contain specified
	 * files, they will have the priority.
	 *
	 * Naturally, such sites are also searched for all other seeked
	 * filenames, not the only specified one.
	 *
	 * If specified URL is on visited URLs list this becomes a no-op.
	 *
	 * Default priority is 0.
	 */
	void addFileSiteUrl(const QString &filename, const QUrl &url);

	/**
	 * @brief Adds a URL to a site where a specified filename may reside.
	 *
	 * Such site will be searched in a different order than global site
	 * URLs. Since it is suspected that these sites may contain specified
	 * files, they will have the priority.
	 *
	 * Naturally, such sites are also searched for all other seeked
	 * filenames, not the only specified one.
	 *
	 * If specified URL is on visited URLs list this becomes a no-op.
	 *
	 * @param priority
	 *     The higher the number, the bigger the URL priority.
	 */
	void addFileSiteUrlWithPriority(const QString &filename, const QUrl &url, int priority);

	/**
	 * @brief Adds a URL to a site that will be used in the search.
	 *
	 * URLs that were already used by this WWWSeeker object are rejected.
	 */
	void addSiteUrl(const QUrl &url);

	/**
	 * @brief Convenience method. Calls addSiteUrl() for each URL on The
	 * list.
	 *
	 * URLs that were already used by this WWWSeeker object are rejected.
	 */
	void addSitesUrls(const QList<QUrl> &urlsList);

	/**
	 * @brief Clears visited URLs list.
	 *
	 * This allows URLs to be reused by the current WWWSeeker object.
	 * Such URLs however need to be readded by using addSiteUrl() method.
	 */
	void clearVisitedUrlsList()
	{
		d.visitedUrls.clear();
	}

	/**
	 * @brief Checks if WWWSeeker is processing any data.
	 */
	bool isWorking() const
	{
		return d.bIsWorking;
	}

	int maxConcurrentSiteDownloads() const
	{
		return d.maxConcurrentSiteDownloads;
	}

	/**
	 * @brief Removes seeked file from the current search.
	 *
	 * This file will no longer be seeked on the downloaded sites.
	 *
	 * All sites that are suspected to refer to this file will also
	 * be dropped.
	 *
	 * If all files are removed this is equal to abort().
	 *
	 * @param Case-insensitive name of the file to be removed.
	 * Only exact matches are dropped.
	 */
	void removeSeekedFile(const QString &file);

	void setCustomSiteUrl(const QUrl &url)
	{
		d.customSiteUrl = url;
	}

	/**
	 * @brief Maximum amount of URLs the WWWSeeker will go through
	 * at the same time.
	 */
	void setMaxConcurrectSiteDownloads(int value)
	{
		d.maxConcurrentSiteDownloads = value;
	}

	void setUserAgent(const QString &userAgent);

	/**
	 * @brief Skips site seeks for specified URL.
	 *
	 * If available a new site URL will be immediately taken from the queue
	 * for inspection.
	 */
	void skipSite(const QUrl &url);

	/**
	 * @brief Begins search for files.
	 *
	 * Search is performed by downloading sites specified by sitesUrls
	 * list and seeking out links on these sites that either directly
	 * point to one of the specified filenames or may lead to subsites
	 * that may contain links to these files.
	 *
	 * WWWSeeker will look for all filenames specified on the list.
	 *
	 * This will do nothing if isWorking() returns true.
	 */
	void startSearch(const QList<FileSeekInfo> &seekedFiles);

	const QString &userAgent() const;

public slots:
	/**
	 * @brief Aborts the current search operation.
	 *
	 * finished() signal is emitted once the abort completes.
	 */
	void abort();

signals:
	/**
	 * @brief Emitted when seeker aborts or there is nothing more to search.
	 */
	void finished();

	/**
	 * @brief Emitted for each direct link to a file that is found.
	 *
	 * @param filename
	 *      Filename to which the URL refers to.
	 * @param url
	 *      URL under which the file resides.
	 */
	void linkFound(const QString &filename, const QUrl &url);

	void message(const QString &message, WadseekerLib::MessageType type);

	/**
	 * @brief Emitted when a WWW site finishes download.
	 */
	void siteFinished(const QUrl &site);

	/**
	 * @brief Emitted when a WWW site redirects to a different site.
	 */
	void siteRedirect(const QUrl &oldUrl, const QUrl &newUrl);

	/**
	 * @brief Emitted when a WWW site is being downloaded.
	 */
	void siteProgress(const QUrl &site, qint64 bytes, qint64 total);

	/**
	 * @brief Emitted when a download of a WWW site starts.
	 */
	void siteStarted(const QUrl &site);

private:
	class PrivData
	{
	public:
		bool bIsAborting;
		bool bIsWorking;

		QUrl customSiteUrl;

		/**
		 * @brief URLs to sites where specified files may reside.
		 *
		 * Key - name of the file.
		 * Value - URLs to the site.
		 */
		QHash<QString, QMultiMap<int, QUrl> > fileSiteUrls;

		/**
		 * @brief Used to rotate over files on the fileSiteUrls
		 * hash map.
		 */
		QStringList fileSiteKeyRotationList;

		/**
		 * @brief Default value: 3
		 */
		int maxConcurrentSiteDownloads;

		/**
		 * @brief Currently running network queries.
		 *
		 * Amount here will not go above the maxConcurrentSiteDownloads.
		 */
		QList<NetworkReply *> networkQueries;

		QNetworkAccessManager *pNetworkAccessManager;

		QList<FileSeekInfo> seekedFiles;

		/**
		 * @brief URLs that will be used in a search.
		 */
		QList<QUrl> sitesUrls;

		/**
		 * @brief Holds list of all used URLs.
		 *
		 * Prevents checking the same site twice.
		 */
		QList<QUrl> visitedUrls;

		/**
		 * @brief User Agent used in WWW queries.
		 */
		QString userAgent;
	};

	PrivData d;

	void addNetworkReply(const QNetworkRequest &request, QNetworkReply *pReply);
	void deleteNetworkReply(NetworkReply *pReply);
	NetworkReply *findNetworkReply(QNetworkReply *pReply);
	NetworkReply *findNetworkReply(const QUrl &url);

	/**
	 * @brief Find FileSeekInfo object by comparing possible filenames.
	 */
	FileSeekInfo *findFileSeekInfo(const QString &seekedName);

	/**
	 * @brief Detects if URL leads to one of the requested files.
	 *
	 * @param url
	 *      URL to check.
	 * @param [out] outFileName
	 *      Name of the file that this URL affects.
	 *
	 * @return True if URL is a direct URL to one of the seeked files.
	 */
	bool isDirectUrl(const QUrl &url, QString &outFileName) const;

	bool isMoreToSearch() const;

	void logHeaders(NetworkReply *reply);

	void parseAsHtml(NetworkReply *pReply);

	/**
	 * @brief Starts network query using specified URL.
	 *
	 * No limitation checks are performed here.
	 */
	void startNetworkQuery(const QUrl &url);

	/**
	 * @brief Takes next available URLs and starts queries to their sites.
	 *
	 * Limitation checks are performed here to make sure that given URL
	 * will not be visited twice and that not too many queries
	 * are running at once.
	 */
	void startNextSites();

	/**
	 * @brief Takes next non-visited site URL from the list and returns it.
	 *
	 * Taken URL is removed from the URLs list.
	 */
	QUrl takeNextUrl();
	bool wasUrlUsed(const QUrl &url) const;

private slots:
	void networkQueryDownloadProgress(NetworkReply *pReply, qint64 current, qint64 total);
	void networkQueryError(NetworkReply *pReply, QNetworkReply::NetworkError code);
	void networkQueryFinished(NetworkReply *pReply);
	void networkQueryMetaDataChanged(NetworkReply *pReply);
};

#endif
