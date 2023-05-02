//------------------------------------------------------------------------------
// wadseeker.h
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
#ifndef __WADSEEKER_H_
#define __WADSEEKER_H_

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "dptr.h"
#include "entities/waddownloadinfo.h"
#include "wadseekerexportinfo.h"
#include "wadseekermessagetype.h"

#define WADSEEKER_CONNECT_TIMEOUT_SECONDS_DEFAULT 30
#define WADSEEKER_DOWNLOAD_TIMEOUT_SECONDS_DEFAULT 120

class Idgames;
class WadArchiveClient;
class WadRetriever;
class WWWSeeker;
class ModSet;
class ModFile;

/**
 * @mainpage Wadseeker API
 *
 * @section intro Introduction
 * Wadseeker is a library for searching for an automatically downloading a
 * list of mods for Doom.  Wadseeker requires a small subset of the Qt Toolkit,
 * namely the QtCore and QtNetwork modules.  In addition, to interact with
 * Wadseeker, you will need to use Qt to connect to the signals.
 */

/**
 * @brief Main controlling class.
 *
 * The Wadseeker class provides an interface for searching for and downloading
 * modifications for Doom engine games.  Wadseeker will search for mods in a
 * list of locations provided by setPrimarySites.
 *
 * Wadseeker will ignore a predefined set of file names which it suspects to
 * be attempts to illegally download commercial game data, so passing in any
 * name which is found in iwadNames will result in the file being ignored.
 *
 * @section usage Usage
 * Using Wadseeker is quite simple, there are three steps required for use:
 *     -#  Initialize an instance of Wadseeker
 *     -#  Configure some values such as the sites to search in and target
 *         directory.
 *     -#  Create a list of wads to search for and pass it into seekWads.
 *
 * Observe the following example on how to do these things.
 * @code
 * Wadseeker ws;
 * ws.setPrimarySitesToDefault(); // Use the default list of sites
 * ws.setTargetDirectory("./");
 * QStringList listOfWads;
 * listOfWads << "doom2.wad"; // This will be ignored because of iwadNames
 * listOfWads << "somemod.wad";
 * listOfWads << "somemod.pk3";
 * ws.seekWads(listOfWads);
 * @endcode
 */
