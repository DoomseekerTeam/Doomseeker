//------------------------------------------------------------------------------
// gamehost.h
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idABA35264_C99B_49FA_BEDEDB4F5978C7EC
#define idABA35264_C99B_49FA_BEDEDB4F5978C7EC

#include "dptr.h"
#include "global.h"
#include "serverapi/polymorphism.h"
#include <QObject>
#include <QStringList>

class CommandLineInfo;
class EnginePlugin;
class GameCreateParams;
class Message;

/**
 * @ingroup group_pluginapi
 * @brief Creates game servers, offline games or demo playbacks.
 *
 * GameHost launches games from the perspective of the "server admin"
 * or "game master" or "local player" (whatever is appropriate in
 * given circumstances). It works in close union with GameCreateParams
 * which basically tells GameHost how to configure the game. This
 * structure can be accessed at any time with params() method.
 *
 * Game launch command line is built by sequentially executing add*()
 * methods. Each of these methods is allowed to set error and bail
 * early by setting appropriate Message with setMessage()
 * method. Reference to command line arguments list can be accessed
 * with args(). This points to a QStringList that can be manipulated
 * freely.
 *
 * Some configuration settings are universal between Doom game
 * engines. Default GameHost implementation will try to handle them,
 * so plugins don't need to build everything from scratch. Sometimes
 * different engines handle the same type of configuration using
 * different argument. This can include operations like loading PWADs
 * or dehacked files, or playing back a demo. You can customize this
 * argument by setting args returned by arg*() methods. Each arg*()
 * method has an equivalent setArg*() method. For example,
 * argForPort() has setArgForPort(). The best place to call setArg*()
 * methods and configure these properties is the constructor of the
 * subclass defined in the plugin.
 *
 * Configuration settings that aren't universal need to be added by
 * overriding addExtra() method.
 */
class MAIN_EXPORT GameHost : public QObject
{
	Q_OBJECT

public:
	GameHost(EnginePlugin *plugin);
	virtual ~GameHost() override;

	/**
	 * @return Message::isError() == false if command line was
	 *         successfully created.
	 */
	Message createHostCommandLine(const GameCreateParams &params, CommandLineInfo &cmdLine);

	/**
	 * @see createHostCommandLine()
	 */
	Message host(const GameCreateParams &params);

	/**
	 * @brief EnginePlugin that this GameHost is associated with.
	 */
	EnginePlugin *plugin() const;

protected:
	/**
	 * @brief Command line parameter that is used to load a BEX file.
	 *
	 * Default: "-deh".
	 */
	const QString &argForBexLoading() const;

	/**
	 * @brief Command line parameter that is used to load a DEHACKED file.
	 *
	 * Default: "-deh".
	 */
	const QString &argForDehLoading() const;

	/**
	 * @brief Command line parameter that is used to set IWAD.
	 *
	 * Default: "-iwad".
	 */
	const QString &argForIwadLoading() const;

	/**
	 * @brief Command line parameter that is used to load optional WADs.
	 *
	 * Default: "-file".
	 */
	const QString &argForOptionalWadLoading() const;

	/**
	 * @brief Command line parameter that is used to set network port for
	 * the game.
	 *
	 * Default: "-port".
	 */
	const QString &argForPort() const;

	/**
	 * @brief Command line parameter that is used to load a PWAD.
	 *
	 * Default: "-file".
	 */
	const QString &argForPwadLoading() const;

	/**
	 * @brief Command line parameter for playing back a demo.
	 *
	 * Default: "-playdemo".
	 */
	const QString &argForDemoPlayback() const;
	/**
	 * @brief Command line parameter for recording a demo.
	 *
	 * Default: "-record";
	 */
	const QString &argForDemoRecord() const;

	/**
	 * @brief Command line parameter used to launch a server.
	 *
	 * No default.
	 */
	const QString &argForServerLaunch() const;

	/**
	 * @brief Reference to command line arguments.
	 *
	 * This is where plugins should write all CMD line arguments they
	 * create for the executable run.
	 */
	QStringList &args();

