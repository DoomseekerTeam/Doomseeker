//------------------------------------------------------------------------------
// demometadatadialog.cpp
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "demometadatadialog.h"

#include "gamedemo.h"
#include "gui/commongui.h"
#include "plugins/engineplugin.h"
#include "serverapi/serverstructs.h"

#include "ui_demometadatadialog.h"

#include <QKeyEvent>
#include <QStandardItemModel>

DClass<DemoMetaDataDialog> : public Ui::DemoMetaDataDialog
{
public:
	GameDemo demo;
	QStandardItemModel *wadsModel;
};

DPointeredNoCopy(DemoMetaDataDialog);

DemoMetaDataDialog::DemoMetaDataDialog(QWidget *parent, const GameDemo &demo)
	: QDialog(parent)
{
	d->setupUi(this);
	d->demo = demo;

	d->cboGame->setAllowUnknown(true);

	d->wadsModel = new QStandardItemModel(this);
	d->wadsList->setModel(d->wadsModel);
	d->wadsList->installEventFilter(this);

	d->cboGame->setPluginByName(demo.game);
	d->dteCreatedTime->setDateTime(demo.time.isValid() ? demo.time : QDateTime::currentDateTime());
	d->leIwad->setText(demo.iwad);

	for (const PWad &wad : demo.wads)
		addWad(wad);
}

void DemoMetaDataDialog::addEmptyWad()
{
	addWad(PWad(""));
}

void DemoMetaDataDialog::addWad(const PWad &wad)
{
	auto it = new QStandardItem(wad.name());
	it->setDragEnabled(true);
	it->setCheckable(true);
	it->setCheckState(wad.isOptional() ? Qt::Unchecked : Qt::Checked);
	d->wadsModel->appendRow(it);
}

bool DemoMetaDataDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == d->wadsList)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			switch (keyEvent->key())
			{
			case Qt::Key_Delete:
			case Qt::Key_Minus:
				removeSelectedWads();
				return true;
			case Qt::Key_Plus:
				addEmptyWad();
				return true;
			default:
				// no-op
				break;
			}
		}
	}
	return false;
}

GameDemo DemoMetaDataDialog::gameDemo() const
{
	EnginePlugin *game = d->cboGame->currentPlugin();
	const QString gameName = game != nullptr
		? game->data()->name
		: d->demo.game;

	GameDemo demo;
	demo.demopath = d->demo.demopath;
	demo.game = gameName;
	demo.time = d->dteCreatedTime->dateTime();
	demo.iwad = d->leIwad->text();
	for (int i = 0; i < d->wadsModel->rowCount(); ++i)
	{
		auto item = d->wadsModel->item(i);
		QString filename = item->text().trimmed();
		if (filename.isEmpty())
			continue;

		const bool optional = item->checkState() == Qt::Unchecked;
		demo.wads << PWad(filename, optional);
	}
	return demo;
}

void DemoMetaDataDialog::removeSelectedWads()
{
	const bool bSelectNextLowest = true;
	CommonGUI::removeSelectedRowsFromStandardItemView(d->wadsList, bSelectNextLowest);
}