class WADSEEKER_API Wadseeker : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief List of default sites to search for files.
	 *
	 * @see setPrimarySitesToDefault
	 */
	static const QString defaultSites[];

	/**
	 * List of file names that will be ignored as they are part of a
	 * commercial product. Empty string at the end of the array
	 * is required and indicates the end of the array.
	 *
	 * @see isForbiddenWad
	 */
	static const QString forbiddenWads[];

	/**
	 * @return Default URL of Idgames archive, hardcoded into the library.
	 */
	static const QString defaultIdgamesUrl();

	/**
	 * Runs content of defaultSites array through
	 * QUrl::toPercentEncoding() and returns a list of so
	 * encoded strings.
	 */
	static QStringList defaultSitesListEncoded();

	/**
	 * @brief Lists only wads for which isForbiddenWad() returns false.
	 */
	static ModSet filterAllowedOnlyWads(const ModSet &wads);
	/**
	 * @brief Lists only wads for which isForbiddenWad() returns true.
	 */
	static ModSet filterForbiddenOnlyWads(const ModSet &wads);

	/**
	 * @brief Attempts to detect if the input is on
	 *        a list of forbidden wads.
	 *
	 * @param wad
	 *     wad trying to be searched for.
	 * @return True if Wadseeker will refuse to download the wad specified.
	 * @see iwadNames
	 */
	static bool isForbiddenWad(const ModFile &wad);

	/**
	 * Initializes a new instance of Wadseeker.
	 */
	Wadseeker();

	/**
	 * Deallocates an instance of Wadseeker.
	 */
	~Wadseeker() override;

	/**
	 * @brief Check if Wadseeker is currently downloading given file.
	 *
	 * @param wad
	 *      Name of the file as specified by seekStarted() signal.
	 */
	bool isDownloadingFile(const ModFile &file) const;

	/**
	 * @brief Check if Wadseeker is still working.
	 *
	 * @return True if any search or download is still in progress.
	 *         False if all site searches are finished and there are no
	 *         downloads in progress - in such case false will also be
	 *         returned if there are still WADs pending for URLs but there
	 *         is no way for these URLs to be provided.
	 */
	bool isWorking() const;

	/**
	 * Sets a custom site. This site has priority over all other
	 * sites and will be searched first. For example a link
	 * provided by the server can be passed here.
	 *
	 * Custom site can be set before the seek is started.
	 *
	 * @param url
	 *     A valid, absolute URL.
	 */
	void setCustomSite(const QString &url);

	/**
	 * @brief Toggles Idgames enabled state.
	 *
	 * If true, Idgames search will be performed.
	 * If false, Idgames archive will be skipped.
	 */
	void setIdgamesEnabled(bool bEnabled);

	/**
	 * @brief Sets URL to Idgames search page.
	 */
	void setIdgamesUrl(QString archiveURL);

	/**
	 * @brief Sets maximum number of concurrent WAD downloads.
	 *
	 * Default value: 3
	 *
	 * @param max
	 *      Number of max. concurrent downloads. Cannot be lesser than one.
	 */
	void setMaximumConcurrentDownloads(unsigned max);

	/**
	 * @brief Sets maximum number of concurrent site seeks.
	 *
	 * Default value: 3
	 *
	 * @param max
	 *      Number of max. concurrent seeks. Cannot be lesser than one.
	 */
	void setMaximumConcurrentSeeks(unsigned max);

	/**
	 * Sets a list of primary sites. This is where Wadseeker begins
	 * it's search and where it returns to if nothing of interest
	 * was found on other pages.
	 *
	 * Primary sites must be set before the seek is started.
	 *
	 * @param urlList
	 *      List of valid, absolute URLs
	 */
	void setPrimarySites(const QStringList &urlList);

	/**
	 * Convenience method. Sets list of primary sites to sites
	 * hardcoded into library itself.
	 *
	 * @see defaultSites
	 */
	void setPrimarySitesToDefault();

	/**
	 * Target directory is a directory where all seeked files will
	 * be saved. If this directory is not writable the seek will fail.
	 *
	 * Must be set before the seek is started.
	 *
	 * @param dir
	 *     Path to a writable directory.
	 */
	void setTargetDirectory(const QString &dir);

	/**
	 * @brief If set to true, Wadseeker will contact
	 * http://www.wad-archive.com to look for download URLs.
	 */
	void setWadArchiveEnabled(bool enabled);

	/**
	 * @brief Skips current URL for the specified file.
	 *
	 * If available a new file URL will be immediately taken from the queue
	 * and new download will begin.
	 *
	 * @param fileName
	 *      This must be a file name specified by seekStarted()
	 *      signal. If file name is not valid or download for this
	 *      file is not in progress then no operation will be
	 *      performed.
	 */
	void skipFileCurrentUrl(const QString &fileName);

	/**
	 * @brief Aborts querying a service if such query is in progress.
	 *
	 * @param service
	 *     Name of the service as emitted by serviceStarted() signal.
	 */
	void skipService(const QString &service);

	/**
	 * @brief Skips site seeks for specified URL.
	 *
	 * If available a new site URL will be immediately taken from the queue
	 * for inspection.
	 *
	 * @param url
	 *     This must be one of the URLs that was emitted through
	 *     siteStarted() or siteRedirect() signals. If unknown URL
	 *     is passed no operation will be performed.
	 */
	void skipSiteSeek(const QUrl &url);

	/**
	 * Library's "entry" method. This is where Wadseeker begins
	 * to iterate through WWW sites to find all files specified
	 * on the list. seekStarted() signal is emitted.
	 *
	 * @param wads
	 *     List of files that will be searched for.
	 * @return True if seek was started. False otherwise.
	 */
	bool startSeek(const ModSet &wads);

	/**
	 * Target directory is a directory where all seeked files will
	 * be saved.
	 *
	 * @return Path to a directory last set by setTargetDirectory().
	 */
	QString targetDirectory() const;

public slots:
	/**
	 * Issues abort commands throughout entire library. aborted() signal
	 * is emitted when Wadseeker finishes aborting.
	 */
	void abort();

