//------------------------------------------------------------------------------
// application.cpp
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
#include "application.h"

#include "configuration/doomseekerconfig.h"
#include "gui/mainwindow.h"
#include <QPointer>
#include <cassert>

const QString Application::NAME = "doomseeker";

DClass<Application>
{
public:
	QPointer<MainWindow> mainWindow;
	bool running;
	bool installUpdatesAndRestart;
	QStringList originalArgs;
};

DPointered(Application)

Application *Application::staticInstance = nullptr;

Application::Application(int &argc, char **argv)
	: QApplication(argc, argv)
{
	d->mainWindow = nullptr;
	d->running = true;
	d->installUpdatesAndRestart = false;
	setApplicationName(NAME);
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
	setDesktopFileName("org.drdteam.Doomseeker");
#endif

	// http://blog.qt.io/blog/2013/04/25/retina-display-support-for-mac-os-ios-and-x11/
	setAttribute(Qt::AA_UseHighDpiPixmaps);
}


Application::~Application()
{
}

void Application::deinit()
{
	if (staticInstance != nullptr)
	{
		staticInstance->destroy();
		delete staticInstance;
		staticInstance = nullptr;
	}
}

void Application::destroy()
{
	d->running = false;
}

QIcon Application::icon()
{
	return QIcon(":/icon.png");
}

bool Application::isInit()
{
	return staticInstance != nullptr;
}

void Application::init(int &argc, char **argv)
{
	assert(staticInstance == nullptr && "Cannot initialize Application twice!");
	// The argv must captured before it's passed to Application because
	// Application calls Qt where the argc and argv may be modified.
	QStringList args;
	for (int i = 0; i < argc; ++i)
		args << argv[i];
	staticInstance = new Application(argc, argv);
	staticInstance->d->originalArgs = args;
}

Application *Application::instance()
{
	assert(staticInstance != nullptr);
	return staticInstance;
}

bool Application::isGameFileIntegrityCheckEnabled() const
{
	return gConfig.doomseeker.bCheckTheIntegrityOfWads;
}

bool Application::isInstallUpdatesAndRestart() const
{
	return d->installUpdatesAndRestart;
}

void Application::setInstallUpdatesAndRestart(bool b)
{
	d->installUpdatesAndRestart = b;
}

bool Application::isRunning() const
{
	return d->running;
}

MainWindow *Application::mainWindow() const
{
	return d->mainWindow.data();
}

QWidget *Application::mainWindowAsQWidget() const
{
	return d->mainWindow.data();
}

void Application::setMainWindow(MainWindow *mainWindow)
{
	d->mainWindow = mainWindow;
}

const QStringList &Application::originalArgs() const
{
	return d->originalArgs;
}

void Application::stopRunning()
{
	d->running = false;
}
