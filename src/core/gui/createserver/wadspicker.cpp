//------------------------------------------------------------------------------
// wadspicker.cpp
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
#include "ui_wadspicker.h"
#include "wadspicker.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "gui/commongui.h"
#include "gui/icons.h"
#include "templatedpathresolver.h"
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QStyle>

DClass<WadsPicker> : public Ui::WadsPicker
{
};

DPointered(WadsPicker)

WadsPicker::WadsPicker(QWidget *parent)
{
	Q_UNUSED(parent);
	d->setupUi(this);
	d->lstAdditionalFiles->setModel(new QStandardItemModel(this));
	d->lstAdditionalFiles->installEventFilter(this);

	d->btnBrowsePwad->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	d->btnClearPwadList->setIcon(Icons::clear());
}

WadsPicker::~WadsPicker()
{
}

void WadsPicker::addEmptyPath()
{
	addPathToTable(QString(), true);
}

void WadsPicker::addWadPath(const QString &wadPath, bool required)
{
	if (wadPath.isEmpty())
		return;
	QFileInfo fileInfo(gDoomseekerTemplatedPathResolver().resolve(wadPath));
	if (!fileInfo.isFile())
		return;

	auto model = static_cast<QStandardItemModel *>(d->lstAdditionalFiles->model());

	// Check if this path exists already, if so - do nothing.
	for (int i = 0; i < model->rowCount(); ++i)
	{
		QStandardItem *item = model->item(i);
		QString existing = item->text();
		Qt::CaseSensitivity cs;

		#ifdef Q_OS_WIN32
		cs = Qt::CaseInsensitive;
		#else
		cs = Qt::CaseSensitive;
		#endif

		if (existing.compare(wadPath, cs) == 0)
			return;
	}
	addPathToTable(wadPath, required);
}

void WadsPicker::addPathToTable(const QString &path, bool required)
{
	auto it = new QStandardItem(path);

	it->setDragEnabled(true);
	it->setDropEnabled(false);
	it->setToolTip(path);
	it->setCheckable(true);
	it->setCheckState(required ? Qt::Checked : Qt::Unchecked);

	auto model = static_cast<QStandardItemModel *>(d->lstAdditionalFiles->model());
	model->appendRow(it);
}

void WadsPicker::browseAndAdd()
{
	QString dialogDir = gConfig.doomseeker.previousCreateServerWadDir;
	QStringList filesNames = QFileDialog::getOpenFileNames(this,
		tr("Doomseeker - Add file(s)"), dialogDir);

	if (!filesNames.isEmpty())
	{
		// Remember the directory of the first file. This directory will be
		// restored the next time this dialog is opened.
		QFileInfo fi(filesNames[0]);
		gConfig.doomseeker.previousCreateServerWadDir = fi.absolutePath();

		for (const QString &strFile : filesNames)
		{
			addWadPath(gDefaultDataPaths->portablizePath(strFile));
		}
	}
}

bool WadsPicker::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == d->lstAdditionalFiles)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			switch (keyEvent->key())
			{
			case Qt::Key_Delete:
			case Qt::Key_Minus:
				removeSelected();
				return true;
			case Qt::Key_Plus:
				addEmptyPath();
				return true;
			default:
				// no-op
				break;
			}
		}
	}
	return false;
}

QList<PickedGameFile> WadsPicker::files() const
{
	QList<PickedGameFile> list;
	auto model = static_cast<QStandardItemModel *>(
		d->lstAdditionalFiles->model());
	for (int i = 0; i < model->rowCount(); ++i)
	{
		auto item = model->item(i);
		// Filter out all the empty entries from the list.
		if (!item->text().trimmed().isEmpty())
		{
			list << PickedGameFile { item->text(), (item->checkState() == Qt::Unchecked) };
		}
	}

	return list;
}

void WadsPicker::setFiles(const QList<PickedGameFile> &files)
{
	removeAll();
	for (auto file : files)
		addWadPath(file.path, !file.optional);
}

void WadsPicker::removeAll()
{
	auto pModel = (QStandardItemModel *)d->lstAdditionalFiles->model();
	pModel->clear();
}

void WadsPicker::removeSelected()
{
	const bool bSelectNextLowest = true;
	CommonGUI::removeSelectedRowsFromStandardItemView(d->lstAdditionalFiles, bSelectNextLowest);
}
