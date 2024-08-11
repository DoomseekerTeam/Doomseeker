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
#ifndef WADSEEKER_H
#define WADSEEKER_H

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "dptr.h"
#include "entities/waddownloadinfo.h"
#include "wadseekerexportinfo.h"
#include "wadseekermessagetype.h"

class ModSet;
class ModFile;

/**
 * @mainpage Wadseeker API
 *
 * @section intro Introduction
 * Wadseeker is a library for searching for and automatically downloading a
 * list of mods for Doom (1993).  Wadseeker requires a small subset of the Qt
 * Toolkit, namely the QtCore and QtNetwork modules.  In addition, to interact
 * with Wadseeker, you will need to use Qt to connect to the signals.
 *
 * @section usage Usage
 * The steps required for use are:
 *     -#  Initialize an instance of Wadseeker.
 *     -#  Connect the Qt signals to your slots.
 *     -#  Configure some values such as the sites to search in and target
 *         directory.
 *     -#  Create a list of wads to search for and pass it into Wadseeker::startSeek().
 *
 * Observe the following example on how to do these things.
 * @code
 * Wadseeker wadseeker;
 * // ... connect the signals ...
 * wadseeker.setPrimarySitesToDefault(); // Use the default list of sites
 * wadseeker.setTargetDirectory("./");
 * QStringList wads;
 * wads << "doom2.wad"; // This will be ignored because of isForbiddenWad()
 * wads << "somemod.wad";
 * wads << "somemod.pk3";
 * wadseeker.startSeek(wads);
 * @endcode
 *
 * Wadseeker runs asynchronously. Wadseeker::startSeek() launches the
 * seek session and returns `true` when the seek starts. Then Wadseeker
 * operates within the Qt's event loop: it navigates the Web, parses WWW
 * sites looking for links, downloads the files and archives, installs
 * or unpacks them. When it's done, successful or not, it emits the
 * Wadseeker::allDone() signal.
 *
 * Any Wadseeker options must be set before Wadseeker::startSeek() is
 * called. Once the seek session starts, any changes to the options are
 * ignored for the currently running session.
 *
 * @section is-it-only-for-WADs Is it only for WADs?
 * The documentation often refers to "WADs", and the name of the library is
 * "Wadseeker", but the library is not just limited to WAD files. It can
 * download anything that constitutes a Doom mod. So, wherever you see "WAD",
 * in reality it also means "PK3" or even "DEH". It will also install unpacked
 * "ZIP" or "7z" archives if they are listed by their exact name and extension.
 *
 * @section how-does-it-work How does it work?
 * Wadseeker belongs to a type of tools known as
 * [Web spiders](https://en.wikipedia.org/wiki/Web_crawler)
 * (or Web crawlers). It's specialized towards searching (scraping) websites,
 * looking for links to Doom WADs.
 *
 * When Wadseeker is in this process of scraping the Web, it is called to be in
 * a "seek session".
 *
 * Before a "seek session" can start, Wadseeker must first be told which WADs to
 * look for -- it's not designed to just go and download everything it
 * encounters. Having the list of WADs to get, Wadseeker starts the search. For
 * this purpose, it has an embedded (hardcoded) list of WAD hosting websites it
 * will first go to if not told otherwise. This is where a seek session starts
 * usually.
 *
 * If the link it navigates to goes to a website, it scrapes this website
 * looking for links that may lead to the seeked WADs. When it finds a link it
 * suspects to lead to the WAD download, it navigates to it and checks if this
 * link is another website or a binary file.
 *
 * If the link goes to another website, Wadseeker continues the scraping on
 * that website. It will also follow HTTP redirects. However, there's a limit
 * for such link chains. Wadseeker doesn't let itself be led on a goose chase
 * by prankster Webmasters ;).
 *
 * If the link goes to a binary file, Wadseeker attempts to install it in the
 * specified target directory. Downloaded files whose filenames match the seeked
 * WADs exactly are saved in the directory as-is. Archives are unpacked and
 * scanned for the seeked WADs. Downloads that do not match the seeked WADs are
 * discarded.
 *
 * Wadseeker declares that the "seek session" is completed when it either finds
 * and installs all WADs that were requested (a success), or when it runs out
 * of links to go to and websites to scrape (a failure). It can also be aborted
 * prematurely on demand.
 *
 * Aside from scraping websites, Wadseeker can also contact the API of
 * **the /idgames Archive** and get the WADs from there.
 *
 * It can also happen that Wadseeker's seek session is configured with a custom
 * link going directly to the desired WAD. In such case, the Web scraping is
 * *bypassed*. Wadseeker may still do it, but its results may be discarded, as
 * Wadseeker will immediately go for the direct download.
 *
 * @section limitations Limitations
 *
 * - Wadseeker can scrape the links only from static HTML sites. It's not a Web
 *   browser, it doesn't run Javascript, so it can't handle dynamically
 *   generated websites.
 *
 * - If Wadseeker fails to find the WAD it doesn't mean that it's not available
 *   online. The Web crawling is limited to only the specified list of URLs. If
 *   it runs out of them, it doesn't use the search engines to look for more.
 *
 * - If a WAD is available on a website, but it's located in an archive with a
 *   slightly different name, Wadseeker will miss it. Imagine a case where you
 *   wish to obtain a file called `Mod-Guns.wad`, and this file belongs to a
 *   multi-file mod named "Mod". The author of "Mod" has put all this mod's
 *   files in an archive named "Mod.7z". Wadseeker has no knowledge of the
 *   structure of this particular mod (or any mods, really) and will not know to
 *   look for `Mod-Guns.wad` in `Mod.7z`. This limitation can be circumvented by
 *   looking for both `Mod.wad` (provided that there is `Mod.wad` in `Mod.7z`)
 *   and `Mod-Guns.wad` in a single seek session. Wadseeker will download
 *   `Mod.7z` looking for `Mod.wad` and, by chance, find `Mod-Guns.wad` there
 *   also.
 */

