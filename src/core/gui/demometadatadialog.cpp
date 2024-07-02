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

DClass<DemoMetadataDialog> : public Ui::DemoMetadataDialog
{
public:
	GameDemo demo;
	QStandardItemModel *model;
};

DPointeredNoCopy(DemoMetadataDialog);

DemoMetadataDialog::DemoMetadataDialog(QWidget *parent, const GameDemo &demo)
	: QDialog(parent)
{
	d->setupUi(this);
	d->demo = demo;

	d->cboGame->setAllowUnknown(true);

	d->model = new QStandardItemModel(this);
	d->modFilesList->setModel(d->model);
	d->modFilesList->installEventFilter(this);

	d->cboGame->setPluginByName(demo.port);
	d->dteCreatedTime->setDateTime(demo.time);
	d->leIwad->setText(demo.iwad);

	for (const PWad &modFile : demo.wads)
		addModFile(modFile);
}

void DemoMetadataDialog::addEmptyModFile()
{
	addModFile(PWad(""));
}

void DemoMetadataDialog::addModFile(const PWad &modFile)
{
	auto it = new QStandardItem(modFile.name());
	it->setDragEnabled(true);
	it->setCheckable(true);
	it->setCheckState(modFile.isOptional() ? Qt::Unchecked : Qt::Checked);
	d->model->appendRow(it);
}

bool DemoMetadataDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == d->modFilesList)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			switch (keyEvent->key())
			{
			case Qt::Key_Delete:
			case Qt::Key_Minus:
				removeSelectedModFiles();
				return true;
			case Qt::Key_Plus:
				addEmptyModFile();
				return true;
			default:
				// no-op
				break;
			}
		}
	}
	return false;
}

GameDemo DemoMetadataDialog::gameDemo() const
{
	EnginePlugin *game = d->cboGame->currentPlugin();
	const QString gameName = game != nullptr
		? game->data()->name
		: d->demo.port;

	GameDemo demo;
	demo.filepath = d->demo.filepath;
	demo.port = gameName;
	demo.time = d->dteCreatedTime->dateTime();
	demo.iwad = d->leIwad->text();
	for (int i = 0; i < d->model->rowCount(); ++i)
	{
		auto item = d->model->item(i);
		QString filename = item->text().trimmed();
		if (filename.isEmpty())
			continue;

		const bool optional = item->checkState() == Qt::Unchecked;
		demo.wads << PWad(filename, optional);
	}
	return demo;
}

void DemoMetadataDialog::removeSelectedModFiles()
{
	const bool bSelectNextLowest = true;
	CommonGUI::removeSelectedRowsFromStandardItemView(d->modFilesList, bSelectNextLowest);
}
