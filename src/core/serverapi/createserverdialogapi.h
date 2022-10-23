//------------------------------------------------------------------------------
// createserverdialogapi.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idebec7725_01df_4b56_a803_07fb69a062ad
#define idebec7725_01df_4b56_a803_07fb69a062ad

#include "dptr.h"
#include "global.h"
#include <QDialog>

class EnginePlugin;

/**
 * @ingroup group_pluginapi
 * @brief Plugin proxy for CreateServerDialog.
 *
 * Spawns a new CreateServerDialog object with a parent specified in the
 * constructor. The dialog can be accessed through dialog() accessor, although
 * the method returns a pointer to QDialog only as this is the highest-level
 * class that is still accessible by plugins. All CreateServerDialog methods
 * need to be accessed through methods of this proxy class.
 *
 * Deleting this proxy object will also delete underlying CreateServerDialog.
 * This object cannot be copied.
 */
class MAIN_EXPORT CreateServerDialogApi
{
public:
	/**
	 * @brief Constructs a new CreateServerDialog that is deleted when the API
	 * object is deleted.
	 */
	static CreateServerDialogApi *createNew(QWidget *parent);

	virtual ~CreateServerDialogApi();

	/**
	 * @brief Accesses underlying CreateServerDialog as a QDialog.
	 *
	 * You can call here any methods that you'd call on a QDialog: show(),
	 * exec(), setAttribute(), and so on.
	 */
	QDialog *dialog() const;
	/**
	 * @brief When QDialog is closed with 'accepted' state, this method
	 * will build appropriate command line.
	 *
	 * @param executable
	 *     Game executable as set by the dialog.
	 * @param args
	 *     All arguments determined by the dialog.
	 */
	bool fillInCommandLineArguments(QString &executable, QStringList &args);
	/**
	 * @brief Configures the dialog box for a setup of a remote game.
	 *
	 * Certain games provide remote servers that can be configured before
	 * the game is started. By calling this method you disable all widgets
	 * that would modify networking parameters. Also, the game executable
	 * is by default set to "client" instead of "server".
	 */
	void makeRemoteGameSetup(const EnginePlugin *plugin);

	/**
	 * @brief Attempt to chose default IWAD by name.
	 *
	 * Attempts to set the path to the IWAD. It might be unsuccessful
	 * if IWAD cannot be found in any directory that was preconfigured
	 * by the user. In this case the method fails silently and does
	 * nothing.
	 */
	void setIwadByName(const QString &iwad);

private:
	Q_DISABLE_COPY(CreateServerDialogApi)

	DPtr<CreateServerDialogApi> d;

	CreateServerDialogApi(QWidget *parent);
};

#endif