/**
 * @brief Scrape the Web for Doom WADs and install them.
 *
 * The Wadseeker class provides an interface for searching for and
 * downloading modifications for Doom (1993) engine games. Create its
 * object, set it up with various setters methods, and then launch it
 * with startSeek().
 *
 * All setup must be done before startSeek() is called. Wadseeker will
 * ignore any setup changes aftewards.
 *
 * Wadseeker will search for mods in a list of locations provided to
 * setPrimarySites() and setCustomSites(). It can also contact the
 * /idgames Archive, which can be toggled with setIdgamesEnabled().
 *
 * Wadseeker will ignore a predefined set of file names which it suspects to
 * be attempts to illegally download commercial game data, so passing in any
 * name for which isForbiddenWad() returns `true` will result in the file
 * being ignored.
 *
 * It's not advisable, even if technically allowed, to have more than one
 * instance of this class running simultaneously. Some Web servers limit
 * the amount of simultaneous connections per client, and Wadseeker
 * organizes its scraping with that mind, but it can only do that within
 * a single seek session. It can't synchronize between multiple
 * instances of itself.
 */
class WADSEEKER_API Wadseeker : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Default sites to scrape for files.
	 *
	 * @see setPrimarySitesToDefault()
	 */
	static const QString defaultSites[];

	/**
	 * @brief Files that are not meant to be freely downloadable.
	 *
	 * List of file names that will be ignored as they are part of a
	 * commercial product. Empty string at the end of the array
	 * is required and indicates the end of the array.
	 *
	 * @see isForbiddenWad()
	 */
	static const QString forbiddenWads[];

	/**
	 * @brief Default URL to the /idgames Archive.
	 */
	static const QString defaultIdgamesUrl();

	/**
	 * @brief Percent-encoded Wadseeker::defaultSites.
	 *
	 * Runs content of defaultSites array through
	 * QUrl::toPercentEncoding() and returns a list of so
	 * encoded strings.
	 */
	static QStringList defaultSitesListEncoded();

	/**
	 * @brief List only WADs for which isForbiddenWad() returns false.
	 */
	static ModSet filterAllowedOnlyWads(const ModSet &wads);
	/**
	 * @brief List only WADs for which isForbiddenWad() returns true.
	 */
	static ModSet filterForbiddenOnlyWads(const ModSet &wads);

	/**
	 * @brief Check if the file is on the list of forbidden WADs.
	 *
	 * @param wad
	 *     WAD to check.
	 * @return True if Wadseeker will refuse to download this WAD.
	 */
	static bool isForbiddenWad(const ModFile &wad);

	/**
	 * @brief Initializes a new instance of Wadseeker.
	 */
	Wadseeker();

	/**
	 * @brief Deallocates an instance of Wadseeker.
	 */
	~Wadseeker() override;

	/**
	 * @brief Check if Wadseeker is currently downloading given file.
	 *
	 * @param file
	 *      Name of the file as specified by seekStarted() signal.
	 */
	bool isDownloadingFile(const ModFile &file) const;

	/**
	 * @brief Check if Wadseeker is still working.
	 *
	 * @return True if any search or download is still in progress.
	 *         False if all site searches are finished and there are no
	 *         downloads in progress. False will also be returned if
	 *         there are still WADs pending for download URLs but there
	 *         are no more sites to scrape for those URLs.
	 */
	bool isWorking() const;

	/**
	 * @brief Set a high-priority URL to scrape for WADs.
	 *
	 * This site has priority over all other sites and will be searched
	 * first. A direct URL to the file can also be used.
	 *
	 * This is a good place to pass in the link provided by the game
	 * server.
	 *
	 * @param url
	 *     A valid, absolute URL.
	 * @see setCustomSites() for multiple sites
	 */
	void setCustomSite(const QString &url);

	/**
	 * @brief setCustomSite(), but more than one.
	 *
	 * Because this is a "set", it overwrites any previous custom sites,
	 * including the one set by setCustomSite().
	 *
	 * @param urls A list of valid, absolute URLs.
	 */
	void setCustomSites(const QStringList &urls);

	/**
	 * @brief Set the /idgames Archive enabled.
	 *
	 * If true, /idgames Archive search will be performed.
	 * If false, the /idgames Archive will be skipped.
	 *
	 * The /idgames Archive is enabled by default.
	 */
	void setIdgamesEnabled(bool bEnabled);

	/**
	 * @brief Set the URL to the /idgames Archive search page.
	 *
	 * Wadseeker already hardcodes the URL to the /idgames Archive and
	 * uses it by default, so in normal operation there should be no
	 * need to use this method.
	 */
	void setIdgamesUrl(QString archiveURL);

	/**
	 * @brief Set maximum number of concurrent WAD downloads.
	 *
	 * Default value: 3.
	 *
	 * @param max
	 *      Number of max. concurrent downloads. Cannot be lesser than 1.
	 */
	void setMaximumConcurrentDownloads(unsigned max);

	/**
	 * @brief Set maximum number of concurrent site seeks (scrapings).
	 *
	 * Default value: 3.
	 *
	 * @param max
	 *      Number of max. concurrent seeks. Cannot be less than 1.
	 */
	void setMaximumConcurrentSeeks(unsigned max);

	/**
	 * @brief Set a list of primary sites to scrape for WADs.
	 *
	 * This is where Wadseeker begins its search.
	 *
	 * @param urlList
	 *      List of valid, absolute URLs.
	 */
	void setPrimarySites(const QStringList &urlList);

	/**
	 * @brief Reset the list of primary sites to the sites hardcoded
	 * into Wadseeker.
	 *
	 * Calling this is normally not needed, because Wadseeker already
	 * initializes the primary sites to this default list. However, if
	 * you changed the list and want to go back to the default one, this
	 * is the way to do it.
	 *
	 * @see defaultSites
	 * @see setPrimarySites().
	 */
	void setPrimarySitesToDefault();

	/**
	 * @brief Set the WAD installation target directory.
	 *
	 * The target directory is the directory where all the seeked files
	 * will be saved to. If this directory is not writable, the seek
	 * will fail.
	 *
	 * @param dir
	 *     Path to a writable directory.
	 */
	void setTargetDirectory(const QString &dir);

	/**
	 * @brief Does nothing, because Wad Archive is defunct.
	 *
	 * If set to true, Wadseeker would contact Wad Archive to
	 * look for WAD URLs, but the site is dead.
	 *
	 * Wad Archive: http://www.wad-archive.com
	 *
	 * @deprecated Wad Archive was shut down in October 2022.
	 * This doesn't do anything anymore.
	 */
	void setWadArchiveEnabled(bool enabled);

	/**
	 * @brief Abort the current URL for the specified file.
	 *
	 * The current download will be aborted, but a new file URL will be
	 * immediately taken from the queue if available, and a new download
	 * will begin for this file.
	 *
	 * This method gives the user the option to abort the current
	 * download for this file if the site is slow.
	 *
	 * @param fileName
	 *      This must be a file name as previously sent by seekStarted()
	 *      signal. If the file name is not valid or download for this
	 *      file is not in progress, then no operation will be performed.
	 */
	void skipFileCurrentUrl(const QString &fileName);

	/**
	 * @brief Abort querying a named service if such query is in progress.
	 *
	 * The name of the service is previously emitted by serviceStarted()
	 * signal.
	 *
	 * @param service
	 *     Name of the service as emitted by serviceStarted() signal.
	 */
	void skipService(const QString &service);

	/**
	 * @brief Abort scraping the site at the specified URL.
	 *
	 * Wadseeker will immediately start scraping the next site in the queue.
	 *
	 * @param url
	 *     This must be one of the URLs that was emitted through
	 *     siteStarted() or siteRedirect() signals. If an unknown URL
	 *     is passed, no operation will be performed.
	 */
	void skipSiteSeek(const QUrl &url);

	/**
	 * @brief Launch the WAD retrieval session.
	 *
	 * Setup the seek first using other methods, then call this method
	 * to start it. This is where Wadseeker begins to iterate through
	 * WWW sites to find all the desired files.
	 *
	 * One of the three things may happen after here:
	 *
	 * - If seek starts properly, seekStarted() signal is emitted and
	 *   the method returns `true`.
	 *
	 * - If seek is about to start, but an error occurs, allDone()
	 *   signal is emitted with a `false` result, but `true` is still
	 *   returned from here.
	 *
	 * - If seek cannot start, `false` is returned and no signals are
	 *   emitted.
	 *
	 * The list of files emitted in seekStarted() may be different than
	 * the list of files passed to startSeek() in case of duplicates or
	 * if any of the files was on the isForbiddenWad() list.
	 *
	 * @param wads
	 *     List of files that will be searched for.
	 * @return True if seek was started. False otherwise.
	 */
	bool startSeek(const ModSet &wads);

	/**
	 * @brief Target directory is a directory where all seeked files
	 * will be saved.
	 *
	 * If setTargetDirectory() is called after a seek session is started
	 * with startSeek(), this method will return the newly set
	 * directory, not the one from the running session.
	 *
	 * @return Path to a directory last set by setTargetDirectory().
	 */
	QString targetDirectory() const;

