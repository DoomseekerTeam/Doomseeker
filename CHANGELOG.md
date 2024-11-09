# Changelog

Visit this file cleanly formatted in repository:
<https://bitbucket.org/Doomseeker/doomseeker/src/master/CHANGELOG.md>

This file follows the format of
[Keep a Changelog 1.0.0](https://keepachangelog.com/en/1.0.0/),
and this project does not adhere to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]
### Fixed
- Use the proper demo file filter when browsing for demo files,
  not the executable file filter. (#4435)
- Demos paths handling in `--portable` mode. (#4437)
- Fix misaligned layout for "Map" field in the "Create Game" box. (#4438)

## [1.5.0]
### Added
- Demo playback and recording features (#4110):
    - Demo playback can now be setup in the usual "Create Game" dialog box.
    - Demos can be exported with Doomseeker's metadata.
    - Import external demos to Doomseeker's demo manager. The import/export
      features facilitate demo sharing.
    - Save the demo's created time in the metadata file with timezone info.
    - Also save the demo's game as plugin's canonical name in the metadata file.
    - The recorded demos will now have their author's name saved in their
      metadata. Your name can be configured in Doomseeker's config box in the
      new "Games" page (the tree section for "Games" was there since forever
      already, but now it finally has an actual page).
- Server filtering by the IP address. (#4117)
- A compilation option to make the IP2C auto-updates disabled by default:
`DOOMSEEKER_IP2C_AUTOUPDATE_DEFAULT`. (#3732)
- El Salvador and Macao country flags.
- Plugin API: `TooltipRenderHint` and `TooltipGenerator::renderHint()`.
- Wadseeker: `Navigation` message type, to inform about visited URLs.
- Zandronum: query the game servers with a segmented challenge, where
the servers can reply with more than one packet. This should help with
servers that need to send a lot of data in their replies and this data
would get lost on the network due to oversized packets. This feature is
enabled by default, but can be disabled from the Zandronum config box. (#4142)

### Changed
- Doomseeker won't request from the game servers the checksums of loaded
files if the file integrity validation is turned off. This relieves some
network traffic when refreshing the servers. (#4130)
- Create game: allow paths to missing files in the files/WADs box; show
which files are missing, allow to store them in the saved game setup and
bring them back when this setup is loaded. (#4127)
- Allow paths to directories in the WADs/files box, just in case some
game decides that it can handle that.
- Players table appearance changed:
    - The header row is now stylized with a bold font.
    - Also, there is a dashed border between the columns (needs Qt >=5.14).
    - As previously, the table is divided into sections: players, bots,
      spectators, but the title of each section is now displayed.
    - For team games, each team has its own section now and the name of the team
      appears as the respective section's title. Players don't have the name of
      the team repeated in each row anymore.
- Redesign server details. Use a tabbed view, present a lot more info which was
previously only available through the tooltips, and say it explicitly when said
info is missing.
- The server tooltips restyled to separate the labels (in bold) from values.
- Present demos in a table instead of a date-time list. There's more metadata
stored for the demos now, and the table displays it all. (#4110)
- Demo manager's Play, Remove and Export buttons will now be disabled when
no demo is selected.
- Remember the previous export dir in demo manager.
- Demo metadata format altered. Doomseeker calls this metadata format V2, but
it will continue to read the previous format. (#4110)
- Managed demos storage format altered. Demos are now also organized by
author and by recording year in subdirectories, avoiding demos from
multiple authors colliding with each other. (#4110)
- Updated the internal zlib library to 1.2.13.
- Updated the internal bzip2 library to version 1.0.8.
- Wadseeker GUI: show Wadseeker logs more like actual logs: use a fixed-width
font, present the logging level and who wrote a given log line (Doomseeker or
Wadseeker).
- Wadseeker: revise the doxygen documentation, correcting the outdated
information and rephrasing the text for better clarity. Populate the doc's
main page with info on how Wadseeker works.
- Wadseeker: rewrite the internals of the lib so that it becomes
a better netizen.
- Wadseeker: upgrade the LZMA SDK to version 23.01.
- Wadseeker: website dudoomers.com changed to audrealms.org. (#4131)
- Q-Zandronum, Zandronum: the command line for launching a demo
playback will now be bereft of arguments that wouldn't affect the
playback anyway.
- Zandronum: clarify the popup message when a testing binary ovewrite
is about to occur; the first sentence says now that the program wants
to install the binaries.

### Deprecated
- Wadseeker: `WADSEEKER_API` unused in Doomseeker:
    - `NetworkReplyTimeouter`
    - `WadDownloadInfo`

### Fixed
- Players table getting squished horizontally in the server details dock.
- Limit the height of players tooltip for very populated servers. Doomseeker
will attempt to calculate the height needed by the players table and if it
exceeds a certain percentage of the available screen space, it will trim the
player list displaying only that there are `<N>` more players on the server.
This should prevent the tooltip from stretching beyond the screen. (#4149)
- Demo metadata: remember the order in which the required and optional WADs
were loaded if they were interleaved during demo recording. (#4110)
- Paths to the managed demos store should not be template-resolved, because
it's not user-configurable (#3262).
- Application icon was missing under Wayland.
- Parsing of IP2C now works correctly under Qt6.
- Wadseeker GUI: in a hypothetical situation where the lib would inform about
a progress on an empty filename, the program would crash. Protect against that.
- Wadseeker: spell "the /idgames Archive" correctly, like the Archive itself
does.
- Wadseeker: it would sometimes get the same URL twice if it at first thought
it would be a HTML website, but it turned out to be a WAD, PK3, or an archive
instead. This was causing unnecessary downloads, unnecessary traffic, and
complaints from server owners that Wadseeker strains their servers. This was
fixed by completely rewriting the Wadseeker's internals.
- Q-Zandronum, Zandronum: single-player demos will now be recorded with
the correct `.lmp` extension.
- Q-Zandronum, Zandronum: LAN broadcast servers were having their addresses
stored as IPv4-mapped IPv6, causing an IPv6 address being passed to the game's
command line when trying to connect to those servers, and the game failing to
handle this properly. This was happening in environments where IPv4 connectivity
was used otherwise. These IPv4-mapped IPv6 addresses shall be converted to
explicit IPv4 addresses now. (#4191)

## [1.4.1] - 2023-03-12
### Added
- Tooltips to the buttons in the file paths config page.

### Changed
- Rename "Add" buttons that would add empty path elements to lists to
"Add empty". (#4102)
- Wadseeker: allfearthesentinel is now a .com domain, not a .net. (#4099)
- Zandronum: correct the appearance of the testing binary download
progress dialog - fix the title and remove the `?` button.

### Fixed
- Avoid refreshing the same game server twice in a quick succession if
the server is both pinned and returned from the master server. (#4093)
- Siblings to a recursive file search path would be ignored if their basename
would begin with the basename of the recursive path. (#4106)
- The create game box will ignore the empty paths to additional files, thus it
will avoid showing error boxes saying that an empty path doesn't exist. (#4102)
- Updater: fix failure to update in `--portable` mode by passing the absolute
paths to the updater.exe. (#4105)
- Q-Zandronum, Zandronum: servers were queried with only a partially prepared
challenge, which could result in the extended server information not being
returned as expected.
- Zandronum: the changes in 1.4.0 to the portable paths have broken the
generation of the launcher scripts for the testing binaries. Fix that. (#4101)
- Zandronum/Linux: the testing binaries come bundled with dynamic
libraries. `LD_LIBRARY_PATH` needs to be set to the testing binary
directory or else those libs won't be found.

## [1.4] - 2023-01-21
### Added
- Upgrade the IpToCountry.dat file format to version 3. The license text
with translations and the database's homepage URL are now embedded into
the database itself.
- Server filters can now be saved as presets. (#4079)
- A tri-state filter for password-protected servers. (#4008)
- A button to reset the game's master server back to default. (#3756)
- New command line argument `--basedir` that changes the base directory
where Doomseeker stores all of its managed files: settings, cache, managed
data. (#3452)
- User-configurable paths now accept some template placeholders that are
dynamically replaced when the path is resolved (#3262):
    - `~` is resolved to user's home directory,
    - Linux only: `~username` is resolved to user's `username` home directory,
    - `$[ENVVAR]` (without the `[]`), if placed at the beginning of the path,
    gets resolved to the environment variable `[ENVVAR]`,
    - `$PROGDIR`, if placed at the beginning of the path, gets resolved to the
    directory where the Doomseeker's executable is stored (yes, it may be `/bin`
    on Linux or app bundle on macOS).
The `$[ENVVAR]` and `$PROGDIR` resolution happens only when the user enables
it in the config box.
- New command line arguments `--enable-plugin` and `--disable-plugin` for
users who wish to force allow or force deny loading of a plugin.
- Check if the demo storage directory exists and is writable when demo recording
is enabled. If there's something wrong, warn the user. (#3501)
- Granular filters for banned, "refreshed too soon" and not responding
game servers. (#4002)
- The additional files list in the create game box now responds to plus
(add empty), minus and delete (both remove selected) keys on the keyboard.
- Demo manager now openable with the F3 keyboard shortcut.
- Hide/reveal password checkboxes to all password inputs.
- Wadseeker: add a bunch of websites to the WAD search list: (#4053)
    - https://doomshack.org/wadlist.php
    - https://wads.firestick.games/
    - https://euroboros.net/zandronum/wads/
    - https://static.dudoomers.com/wads/
- Cyprus country flag.
- Zandronum, Q-Zandronum: add "Don't count monsters in the 'end level' sector
towards kills" checkbox to the Create Game box. (#4055)

### Changed
- The portable mode adjusted to truly support carrying Doomseeker on
a portable device. The configurable paths will be saved and stored in
the configuration as relative to the location of Doomseeker's executable.
If both the games and Doomseeker get placed on the same portable drive, it
should no longer matter under which letter/path the portable device appears
in the system. (#3354)
- Refresh the servers in a randomized order rather than the order
in which they were provided by the plugin (ergo: by the master
server, ergo: sorted by IP). This should be more fair, as the previous
method favored servers that were in the lower IP ranges.
- Updated Flag of Ukraine colors to DSTU 4512:2006. Previously, Doomseeker was
displaying the flag of Post-Soviet Ukraine, a design with lighter blue color
shade, from 1991-1992. (#3983)
- DemoManager: demo playback can now be started with a double-click or enter.
- In case if Doomseeker uses the old version 2 file format for IP2C,
redirect the IP2C licensing info to a Doomseeker's webpage instead of
displaying anything that was hardcoded at the time when Doomseeker was
compiled and could potentially not be accurate anymore at the time of
viewing. (#3952)
- Hide the IP2C update actions in the IP2C updater dialog box if the
updater determines that the local DB is the same as the remote one. (#3731)
- When user attempts to join a game without having the game configured,
the error box will now have the "Configure game" button instead of just opening
the config box indiscriminately. Also make the related error messages sound
less robotic. (#4019)
- Buttons for the custom parameters config stay in place, behave more
predictably, have tooltip descriptions. (#3754, #3755)
- The master server messages (about the user being banned) will be
automatically cleared when the user starts a full server refresh. (#3584)
- Don't count the spectators as joined players when showing how many players
can still join a game. (#4038)
- Doomseeker maintainers info in the about dialog is now labeled as such
and the email links are clickable with the `mailto:` prefix. (#4069, #4070)
- Correction of various inconsistencies in the UI (#3676):
    - Use the standard dialog box buttons anywhere the custom "Close",
      "Cancel" and etc. buttons were. This will also ensure that these
      buttons are in-line with the current OS theme.
    - Use natural capitalization for buttons, etc.
    - All "Browse directory" buttons will now have the directory icon
      and an apt tooltip and the title where appropriate. No more mixing
      between "..." and "Browse".
    - The "Default" buttons will have the Doomseeker 'D' icon.
    - The "Clear" buttons will now use the "edit-clear" Freedesktop
      icon, defaulting to a broom icon if "edit-clear" isn't available;
      they don't use the same red X that the "Remove" buttons use.
    - Toggle the visibility of the server passwords in the config
      with a checkbox as in other places, not with a button.
    - Make the "Add defaults"/Add/Remove layout the same in both cases.
    - "Delete" text changed to "Remove" for consistency.
    - The program arguments help box's title will now include the name
      of the program. Also add the terminal icon to its main menu action.
    - The labeled "extra settings" in the Create Game box will end with
      colons (#4074).
- The "Clear" button in the server filter renamed to "Reset filter". (#4078)
- The additional files picker in the Create Game box reorganized to reclaim
some vertical space and make it more clear that the buttons next to it
belong to it. (#4072)
- Hide the passwords by default!
- UI improvements to the server passwords config box.
- Plugin API: rewrite the documentation for `DataPaths`.
- Wadseeker: make the "managed data" directory (Local) the default for
WAD downloads.
- Wadseeker: added `sigil_shreds` and `sigil_shreds_compat` to the
list of forbidden downloads.
- Wadseeker: upgrade LZMA SDK to version 22.01. (#3934)
- Odamex, Zandronum, Q-Zandronum: untie the max. clients and max. players
settings when creating a server. Previously if max. clients was lower than
max. players, max. clients was increased to the max. players value. Now
Doomseeker won't make that correction anymore and just pass both numbers
to the games as-is. (#4038)
- Q-Zandronum: adapt plugin to Q-Zandronum 1.3.
- Zandronum, Q-Zandronum: rely on Doomseeker's internal mechanism to
calculate the ping to the game server rather than using the port's
feature where the timestamp is sent back in the challenge reply
packet. (#3992)
- Zandronum: remove the defunct IRC server.

### Deprecated
- Wadseeker: `Wadseeker::setWadArchiveEnabled()` (#4048)
- Plugin API:
    - `DataPaths::canWrite()`, `DataPaths::setWorkingDirectory()`,
      `DataPaths::workingDirectory()`, `DataPaths::validateAppDataDirectory()`.
    - `Server::numTotalSlots()` (#4038)

### Fixed
- Use signed integers to display the dmflags in the server details. (#3944)
- Buffer the messages in the RCon to fix performance issue when many
small messages arrive. (#4026)
- The "Put populated servers on top" option would not be applied to the
server list on program start if disabled. (#3772)
- Wrong amount of free server slots was reported if server allowed more
players to join the game than clients to connect to the server. (#4038)
- Bring up the context menu for the server list and for Wadseeker's WADs
table when the keyboard menu key is pressed. (#3671)
- Fix the layouting issues with progress bars in Wadseeker's dialog box. (#3707)
- When Doomseeker opened a config box for a specific plugin, it failed
to show the config page for that plugin, showing a blank page instead. (#4019)
- Create Game box: display a proper error message if the game executable
path is empty. (#3839)
- Create Game box, server page: fix the spacing of the form when various
inputs are hidden.
- Fix imprecise issue tracker hyperlinks in the documentation. (#4063)
- Pass all the original command line arguments from the auto-updater
back to Doomseeker after the update is installed.
- Auto-updater will now create a log file in Doomseeker's current local
data directory, as intended.
- When showing the command line for offline play, the demo record
command should use the unmanaged (relative) demo path, not the path
to Doomseeker's managed data storage.
- Prevent horizontal scrollbar in the server filter panel by enforcing
a minimal width.
- The `true` values in the config sometimes interpreted as `false`, leading
to Zandronum's testing builds disabling themselves if user canceled the first
time configuration. (#4080)
- Sometimes the main window of the program was expected to exist even though the
opposite could be the case, in which case the program would try to use the
absent main window anyway and crash. One such case was if the configuration box
was opened from the Create Game box after the main window was closed. Fixed that
by adding some guards that check if the main window is gone. (#4087)
- Wadseeker hyperlink in the about dialog made accessible with keyboard. (#4071)
- Wadseeker: custom (server-provided) template URLs with the %WADNAME%
placeholder wouldn't get the higher priority status that the plain
custom URLs normally enjoy. (#3996)
- Windows: updater.exe will use `_splitpath()` instead of `_splitpath_s()`,
allowing it to run again in Windows XP where the MSVCRT doesn't have the
`_s` variant of the function.
- Turok 2 Ex: fix writing beyond the size of the buffer when encrypting the
challenge packets. This used to work in Qt previously but is now deprecated.
- Q-Zandronum: set the canonical name of the plugin to `zandronumq`, which
fixes the loading of the translation files.
- Zandronum, Q-Zandronum: servers that emitted empty PWAD names in their
challenge reply would appear as errored because the plugins failed to
parse the subsequent MD5 checksum hashes for these PWADs. The plugins
will now sanitize such empty PWADs so that such servers can appear
correctly on the list. (#3624)
- Zandronum, Q-Zandronum: `+compatflags2` not set from the Create Game
box if the (Q-)Zandronum tab was not selected. (#4088)
- Zandronum: fix a failure to parse the version of the game for the
servers, which resulted in the dmflags being displayed incorrectly
in the server details and the game's beta builds from being
installed. (#4020, #4024)
- Zandronum: fix the game setup page (#4055):
    - "Keep teams after a map change" flag wasn't toggling its
      corresponding zadmflag.
    - Add proper voting flags and hide the obsolete for Zandronum 3.0.
    - Auto-correct the spin boxes in the flags page to the nearest
      allowed value instead of snapping to the previous one.
- Zandronum: update the game icon to Zandronum 3.1's.

### Removed
- Config migration from Doomseeker 1.1. (#4081)
- Wadseeker: Removed Wad Archive support due to the site shutdown. (#4048)
- SRB2: plugin deprecated, discontinued and removed from Doomseeker. (#3958)

## [1.3.3] - 2022-01-18
### Added
- An IP2C option to toggle honoring of the countries received from
the game servers; enabled by default. If enabled, display the country
flags and the locations basing on this info. (#3939)
- The game settings inputs in the "Rules" panel of the Create Game box can now
be more elaborate than just "setters for numeric values". If a game requires it,
these inputs can be rendered as checkboxes or text editors.
- Odamex: Support for Odamex version 10.0 (#3909)
- Zandronum: Add the new settings from Zandronum 3.1 to the Create
Game box: DM and compat. flags, server country, voting permissions,
respawn delay times. (#3938)
- Zandronum: accept the country code sent by the game servers. (#3939)
- Zandronum: accept the minplayers/maxplayers arguments on the maplist. (#3940)

### Changed
- Move the IP2C options to a separate configuration page.
- Stick the description labels to the checkboxes in the appearance config.
- Permit parameters on the map list, ignore them when checking if a map is
on the map list. (#3940)
- Doomseeker now knows the list of countries and their alpha-3 codes
by heart (it's hardcoded), instead of drawing them from the IP2C
database. The country names can now be translated. (#3957)
- Remove the useless question mark button from the titlebars of the
dialog boxes.
- Make the UI more spacious by removing paddings in the main dockable
windows and in the configuration box. (#3676)
- Odamex: Adapt the game modes in the Create Game box to how Odamex wants
them. The box will present only the selection of the 5 base modes: Coop,
DM, Team DM, CTF and the Horde mode from Odamex 10.0. The other game
modes can be activated by enabling the lives and the sides settings on
the Rules page. (#3937)
- Zandronum: Clean-up the layout of the "Zandronum" tab in the
Create Game box.
- Zandronum: Rename the "Disallow - Use custom GL lighting settings"
to "Disallow - Use custom renderer settings" because this is what it
actually does.
- Under "Configuration", the "Wadseeker - General" config node has been moved
to the "Wadseeker" config node. The "Games" node now also displays an empty
configuration page. Previously, both of these nodes did nothing. (#3953)

### Deprecated
- SRB2: The Sonic Robo Blast 2 game plugin may be soon removed from the
Doomseeker distribution. After removal, the core Doomseeker development team
will no longer develop, maintain, or test compatibility with Sonic Robo Blast
2. (#3958)
- Windows XP support may soon be removed from the Doomseeker distribution.
After removal, the core Doomseeker development team will no longer develop,
maintain, or test compatibility with the Windows XP operating system. (#3964)

### Fixed
- The warning that warns about the current map not being on the map list
wouldn't appear or disappear if the map entry was edited directly.
- Minor text changes throughout Doomseeker and the plugins to improve on
robotic or clumsy language, also (or primarily) in the Polish
translation. (#3942, #3943)
- Windows: Update the bundled "cacerts.pem" with the ISRG Root X1 root
certificate which replaces DST Root CA X3 that expired in September 2021 (#3954)
- Windows: Update OpenSSL to version 1.0.2u and remove OpenSSL's dependency on
MSVC 2013, which really should not be there since the program's executable
distribution is built in MinGW. (#3955)
- Zandronum, QZandronum: fix a crash if server sends more PWAD hashes than
it sent PWADs. (#3927)
- Zandronum: dmflags and compatflags are signed integers, not unsigned. (#3944)

## [1.3.2] - 2021-08-07
### Added
- Wadseeker: multiple custom sites can now be defined
- Odamex: Support for Odamex version 0.9 (#3878)
- Q-Zandronum: Plugin added (#3866)
- SRB2: Support SRB2 version 2.2.6 (#3851)

### Fixed
- Windows: updater.exe is now linked statically with the runtime libs (#3759)

## [1.3.1] - 2020-01-27
### Fixed
- Hidden server list columns would still become exposed if server name
filter was used. Fix this by removing these columns altogether. (#3411, #3703)
- Disable the ignore button in the WAD download prompt unless all the WADs
are optional files, there are only incompatible WADs or the game can download
the WADs instead. This prevents from using this ignore button in a state where
it would lead to launching the game in a corrupted manner. (#3695)
- Simplify a bit the IP2C update routine. The parser thread will no longer
update the live IP2C data as it is being used. It now signals the main
thread that the parsing has finished. The main thread can then take this
parsed data and swap the IP2C database on its own convenient terms. Potential
race conditions are mitigates by this, which may help with resolving IP2C
update crashes. (#3395)
- SRB2: it's possible that a server will not report any files loaded.
This possibility was not anticipated and if encountered, resulted in
an assertion causing the whole program to close. (#3728)
- Windows: loading game plugins wouldn't work if the "engines" directory
was in a path that had non-ASCII characters. (#3706)

## [1.3] - 2019-07-27
### Added
- WAD checksum verification when joining a game. The checksum verification
is done for servers where the games provide the checksum info for the
mod files used on the server (Odamex and Zandronum). Doomseeker collects
these checksums during the usual server refresh procedure. It can then
use this info to calculate the checksums of locally stored mod files and
then compare them to the checksums provided by the game servers. If checksums
mismatch, user will be informed about this. The checksums are also used
during WAD downloads in Wadseeker. If Wadseeker knows the checksum of the
desired file and this checksum doesn't match after download then the downloaded
file will be rejected. Because checksums verification can be lengthy for
large files, a prolonged verification procedure will pop a dialog box where
the verification can be manually skipped. User also has an option to disable
the verification altogether from Doomseeker's configuration. This addresses
Mantis ticket #3369.
- "Create Game" box:
    - Allows now to define a game log storage directory for games that support
      it. Odamex and Zandronum plugins are wired up to use this feature.
      (addresses #3568)
    - "Pick maps for maplist" button added. The UI lists maps from loaded game
      mods and allows to select them for the maplist in bulk. IWAD maps are
      displayed in a gray color, while normal color is used for maps that are
      present in the loaded custom mods. The list also displays which mod file
      each map comes from through a tooltip. Reading map lumps from WAD and PK3
      formats is supported, as well as extracting mods from archives and parsing
      them. (addresses #3599)
    - CTRL+S and CTRL+O keyboard shortcuts can be used to save/load
      game config (#3645)
- File (game mod, WAD) search paths can now be reordered in the config box
by drag'n'drop. (#3669)
- Catalan translation by Pol Marcet Sardà

### Changed
- Completely dropped compilation support for Qt4. (#3514)
- On non-Windows systems don't seek mod files (WADs) in the executable
directory, since it's usually somewhere like /usr/bin. (#3625)
- Windows compiler change. Use MinGW-w64 to build Doomseeker instead of
Microsoft's Visual Studio Compiler with v120_xp toolset. From the source-code
perspective, this allows to use the modern C++ standards. From the end-user
perspective, the program performance seems to have improved by 50%, while
in exchange the size of the release package has increased by 25%. (#3598)
- Windows: error dialogs will no longer pop-up when incompatible plugin DLLs
are loaded. Incompatible plugins now simply not appear as available and
the load error is only logged to the program log. (#3401)
- Present the Plugin ABI version of Doomseeker in the About dialog. (#3644)
- Present the Plugin ABI version of each plugin in the About dialog
as a separate value instead of as "%ABI.%actual_version" string.
Plugin's actual version is now a single number. (#3644)
- "Create Game" box:
    - There's now a distinctive split between hosting a server and
      playing an offline game. This choice is no longer made as a final
      step by pressing one of the two "Start" buttons, but is now
      determined from the "Create Game" dialog box menu that also
      affects how this "Create Game" box looks like. When "offline"
      mode is selected, the options that only make sense for a server
      hosting are hidden. The values stored in those options are,
      however, remembered and will be saved in the configuration if user
      chooses to switch their mode back to "host a server". There is
      some visual feedback provided to inform the user in which mode
      the box currently is: the title of the box is directly related
      and also the text of the "Start game" button adjusts. (#3636)
    - Move config-related bottom buttons of the "Create Game" dialog box
      to menu bar actions. The menu bar is presented at the top of that
      dialog box just like in the main window. Game config, program
      config and launch mode can now be picked from that menu.
    - Another (initially invisible) change splits the behavior of
      the game executable selector combo box. Each of the 3 modes:
      "Play offline", "Host a server" and "Configure remote game"
      now track separate game executables. This is to accommodate
      for source-ports that use more than one executable.
    - Rename "Misc." tab in the Create Game box to "Server".
- Local wad searching:
    - Now there's no need to add the path from Wadseeker's general settings
    since it will be automatically added to the list of dirs to check. That list
    will be checked is better filtered to remove repeated directories. (#3639)
    - On case-sensitive systems (non-windows systems) the search now caches the
    results in each file search iteration, meaning that in servers with lots of
    wads the search will last less. (#3638)
    - Also, now we no longer check for wads on the binary folder on those
    systems. (#3640)
    - Recognize duplicate search dirs and subdirs of recursive paths and don't
    search in them more than once.
    - Overall, the time that it takes to find a set of wads on the tested Linux
    environment was around 48 times less.

### Fixed
- If no plugins are loaded only one notification will now appear instead
of two notifications that said the same thing only with different
phrasing. (#3401)
- Manually bind client UDP sockets to address "0.0.0.0" which fixes
problem with the sockets not working on OpenBSD system. (#3494)
- Fix minor visual problems with the server tables. Enforce the size of the
fixed-width columns even if configuration tries to load a different size.
Don't display the focus rectangles in the game icon and player slots columns.
This fixes tickets #3461 and #3463.
- Buddies not being detected in LAN servers. (#3397)
- Fix visual glitches when modifying contents of the WAD alias table in the
config box and when pressing the "Apply" button for that table. (#3642)
- Prevent empty wads from Zandronum servers, this was caused rarely by some
servers that reported a non-existent wad. (#3624)
- SRB2: Discard master server header replies that don't have the expected
length declaration, thus preventing crashes when trying to read them.
The relevant
[CVE Identifier](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2019-12968)
has also been created. (#3660)



### Removed
- The "Allow servers to display my country" option in the zandronum's config
menu has been removed since Zandronum can now handle this option by itself.
(#3626)

## [1.2] - 2018-10-27
### Added
- List the '--help' argument in the help.
- '--version-json' argument can now write to stdout if file is not specified
or specified as '-'.
- "Remove IRC network" button was added to the IRC network selection dialog.
- Attributions for icons added to the about dialog.
- Allow to load player slots styles from "theme/slots" directories that can
be placed in the static search paths.
- Extra CA certificates for SSL will be loaded from "cacerts.pem" file
if such file is present in one of the program's static data directories
(so, right next to the .exe on Windows).
- It can now be controlled whether logs are printed to stderr
by '--quiet' and '--verbose' command line switches. Logging
to the "Logs" box will happen regardless of this switch.
- RCon connection box can now toggle showing/hiding passwords.
- Create Game misc. tab can now toggle showing/hiding passwords.
- Button to show JSON library license added to the About dialog (addresses 3451).
- Servers can now be dynamically pinned or unpinned using the server
list's context menu. This adds them to the "Custom/Pinned Servers"
list and essentially servers the purpose of marking servers
as favourite (addresses 3388).
- "Custom/Pinned" servers can be disabled without removing
them from configuration.
- File aliases can now be either configured as "left-to-right" where a single
file (WAD) can be replaced by one of aliases, or can be marked as equal where
each file is replaceable by any other file on the same list. (addresses 2740).
- '--create-game' argument added; it launches Doomseeker into the
"Create Game" box, omitting the main window altogether. (addresses 3479)
- Allow to pick *no* game mode and *no* difficulty level when creating
a game. The idea here is to allow the game to decide on these values or
to start the game into the title screen instead of dropping immediately
into the map. (addresses 3505)
- Translations contributors mentioned in About dialog. (addresses 3516)
- Guyana, Bosnia and Herzegovina, Cameroon, and Malta country flags. (addresses
3327, 3330, 3457 and 3539)
- IRC: User name can now be specified (this is different than nick name).
- Windows: cacerts.pem database with "DST Root CA X3" and
"DigiCert High Assurance EV Root CA" certificates.
- Zandronum: Support "sv_deadplayerscankeepinventory" zadmflag.

### Changed
- **Relicensed Doomseeker and plugins to LGPLv2.1+.**
- Disabled autoupdates on macOS due to Apple restrictions on signed apps.
- Reworked data paths so Doomseeker should now follow XDG on Linux (Qt5 only).
Configs will be migrated. On Mac the demos are migrated to Application Support.
On Windows, paths will be moved to %LOCALAPPDATA% and leading dot is removed
from "doomseeker" directory name.
Otherwise paths are more or less the same as before.
- IRC network selection dialog buttons have only the icons and no text now.
- Replaced any hardcoded HTTP addresses with HTTPS wherever applicable & possible.
- Replaced several icons that would violate the LGPLv2.1+ licensing.
Most notably, the "Buddies" icon that depicted 2 Doomguys was replaced
with 2 ballheads. Sorry, I hate this too.
- "Marines" slot style is considered non-free and conflicting with Doomseeker's
licensing, hence it will now be distributed as slot theme using the new theming
system. This change should be transparent to the end-user.
- To support slot themes, changed how slot style is saved in the configuration.
The setting will not be preserved correctly when going back to prior versions
of Doomseeker.
- If Doomseeker is installed into a directory named "bin" (typical on Linux)
or "MacOS" (application bundle), the binary directory will no longer be added to
the wad file paths by default.
- Updated the internal zlib library to 1.2.11. This affects Windows builds.
- Logs will now be printed to stderr instead of stdout.
- Only show games that support Remote Console in RCon connection box.
- Starting Doomseeker with incorrect command line arguments will now
print help and quit immediately (addresses 3294).
- Reword a bit the log and error messages associated with total server
refresh - don't say that "the operation is senseless" and don't notify
about problem with deselected masters when there are some permanent
(pinned) servers on the list.
- "Custom Servers" are now known as "Pinned Servers" (addresses 3388).
- Custom Servers config box: "Set Engine" button now says "Set Game".
- When server is already "pinned", prevent it from appearing twice
on the list if master server also reports it.
- Several appearance settings, such as server table colors, player slots
style and grid will now be dynamically applied as the user changes them
in the configuration box. This allows for quick preview (addresses 3306).
- When user changes the language, inform that full translation is only applied
after the restart of the program (addresses 3482).
- United Kingdom flag to current revision from year 1801; was the pre-1801
Union Flag (of Great Britain). (addresses 3365)
- Create Game box: moved difficulty setting from the Rules tab to the General
tab. This allows to disable the Rules tab fully for Chocolate Doom for which no
other widgets are present on this tab. The setting also fits better when its
grouped with game mode and map selection (related to 3480).
- Fix problems when specifying invalid basic patterns in the Buddies
list. It was possible to create patterns that were accepted in the list
but were actually invalid to the Qt's regex engine. Such patterns would
not be saved in the config and they would throw off the search & destroy
routine when deleted from the UI. Right now, the "Add Buddy" dialog
box should outright reject invalid patterns (addresses 3398, 3399).
- Linux: Also try to look for WADs in /usr/[local]/share/games/doom.
- Linux: Explicit "Copy (text)" actions shall store the data both in keyboard
and mouse selection clipboards, allowing it to be pasted with either keyboard
or middle mouse button (addresses 3257).
- Linux: Move plugins to /usr/lib/doomseeker (addresses 3232).
- Windows: install with the "INSTALL" target the license file for the updater.
- Wadseeker, API: Removed FixedNetworkAccessManager class from API.
- Odamex: When launching game client, use Doomseeker's WAD finding mechanism
specialized for this task instead of the generic one.
- Zandronum: Zandronum 3 was released, so made its ruleset the default in
Create Game box and removed its beta status.
- Zandronum: default testing binaries path is now in "local data"
directory, not in "documents".

### Fixed
- Count players and decrease refreshing servers amount just once for
each server. This should fix incorrect numbers in the status bar.
- Display Doomseeker icon in Demo Manager dialog.
- Set correct title on IRC network selection dialog.
- Adjusted size of the configuration dialog and its widgets to minimize
the risk of its size jumping around when browsing or the text becoming
hidden behind ellipsis.
- Fix broken English "time has past" (addresses 3269).
- "Refresh server before game launch" option did not work at all.
- Manually refreshing a server for which a Wadseeker window was open would
show "Wadseeker is running, do you wish to ignore missing files" window.
If user clicked "cancel" or "ignore" on such dialog box, Doomseeker crashed
(addresses 3268).
- Prevent infinite IP2CLoader loop if IP2C fallback is broken. This could
never happen in reality because an always valid IP2C fallback is always baked
into the executable.
- It was possible to create an IRC network configuration with no address
or port 0, but it was impossible to remove it.
- The widget that displayed IRC network port when connecting to IRC network
had a minimum value set to 6667. If IRC network used a port lower than 6667,
it would not display properly.
- Remote Console connect dialog had no title.
- Starting Doomseeker with "--rcon" argument and then immediately closing
the connection prompt or failing it would get Doomseeker stuck running forever.
- Connection prompt box with "--rcon" argument would not be centered
on the screen because its parent widget was not yet drawn when
the prompt was first shown.
- Players with TEXTCOLO color names (custom colors in brackets) would not
properly show up on the buddy list (addresses 3193).
- Player score would be displayed incorrectly if it exceeded 32768.
Moreover, it was also wrongly assumed that the player score cannot be negative.
- Show more precise error messages when there are issues with one of the
Doomseeker's data directories other than "failure to create" (addresses 3396).
- Fixed problem with exposing hidden columns or other unintended behavior in
servers table when restoring state from a saved config. (addresses 3411)
- Ensure contents of copy text dialog are displayed from the top.
- Server counting widgets would not mark servers that come in with
already known data as "counted", which resulted in counting them twice
upon removal, which throwed the refresh counter off. This problem was
triggered by "Custom/Pinned Servers".
- Localization: Doomseeker has now option to always try to use
the same language as the OS. This is the default setting for new
configurations. (addresses 3481)
- Be more flexible when matching locale names to program localizations
definitions. Try to match locales like "en_GB" to the default English
translation. Try to match known localization definition just basing on
the "language" part of the "language_country" locale code. This should
fix issues when loading translations for a matching language but
mismatching country and with "Appearance" config box displaying an
invalid translation for a language that is known to the
program. (addresses 3260)
- As English translation is hardcoded into the program, it's no longer
necessary to explicitly mention it in the .def file. It is now
available by default regardless if there are any .def files or if any
of those files contain it. This change should be backward-compatible
with .def files that define localization for the invalid 'en_EN' locale
- Try to remember previously selected game modifier when the "Rules" panel
in Create Game box resets upon actions such as game or game mode change.
- IRC: connect to networks by using the hostnames directly, allowing
Qt to choose the most appropriate address. This should prefer IPv6 connectivity
when available.
- Linux: The name of the desktop file is now a reverse Doomseeker domain.
- Wadseeker: Fixed extraction of bz2 files with multiple streams.
- SRB2 & Turok 2 EX: fix translations not being loaded. (addresses 3554)
- Zandronum: fix problem with widgets for a different Zandronum version being
visible when first opening the "Create Game" box.
- Zandronum: fix tabulation focus ordering for Zandronum's Create Server page.
- Zandronum: Fixed silent error loop if testing binary extraction failed.
- Attributions, licensing, copyrights, "FOSS stuff": a lot of effort has
been put into cleaning up the source code from various license violations,
lack of appropriate attributions, lack of backlinks to the source material
and imprecise or missing licensing information. With an on-and-off effort
it took over a year to find and fix all of the problems. Special thanks in
this regard go to WubTheCaptain. Without her ability to meticuously spot
the issues, and patience and effort put into explaining the problems and
appropriate solutions, this task might have not been possible (addresses
3237 and children).

## [1.1-p1] - 2017-07-17
### Added
- Turok 2 EX plugin.

### Fixed
- Disconnect QProcess's signals from "Standard Server Console" when
closing. This should prevent "process closed with SIGKILL (exit
code 9)" message that could appear whenever Doomseeker popped
up a server console window.
- "Standard Server Console" will also try to gracefully terminating
the process first and only issue a SIGKILL after a 1 second timeout.
- Zandronum: use "$@" instead of $* in Linux launch scripts for
testing builds. (also add a trailing \n to these scripts)
- Zandronum: dmflags values from some widgets weren't applied when
loading server config.

## [1.1] - 2017-06-05
### Added
- Provide more information in game status widgets (bottom-right corner).
Humans and bots are now counted separately and together, refresh progress
is included and the numbers also encompass custom and LAN servers.
- Provide more statistic information in Doomseeker's tray icon's tooltip.
Tooltip is also refreshed live during server list refresh (it might
require moving cursor away from the tray icon depending on platform.)
- Human-readable versions in update packages file names.
- Redistribute OpenSSL libraries together with Doomseeker.
- Servers can be filtered by the "testing server" flag.
- Use global game custom parameters when hosting a server or creating
offline game. These parameters are used in addition to parameters
defined in "Custom Parameters" tab in Create Game dialog.
- Support varying executables for each game instead of just "game.exe"
or "client/server.exe". Chocolate Doom uses different executables
depending on IWAD and Doomseeker now supports that.
- Suggest different game executables in Create Game dialog depending
on what the plugin for each game declares. Executables can be picked
from a drop-down list. Zandronum testing executables should also be on
this list.
- Added "Install Freedoom" dialog box to main menu (under "File").
- Apply default values to game limits in Create Game box
if engine plugin provides them.
- Log plugin load failure when ABI version mismatches (when plugin is too old).
- Added "play sound" button to IRC sounds configuration page.
- Added UPnP checkbox to "Create Game" box. Works with Odamex and Zandronum.
- Paint 'T' icon over game logos for testing servers.
- IRC: Send NOTICEs to channel or priv chat box if such box is open,
send them to network tab if not.
- IRC: Consider reply code 338 (RPL_WHOISACTUALLY) a part of /whois
response.
- IRC: Parse response codes RPLChannelUrl (328) and
RPLCreatedTime(329) and forward them to originating channel's
window.
- IRC: Parse channel related errors 471-477 (bad password, banned,
etc.) and forward some of them to current chat box.
- Windows: Server list refresh progress and Wadseeker download progress
are now shown in Windows' task bar.
- Wadseeker GUI: Added option to always use Wadseeker's hardcoded list
of sites as it exists in the currently used Wadseeker version.
With Wadseeker updates new sites may come and old (defunct) sites may go.
This option is on by default.
- Wadseeker: Added "Fetch Freedoom version info" feature to
WADSEEKER_API.
- Wadseeker: Added ModInstall class to WADSEEKER_API for mods with
known URLs.
- Wadseeker: Queries to services (idgames & wadarchive) are shown in
the table with site queries.
- Wadseeker: Sites and services (idgames & wadarchive) queries can
be aborted by the user.
- Wadseeker: install files from archive's subdirectories if archive's root
doesn't have them. This indirectly fixes problem with Freedoom no longer
installing from the "Install Freedoom" dialog box.
- SRB2: New plugin for Sonic Robo Blast 2 - supports game setup,
master server, server queries, IRC channel.
- Zandronum: LAN broadcast servers will now appear on server list.
- Zandronum: "force spectator" voting can be configured from "Create
Game" dialog.
- Zandronum: display DeHackEd patches loaded on servers, allow the
game client to load them when joining.
- Zandronum: support switching between dmflag sets for Zandronum 2.x
and Zandronum 3.x. Allow to choose Zandronum version in "Create Game"
dialog. Understand Zandronum server versions when displaying
dmflags in "Server Details" info box.

### Changed
- Switched from Qt4 to Qt5.
- Windows: Filter files by *.exe, *.com and *.bat when browsing for
either server or client game executables.
- Windows & Mac: Update packages have been split from monolithic
'doomseeker' to smaller packages. Downloading updates won't
require to download Qt libraries everytime anymore.
- Completely reworked "missing files" dialog that appears when
trying to join a server and not having all the necessary files.
- "Find missing WADs" server list context menu action: allow user
to select the files that are to be installed.
- Added "Install Freedoom" button to "missing files" dialog
if Doom IWAD is missing.
- To reduce noise in generated command lines, wrap with quotes only
those arguments that have characters outside of a very narrow, safe
range.
- Use GeoLite2 database for IP2C.
- Restructured how game dmflags are stored to account for the fact
that games sometimes like to change numerical values of the dmflags
while dmflags themselves are effectively the same (ie. same name, same behavior,
just a different code). Doomseeker will now rely on an internal dmflag
name and deduce the value basing on what's currently programmed
into the plugins it uses.
- Server name filter, when non-empty, is now always in effect,
regardless of whether all other server filters are enabled or not.
This allow to search the list for a server name while not having
to enable other filters.
- hexdd.wad can no longer be picked as an IWAD in "Create Game" box.
- Updated internal 7z (LZMA SDK) to version 9.35 (from 4.65).
- Updated internal bzip2 sources to version 1.0.6 (from 1.0.5).
- Improved validation of configuration dialogs. User will be immediately
and clearly notified when Doomseeker detects a configuration problem.
Configuration dialog page tree will show a warning icon next to pages
that report problems. Pages that configure file or directory paths
have been programmed to report validation errors.
- Query speed improvements:
    - Split server querying onto several client sockets, max. 25 host-ports per
      socket. This prevents packets from "getting lost" and servers appearing as
      "not responding" during aggressive queries.
    - Lowered speed limits in "Query" configuration page. Users can now
      configure more aggressive queries (1 attempt per server, 1 second timeout,
      1 millisecond interval between servers).
    - Added "Very Aggressive" query preset that will push it to the limit.
    - "Aggressive" query preset: attempts reduced to 2 (from 3) and interval
      between servers reduced to 5 ms (from 10).
    - Made "Aggressive" query mode the default one for new configs.
- Changed appearance of color buttons to mesh better with OS X.
The altered look affects all OSes.
- Also for OS X purposes, minimally altered the sizes of "browse" buttons.
- Chocolate Doom: Don't show max players/clients and map list
widgets in Create Game box.
- Vavoom: plugin deprecated, discontinued and blacklisted by Doomseeker.
- Zandronum: automatically try to set "Testing binaries" root path
to Doomseeker's "My Documents" directory. This should help new users to start
using Zandronum testing builds without having to configure the path.
- Zandronum: allow to install testing version of the game even if target
directory exists and is non-empty. The message box that pops up when
encountering an unclean directory is slightly different and is displayed
with a warning icon to inform the user that pre-existing files may be
overwritten by the installation.
- Zandronum: Renamed "Flags" Create Game page to "Zandronum". The page has
more generic stuff now than just "flags" and the new name fits better.

### Fixed
- Don't reset difficulty or game limit values in "Create Game"
dialog when changing game mode.
- Auto-updater wouldn't install all files from auxiliary update packages.
This wasn't very critical for a long time as most files in auxiliary
packages (plugin's packages) weren't vital and the plugins themselves
were always installed correctly.
- Use QProcessEnvironment to obtain environment variables instead of
getenv(). The new solution should be unicode-aware.
- Downloading missing files from server list context menu:
    - Don't display "missing files" error message anymore if user chooses
      not to install optional WADs.
    - JoinCommandLineBuilder was never freed and created a memory leak.
- Possible buffer overlow in WinMain() when using characters outside
ASCII range in command line args.
- Replaced standard dialog buttons in demo manager with custom ones as
standard naming could be odd depending on platform ("close without
saving" instead of "delete"/"discard").
- Remember which PWADs were optional in recorded demos.
- "Update IP2C" menu action was available at the start of the program while
the IP2C parser/updater was still working. This behavior was incorrect,
as the menu action should always be disabled while parser/updater is working.
- Linux: Adhere to FreeDesktop standards regarding category and icon name.
- Linux: Proper "make install" destinations for:
    - Doomseeker.desktop - don't use absolute dir path, use "share/applications"
    - Doomseeker's icon.png - use "share/icons".
- Linux (openSUSE): Fix LZMA compilation issues.
- Wadseeker: Automatically abort queries to sites & services when all
files are installed.
- Wadseeker: Problems with URLs containing '%' character or non-ASCII
characters.
- Wadseeker: the /idgames Archive now forcibly redirects HTTP to HTTPS.
Wadseeker didn't handle this properly and this functionality stopped working.
- Wadseeker: fixed ZIP extraction to gather info from central directory rather
than from local file headers, which is the correct approach for not corrupted
archives. Fixes extraction of e1m4b.wad and e1m8b.wad. Thanks, John Romero!
- Chocolate Doom: improved recognition of what IWADs are hosted on servers.
- Chocolate Doom: Strife's IWAD is strife1.wad, not strife.wad.
- Zandronum:  Fixed "Sucide" typo in one of the Create Game's flags pages.

## [1.0] - 2015-03-22
- Version 1.0 released. No changelog was kept before.


Copyright (this file)
---------------------
Copyright (C) 2015, 2017 - 2024 The Doomseeker Team

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.


[unreleased]: https://bitbucket.org/Doomseeker/doomseeker/commits/all
[1.5.0]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.5.0
[1.4.1]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.4.1
[1.4]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.4
[1.3.3]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.3.3
[1.3.2]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.3.2
[1.3.1]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.3.1
[1.3]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.3
[1.2]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.2
[1.1-p1]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.1-p1
[1.1]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.1
[1.0]: https://bitbucket.org/Doomseeker/doomseeker/commits/tag/1.0