	/**
	 * @brief "Custom parameters" are specified directly by user
	 * in "Create Game" dialog box.
	 */
	virtual void addCustomParameters();

	/**
	 * @brief @b [Virtual] Creates engine specific command line parameters
	 *        out of passed DM flags list.
	 *
	 * Default behavior does nothing.
	 */
	void addDMFlags();
	POLYMORPHIC_SETTER_DECLARE(void, GameHost, addDMFlags, ())
	void addDMFlags_default();

	/**
	 * @brief Creates engine specific command line parameters out of
	 *        Server class fields.
	 *
	 * Following settings are already set by default implementation of
	 * createHostCommandLine() and don't need any additional handling:
	 *
	 * - Calls to all other add*() methods.
	 * - Network port on which the server is hosted (if any is hosted).
	 * - GameCVars passed from CreateServerDialog.
	 *   GameCVar objects are interpreted as 'argument - value' pairs.
	 * - Demo record or playback.
	 *
	 * @see createHostCommandLine() - cvars parameter.
	 */
	virtual void addExtra();

	/**
	 * @brief @b [Virtual] Adds custom parameters as defined in
	 * Doomseeker's configuration box.
	 *
	 * These parameters are added at the very beginning of the
	 * command line.
	 */
	void addGlobalGameCustomParameters();
	POLYMORPHIC_SETTER_DECLARE(void, GameHost, addGlobalGameCustomParameters, ())
	void addGlobalGameCustomParameters_default();

	/**
	 * @brief @b [Virtual] Loads IWAD.
	 */
	void addIwad();
	POLYMORPHIC_SETTER_DECLARE(void, GameHost, addIwad, ())
	void addIwad_default();

	/**
	 * @brief @b [Virtual] Loads PWADs and other mod files
	 * (dehacked patches, pk3s, etc.)
	 */
	void addPwads();
	POLYMORPHIC_SETTER_DECLARE(void, GameHost, addPwads, ())
	void addPwads_default();
	/**
	 * @brief See: GameClientRunner::addModFiles_prefixOnce()
	 */
	void addPwads_prefixOnce();

	/**
	 * @brief Builds command line arguments sequentially by
	 * calling other methods.
	 *
	 * This can be overridden if the default behavior does the
	 * completely wrong thing. In most cases however this method
	 * should be left untouched and appropriate add*() methods
	 * should be overridden instead, or appropriate arg*()
	 * properties should be configured.
	 */
	virtual void createCommandLineArguments();

	/**
	 * @brief GameCreateParams with which this game should be
	 * configured.
	 */
	const GameCreateParams &params() const;

	void setArgForBexLoading(const QString &arg);
	void setArgForDehLoading(const QString &arg);
	void setArgForIwadLoading(const QString &arg);
	void setArgForOptionalWadLoading(const QString &arg);
	void setArgForPort(const QString &arg);
	void setArgForPwadLoading(const QString &arg);
	void setArgForDemoPlayback(const QString &arg);
	void setArgForDemoRecord(const QString &arg);
	void setArgForServerLaunch(const QString &arg);

	/**
	 * @brief Call this method to convey errors.
	 *
	 * GameHost checks for errors before making certain steps. If
	 * plugins want to prevent execution of the game, they should
	 * set a Message instance that will return 'true' on Message::isError().
	 */
	void setMessage(const Message &message);

	/**
	 * @brief Verifies if all `params().pwadsPaths()` exist.
	 *
	 * If at least one path points to a non-existing file, an error message
	 * is set through setMessage() and false is returned.
	 * If all paths are fine, true is returned.
	 *
	 * Called by addPwads_default() and addPwads_prefixOnce().
	 */
	bool verifyPwadPaths();

private:
	DPtr<GameHost> d;

	Q_DISABLE_COPY(GameHost)

	void addDemoPlaybackIfApplicable();
	void addDemoRecordIfApplicable();
	QString fileLoadingPrefix(int index) const;
	void saveDemoMetaData();
	void setupGamePaths();
};

#endif
