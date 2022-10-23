//------------------------------------------------------------------------------
// application.h
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
#ifndef id5E252D92_36B9_40C2_964DBFD7D0E99622
#define id5E252D92_36B9_40C2_964DBFD7D0E99622

#include "dptr.h"
#include "global.h"

#include <QApplication>
#include <QIcon>

#define gApp (Application::instance())

class MainWindow;

/**
 * @ingroup group_pluginapi
 * @brief Program central hub of information.
 *
 * Accessors provide plugins with certain objects and data that is not
 * encapsulated in any singleton. Calling any non-const method from
 * plugin perspective may result in undefined behavior of the program
 * or even other plugins.
 */
class MAIN_EXPORT Application : public QApplication
{
public:
	/**
	 * @brief Program name - doomseeker.
	 */
	static const QString NAME;

	/**
	 * @brief Deinitializes the program; executed when program is shutting
	 *        down.
	 *
	 * Doesn't delete the actual instance() but calls destroy()
	 * method.
	 *
	 * That way certain data fields can still be accessed after the
	 * program is deinitialized. Actual instance is never deleted
	 * explicitly, and OS will clear the memory anyway.
	 */
	static void deinit();
	static bool isInit();
	static void init(int &argc, char **argv);
	static Application *instance();
	static QIcon icon();

	virtual ~Application() override;

	/**
	 * @brief Plugins and other threads can use this to figure out
	 *        if program is closing.
	 *
	 * Threads should exit their loops and let the program close gracefully.
	 */
	bool isRunning() const;

	/**
	 * @brief MainWindow of the program.
	 *
	 * Might be nullptr if current run doesn't create a MainWindow.
	 */
	MainWindow *mainWindow() const;
	/**
	 * @brief Returns MainWindow as a QWidget.
	 *
	 * Useful for plugins that need to specify parent widget for dialog
	 * boxes or such, as MainWindow class itself is not exported.
	 *
	 * Might be nullptr if current run doesn't create a MainWindow.
	 */
	QWidget *mainWindowAsQWidget() const;

	void setMainWindow(MainWindow *mainWindow);
	/**
	 * @brief Makes isRunning() return false.
	 *
	 * Called when program is shutting down.
	 */
	void stopRunning();

private:
	DPtr<Application> d;

	static Application *staticInstance;

	Application(int &argc, char **argv);
	void destroy();
};


#endif