signals:
	/**
	 * Emitted when Wadseeker finishes iterating through all
	 * files passed to seekWads() method.
	 *
	 * @param bSuccess
	 *      True if all seeked WADs were installed. False if any single
	 *      one of the WADs was not found or if abort was issued.
	 */
	void allDone(bool bSuccess);

	/**
	 * @brief Emitted when a particular file finishes downloading.
	 *
	 * @b NOTE: This doesn't mean that the WAD was successfully installed.
	 * It only serves as a notification that a download has been
	 * completed.
	 *
	 * @param filename
	 *      Unique filename for the affected file.
	 *      Emitted previously by seekStarted() signal as an entry on the
	 *      filenames list.
	 */
	void fileDownloadFinished(const ModFile &filename);

	/**
	 * @brief Emits download progress.
	 *
	 * Programmer may use this for example to update
	 * a progress bar.
	 *
	 * @param filename
	 *     Unique filename for the affected file.
	 *     Emitted previously by seekStarted() signal as an entry on the
	 *     filenames list.
	 * @param done
	 *     Bytes already downloaded.
	 * @param total
	 *     Total size of the downloaded file.
	 */
	void fileDownloadProgress(const ModFile &filename, qint64 done, qint64 total);

	/**
	 * @brief Emitted when a file download starts.
	 *
	 * @param filename
	 *     Name of the downloaded file.
	 * @param url
	 *     URL from which the file is being downloaded.
	 */
	void fileDownloadStarted(const ModFile &filename, const QUrl &url);

	/**
	 * @brief Emitted when a particular file is installed correctly.
	 *
	 * @param filename
	 *      Unique filename for the affected file.
	 *      Emitted previously by seekStarted() signal as an entry on the
	 *      filenames list.
	 */
	void fileInstalled(const ModFile &filename);

	/**
	 * @brief Emitted when Wadseeker wants to communicate about its
	 *        progress with outside world.
	 *
	 * @param msg - content of the message
	 * @param type - See: Wadseeker::MessageType
	 */
	void message(const QString &msg, WadseekerLib::MessageType type);

	/**
	 * @brief Emitted when Wadseeker begins the seek operation.
	 *
	 * @param filenames
	 *     Contains unique names of all files that will be seeked.
	 *     Wadseeker will continue to refer to those filenames in other
	 *     signals.
	 */
	void seekStarted(const ModSet &filenames);

	/**
	 * @brief Notifies that Wadseeker started querying a
	 * customized service.
	 *
	 * Customized services are: WadArchive, idgames.
	 *
	 * @param name
	 *     Unique name of the service. Can be used to abort
	 *     querying of this service by passing it to
	 *     skipService().
	 *
	 * @see serviceFinished()
	 */
	void serviceStarted(const QString &name);

	/**
	 * @brief Notifies that Wadseeker is done querying a
	 * customized service.
	 *
	 * @param name
	 *     Unique name of the service.
	 *
	 * @see serviceStarted()
	 */
	void serviceFinished(const QString &name);

	/**
	 * @brief Emitted when a WWW site finishes download.
	 */
	void siteFinished(const QUrl &site);

	/**
	 * @brief Emitted when a WWW site is being downloaded.
	 */
	void siteProgress(const QUrl &site, qint64 bytes, qint64 total);

	/**
	 * @brief Emitted when a WWW site redirects to a different site.
	 */
	void siteRedirect(const QUrl &oldUrl, const QUrl &newUrl);

	/**
	 * @brief Emitted when a download of a WWW site starts.
	 */
	void siteStarted(const QUrl &site);

private:
	DPtr<Wadseeker> d;

	void abortSeekers();
	void abortWwwSeeker();
	void cleanUpAfterFinish();
	bool isAllFinished() const;

	/**
	 * @brief Spawns WWWSeeker and WadRetriever instances.
	 */
	void prepareSeekObjects();

	void setupIdgamesClients(const QList<WadDownloadInfo> &wadDownloadInfoList);
	void setupSitesUrls();
	void setupWadArchiveClient(const QList<WadDownloadInfo> &wadDownloadInfos);

	void startNextIdgamesClient();
	void startIdgames();
	void stopIdgames();
	void startWadArchiveClient();
	void stopWadArchiveClient();

private slots:
	void cleanUpIfAllFinished();
	void fileLinkFound(const QString &filename, const QUrl &url);
	void fileMirrorLinksFound(const QString &filename, const QList<QUrl> &urls);
	void idgamesClientFinished(Idgames *pEmitter);
	void reportBadUrl(const QUrl &url);
	void wadArchiveFinished();
	void wadRetrieverDownloadFinished(WadDownloadInfo wadDownloadInfo);
	void wadRetrieverDownloadProgress(WadDownloadInfo wadDownloadInfo, qint64 current, qint64 total);
	void wadRetrieverDownloadStarted(WadDownloadInfo wadDownloadInfo, const QUrl &url);
	void wadRetrieverFinished();
	void wadRetrieverMessage(const QString &message, WadseekerLib::MessageType type);
	void wadRetrieverWadInstalled(WadDownloadInfo wadDownloadInfo);
	void wwwSeekerFinished();
};

#endif