public slots:
	/**
	 * @brief Tell Wadseeker to stop the seek session.
	 *
	 * This is asynchronous. Wadseeker doesn't stop outright. It takes
	 * care to clean up after itself. Once everything is stopped,
	 * allDone() with `false` result is emitted.
	 */
	void abort();

signals:
	/**
	 * @brief Emitted when Wadseeker finishes iterating through all
	 * files passed to startSeek().
	 *
	 * @param bSuccess
	 *      True if all seeked WADs were installed. False if any single
	 *      one of the WADs was not found or if abort was issued.
	 */
	void allDone(bool bSuccess);

	/**
	 * @brief Emitted when a particular file finishes downloading.
	 *
	 * **NOTE:** This doesn't mean that the WAD was successfully installed.
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
	 * The progress values may be used to update a progress bar.
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
	 * @brief Emits Wadseeker's log messages.
	 *
	 * @param msg
	 *     Text of the message.
	 * @param type
	 *     Log severity level.
	 */
	void message(const QString &msg, WadseekerLib::MessageType type);

	/**
	 * @brief Emitted when Wadseeker begins the seek operation, upon
	 * a successful startSeek().
	 *
	 * @param filenames
	 *     Contains unique names of all files that will be seeked.
	 *     Wadseeker will continue to refer to those filenames in other
	 *     signals.
	 */
	void seekStarted(const ModSet &filenames);

	/**
	 * @brief Notifies that Wadseeker started querying a named service.
	 *
	 * Currently there is only one such service: `idgames`.
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
	 * @brief Notifies that Wadseeker is done querying a named service.
	 *
	 * @param name
	 *     Unique name of the service.
	 *
	 * @see serviceStarted()
	 */
	void serviceFinished(const QString &name);

	/**
	 * @brief Emitted when a WWW site finishes downloading.
	 */
	void siteFinished(const QUrl &site);

	/**
	 * @brief Emits download progress of a WWW site.
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
};

#endif
