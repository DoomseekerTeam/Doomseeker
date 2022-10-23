//------------------------------------------------------------------------------
// createserverdialogapi.cpp
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
#include "createserverdialogapi.h"

#include "gui/createserverdialog.h"
#include <cassert>
#include <QPointer>

DClass<CreateServerDialogApi>
{
public:
	QPointer<CreateServerDialog> dialog;
};

DPointered(CreateServerDialogApi)

CreateServerDialogApi::CreateServerDialogApi(QWidget *parent)
{
	d->dialog = new CreateServerDialog(GameCreateParams::Host, parent);
}

CreateServerDialogApi::~CreateServerDialogApi()
{
	if (d->dialog)
	{
		delete d->dialog;
	}
}

CreateServerDialogApi *CreateServerDialogApi::createNew(QWidget *parent)
{
	auto api = new CreateServerDialogApi(parent);
	return api;
}

QDialog *CreateServerDialogApi::dialog() const
{
	return d->dialog;
}

bool CreateServerDialogApi::fillInCommandLineArguments(QString &executable, QStringList &args)
{
	assert(d->dialog);
	return d->dialog->commandLineArguments(executable, args, false);
}

void CreateServerDialogApi::makeRemoteGameSetup(const EnginePlugin *plugin)
{
	Q_UNUSED(plugin)
	assert(d->dialog);
	d->dialog->makeRemoteGameSetupDialog(const_cast<EnginePlugin*>(plugin));
}

void CreateServerDialogApi::setIwadByName(const QString &iwad)
{
	assert(d->dialog);
	d->dialog->setIwadByName(iwad);
}
