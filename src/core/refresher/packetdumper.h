//------------------------------------------------------------------------------
// packetdumper.h
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
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_REFRESHER_PACKETDUMPER_H
#define DOOMSEEKER_REFRESHER_PACKETDUMPER_H

#include "global.h"
#include <QByteArray>
#include <QDateTime>
#include <QHostAddress>
#include <QString>

/**
 * Developer utility for capturing packets.
 */
class MAIN_EXPORT PacketDumper
{
public:
	/** Traffic direction relative to Doomseeker. */
	enum class Direction
	{
		Recv = 0, ///< Received from the peer.
		Sent = 1 ///< Sent to the peer.
	};

	/** Optional settings for a dump. */
	struct Options
	{
		Options() : label("raw") {}

		QString label; ///< Capture label included in the filename. Defaults to "raw".
		QString directory; ///< Output directory. Empty selects defaultDirectory().
		QDateTime timestamp; ///< Filename timestamp. Invalid selects the current time at dump().
	};

	/** The doomseeker-packet-dumps directory under QDir::tempPath(). */
	static QString defaultDirectory();

	/**
	 * Save raw binary data with relation to `address` and `port`.
	 *
	 * Each call saves the supplied bytes in a separate file verbatim.
	 * Filenames contain the peer IP, port, direction, label, UTC timestamp, and
	 * a unique suffix. The direction is "recv" or "sent", relative to
	 * Doomseeker. IPv4-mapped IPv6 peers use dotted-decimal IPv4 addresses;
	 * genuine IPv6 peers remain IPv6. Unsafe filename characters, including
	 * IPv6 colons, become underscores.
	 *
	 * For example, the Odamex plugin can configure this directory:
	 *
	 * @code
	 * PacketDumper::Options options;
	 * options.directory = QDir(PacketDumper::defaultDirectory()).filePath("p-odamex");
	 * @endcode
	 *
	 * An empty Options::directory selects defaultDirectory(). A relative path such
	 * as "p-odamex" alone is relative to the working directory, not
	 * defaultDirectory(). The directory is created if needed.
	 *
	 * An Options::label such as "decoded" can be used to distinguish processed
	 * captures from the "raw" UDP payloads.
	 *
	 * A valid Options::timestamp specifies the filename's time instant, converted
	 * to UTC. An invalid timestamp (the default) selects the current time when
	 * dump() is called.
	 *
	 * Function logs saved dump's path, or logs an error and returns an empty
	 * string.
	 *
	 * **Caller convention:** plugin captures should be saved in a subdirectory of
	 * defaultDirectory() named "p-" followed by EnginePlugin::nameCanonical(),
	 * such as "p-odamex". Captures without an identified plugin should use the
	 * "doomseeker" subdirectory. This convention is not enforced.
	 *
	 * @param address Peer IP address included in the filename. The peer is the
	 *     sender for incoming traffic and the destination for outgoing traffic.
	 * @param port Peer UDP port included in the filename.
	 * @param direction Traffic direction relative to Doomseeker.
	 * @param data Bytes saved verbatim in the dump file.
	 * @param options Extra options for storing the dump.
	 *
	 * @return Absolute path to the saved dump, or an empty string if saving fails.
	 */
	static QString dump(const QHostAddress &address, quint16 port, Direction direction,
		const QByteArray &data, const Options &options = Options());
};

#endif
