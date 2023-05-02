//------------------------------------------------------------------------------
// checkwadsdlg.cpp
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------
#include "checkwadsdlg.h"
#include "configuration/doomseekerconfig.h"
#include "gui/commongui.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "serverapi/serverstructs.h"
#include <cassert>
#include <QScopedPointer>
#include <QTimer>

DClass<CheckWadsDlg> : public Ui::CheckWadsDlg
{
public:
	QList<PWad> wads;
	const PathFinder *pathFinder;
	CheckResult wadsChecked;
	QScopedPointer<QTimer> checkWadTimer;
	QScopedPointer<QTimer> showWindow;
	bool checkIncompatibility;
};

DPointeredNoCopy(CheckWadsDlg)

CheckWadsDlg::CheckWadsDlg(const PathFinder *pathFinder, QWidget *parent) : QDialog(parent)
{
	assert(pathFinder != nullptr);
	d->pathFinder = pathFinder;
	d->checkIncompatibility = gConfig.doomseeker.bCheckTheIntegrityOfWads;
	d->checkWadTimer.reset(new QTimer);
	d->showWindow.reset(new QTimer);

	d->setupUi(this);
	CommonGUI::setupDialog(*this);
	connect(d->checkWadTimer.data(), SIGNAL(timeout()), this, SLOT(performCheckStep()));
	connect(d->showWindow.data(), SIGNAL(timeout()), this, SLOT(openWindow()));
	connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(abortCheck()));
}

CheckWadsDlg::~CheckWadsDlg()
{
}

void CheckWadsDlg::addWads(const QList<PWad> &wads)
{
	d->wads << wads;
	d->progressBar->setMaximum(d->progressBar->value() + wads.size());
}

const CheckResult CheckWadsDlg::checkWads()
{
	d->showWindow->setSingleShot(true);
	d->showWindow->start(500);
	d->checkWadTimer->start(0);

	// we'll wait for the checking to finish.
	QEventLoop loop;
	connect(this, SIGNAL(finishedChecking()), &loop, SLOT(quit()));
	loop.exec();

	d->showWindow->stop();
	close();
	return d->wadsChecked;
}

void CheckWadsDlg::openWindow()
{
	open();
}

void CheckWadsDlg::performCheckStep()
{
	if (d->wads.isEmpty())
	{
		d->checkWadTimer->stop();
		emit finishedChecking();
		return;
	}

	const PWad wad = d->wads.first();
	d->wads.removeFirst();
	WadPathFinder wadFinder(*d->pathFinder);
	WadFindResult findResult = wadFinder.find(wad.name());

	if (!findResult.isValid())
		d->wadsChecked.missingWads << wad;
	else if (d->checkIncompatibility && !wad.validFile(findResult.path()))
		d->wadsChecked.incompatibleWads << wad;
	else
		d->wadsChecked.foundWads << wad;

	d->progressBar->setValue(d->progressBar->value() + 1);
}

void CheckWadsDlg::abortCheck()
{
	d->checkIncompatibility = false;
}
