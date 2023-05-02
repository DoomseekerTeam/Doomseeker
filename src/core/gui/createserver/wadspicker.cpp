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
#include "gui/commongui.h"
#include <QFileDialog>
#include <QStandardItemModel>

DClass<WadsPicker> : public Ui::WadsPicker
{
};

DPointered(WadsPicker)

WadsPicker::WadsPicker(QWidget *parent)
{
	Q_UNUSED(parent);
	d->setupUi(this);
	d->lstAdditionalFiles->setModel(new QStandardItemModel(this));
}

WadsPicker::~WadsPicker()
{
}

void WadsPicker::addWadPath(const QString &wadPath, bool required)
{
	if (wadPath.isEmpty())
		return;
	QFileInfo fileInfo(wadPath);
	if (!fileInfo.isFile())
		return;

	auto model = static_cast<QStandardItemModel *>(d->lstAdditionalFiles->model());

	// Check if this path exists already, if so - do nothing.
	for (int i = 0; i < model->rowCount(); ++i)
	{
		QStandardItem *item = model->item(i);
		QString dir = item->text();
		Qt::CaseSensitivity cs;

		#ifdef Q_OS_WIN32
		cs = Qt::CaseInsensitive;
		#else
		cs = Qt::CaseSensitive;
		#endif

		if (dir.compare(wadPath, cs) == 0)
			return;
	}

	auto it = new QStandardItem(wadPath);

	it->setDragEnabled(true);
	it->setDropEnabled(false);
	it->setToolTip(wadPath);
	it->setCheckable(true);
	it->setCheckState(required ? Qt::Checked : Qt::Unchecked);

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
			addWadPath(strFile);
		}
	}
}

QList<bool> WadsPicker::fileOptional() const
{
	QList<bool> checked = CommonGUI::listViewStandardItemsToBoolList(d->lstAdditionalFiles);
	// We check required WADs and want to return optional wads.
	for (bool &i : checked)
		i = !i;
	return checked;
}

QStringList WadsPicker::filePaths() const
{
	return CommonGUI::listViewStandardItemsToStringList(d->lstAdditionalFiles);
}

void WadsPicker::setFilePaths(const QStringList &paths, const QList<bool> &optionals)
{
	removeAll();
	for (int i = 0; i < paths.size(); ++i)
		addWadPath(paths[i], i >= optionals.size() || !optionals[i]);
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
