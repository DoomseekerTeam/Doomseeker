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

#include "gui/mainwindow.h"
#include <cassert>

const QString Application::NAME = "doomseeker";

DClass<Application>
{
public:
	MainWindow *mainWindow;
	bool running;
};

DPointered(Application)

Application *Application::staticInstance = nullptr;

Application::Application(int &argc, char **argv)
	: QApplication(argc, argv)
{
	d->mainWindow = nullptr;
	d->running = true;
	setApplicationName(NAME);

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
	staticInstance = new Application(argc, argv);
}

Application *Application::instance()
{
	assert(staticInstance != nullptr);
	return staticInstance;
}

bool Application::isRunning() const
{
	return d->running;
}

MainWindow *Application::mainWindow() const
{
	return d->mainWindow;
}

QWidget *Application::mainWindowAsQWidget() const
{
	return d->mainWindow;
}

void Application::setMainWindow(MainWindow *mainWindow)
{
	d->mainWindow = mainWindow;
}

void Application::stopRunning()
{
	d->running = false;
}
