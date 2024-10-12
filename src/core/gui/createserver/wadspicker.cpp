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
#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>
#include <QIcon>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QStyle>

DClass<WadsPicker> : public Ui::WadsPicker
{
public:
	QStandardItemModel *model;
};

DPointered(WadsPicker)

WadsPicker::WadsPicker(QWidget *parent)
{
	Q_UNUSED(parent);
	d->setupUi(this);
	d->model = new QStandardItemModel(this);
	connect(d->model, &QStandardItemModel::itemChanged,
		this, &WadsPicker::checkItemPathExists);
	d->lstAdditionalFiles->setModel(d->model);
	d->lstAdditionalFiles->installEventFilter(this);

	d->btnBrowsePwad->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	d->btnClearPwadList->setIcon(Icons::clear());

	auto *checkPathsAction = new QAction(QIcon(":/icons/refresh.png"),
		WadsPicker::tr("Check paths"), this);
	d->lstAdditionalFiles->addAction(checkPathsAction);
	connect(checkPathsAction, &QAction::triggered,
		this, &WadsPicker::checkAllItemsPathsExist);
	d->lstAdditionalFiles->setContextMenuPolicy(Qt::ActionsContextMenu);
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
	QStandardItem *existing = findPath(wadPath);
	if (existing != nullptr)
	{
		// Update the path check if it's re-added.
		checkItemPathExists(existing);
	}
	else
	{
		addPathToTable(wadPath, required);
	}
}

void WadsPicker::addPathToTable(const QString &path, bool required)
{
	auto it = new QStandardItem(path);

	it->setDragEnabled(true);
	it->setDropEnabled(false);
	it->setToolTip(path);
	it->setCheckable(true);
	it->setCheckState(required ? Qt::Checked : Qt::Unchecked);
	it->setData(path, Qt::EditRole);
	checkItemPathExists(it);

	d->model->appendRow(it);
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

void WadsPicker::checkAllItemsPathsExist()
{
	for (int i = 0; i < d->model->rowCount(); ++i)
	{
		checkItemPathExists(d->model->item(i));
	}
}

void WadsPicker::checkItemPathExists(QStandardItem *item)
{
	QFont font = item->font();
	QString path = item->data(Qt::EditRole).toString().trimmed();
	QFileInfo fileInfo(gDoomseekerTemplatedPathResolver().resolve(path));
	// Squelch the model signals because this check is triggered by
	// item edit, but it also does an item edit, which triggers the
	// signal again, which causes an infinite loop. Also I **hope** this
	// will never break anything in any version of Qt because who the
	// hell knows how the internals of the model and widgets are
	// implemented and if some future widget will come to rely on these
	// signals and if they're blocked, then the changes will not be
	// displayed, even though blocking the signals on the model is the
	// way to do this as suggested by everyone on the net
	// roflmao-through-tears.
	QSignalBlocker signalBlocker(d->model);
	if (fileInfo.exists())
	{
		font.setItalic(false);
		item->setToolTip(path);
		item->setIcon(QIcon());
	}
	else
	{
		font.setItalic(true);
		item->setToolTip(QString("<font color=\"#ff0000\">")
			+ tr("%1 MISSING").arg(path)
			+ QString("</font>"));
		item->setIcon(QIcon(":/icons/exclamation_16.png"));
	}
	item->setFont(font);
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
		else if (event->type() == QEvent::Show)
		{
			checkAllItemsPathsExist();
		}
	}
	return false;
}

QList<PickedGameFile> WadsPicker::files() const
{
	QList<PickedGameFile> list;
	for (int i = 0; i < d->model->rowCount(); ++i)
	{
		auto item = d->model->item(i);
		// Filter out all the empty entries from the list.
		if (!item->data(Qt::EditRole).toString().trimmed().isEmpty())
		{
			list << PickedGameFile { item->text(), (item->checkState() == Qt::Unchecked) };
		}
	}

	return list;
}

QStandardItem *WadsPicker::findPath(const QString &path)
{
	#ifdef Q_OS_WIN32
	static const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
	#else
	static const Qt::CaseSensitivity cs = Qt::CaseSensitive;
	#endif

	for (int i = 0; i < d->model->rowCount(); ++i)
	{
		QStandardItem *item = d->model->item(i);
		if (item->text().compare(path, cs) == 0)
			return item;
	}
	return nullptr;
}

void WadsPicker::setFiles(const QList<PickedGameFile> &files)
{
	removeAll();
	for (auto file : files)
		addWadPath(file.path, !file.optional);
}

void WadsPicker::removeAll()
{
	d->model->clear();
}

void WadsPicker::removeSelected()
{
	const bool bSelectNextLowest = true;
	CommonGUI::removeSelectedRowsFromStandardItemView(d->lstAdditionalFiles, bSelectNextLowest);
}
