//------------------------------------------------------------------------------
// filepickwidget.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "filepickwidget.h"

#include "filefilter.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "pathfinder/pathfind.h"
#include "serverapi/gamefile.h"
#include "templatedpathresolver.h"
#include "ui_filepickwidget.h"
#include <QFileDialog>
#include <QString>

DClass<FilePickWidget> : public Ui::FilePickWidget
{
public:
	bool changed;
	GameFile file;
	QSharedPointer< ::FilePickWidget::NeighbourStrategy> neighbourStrategy;
};
DPointered(FilePickWidget)

FilePickWidget::FilePickWidget(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
	d->changed = false;
	d->neighbourStrategy = QSharedPointer<NeighbourStrategy>(new NeighbourStrategy);
	d->lblWarning->hide();
	this->connect(d->lePath, SIGNAL(editingFinished()), SLOT(emitPathChangedIfChanged()));
	this->connect(d->lePath, SIGNAL(textEdited(QString)), SLOT(trackEdit()));
}

FilePickWidget::~FilePickWidget()
{
}

void FilePickWidget::browsePath()
{
	QString filepath = QFileDialog::getOpenFileName(this,
		tr("Doomseeker - choose executable file"),
		d->lePath->text(), FileFilter::executableFilesFilter());
	if (!filepath.isEmpty())
	{
		d->lePath->setText(filepath);
		emit pathChanged();
	}
}

void FilePickWidget::findPath()
{
	QFileInfo currentFile(gDoomseekerTemplatedPathResolver().resolve(path()));
	if (currentFile.isFile() && d->file.isSameFile(currentFile.fileName()))
		return;
	QString path = PathFind::findGameFile(d->neighbourStrategy->neighbours(), d->file);
	if (!path.isEmpty())
	{
		d->lePath->setText(path);
		emit pathChanged();
	}
	else
		emit findFailed();
}

bool FilePickWidget::isEmpty() const
{
	return path().trimmed().isEmpty();
}

void FilePickWidget::setFile(const GameFile &file)
{
	d->file = file;
	d->lblTitle->setText(tr("Path to %1 executable:").arg(file.niceName()));
	d->btnFind->setVisible(canSearch());
}

void FilePickWidget::setNeighbourStrategy(QSharedPointer<NeighbourStrategy> strategy)
{
	d->neighbourStrategy = strategy;
}

QString FilePickWidget::path() const
{
	return d->lePath->text().trimmed();
}

bool FilePickWidget::canSearch() const
{
	return !d->file.fileName().isEmpty();
}

void FilePickWidget::emitPathChangedIfChanged()
{
	if (d->changed)
	{
		d->changed = false;
		emit pathChanged();
	}
}

void FilePickWidget::trackEdit()
{
	d->changed = true;
}

void FilePickWidget::load(const IniSection &cfg)
{
	d->lePath->setText(cfg[d->file.configName()].valueString());
}

void FilePickWidget::save(IniSection &cfg)
{
	cfg[d->file.configName()].setValue(d->lePath->text());
}

bool FilePickWidget::validate()
{
	QString error;

	QFileInfo fileInfo(gDoomseekerTemplatedPathResolver().resolve(path()));
	if (!path().isEmpty())
	{
		if (error.isEmpty() && !fileInfo.exists())
			error = tr("File doesn't exist.");

		if (error.isEmpty() && fileInfo.isDir() && !fileInfo.isBundle())
			error = tr("This is a directory.");
	}

	d->lblWarning->setVisible(!error.isEmpty());
	d->lblWarning->setToolTip(error);
	return error.isEmpty();
}
