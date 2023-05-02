//------------------------------------------------------------------------------
// cfgfilepaths.cpp
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "cfgfilepaths.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "pathfinder/filesearchpath.h"
#include "templatedpathresolver.h"
#include "ui_cfgfilepaths.h"
#include <QApplication>
#include <QFileDialog>
#include <QStyle>

static const int COL_PATH = 0;
static const int COL_RECURSE = 1;
static const int NUM_COLS = 2;

DClass<CFGFilePaths> : public Ui::CFGFilePaths
{
public:
	struct SavedState
	{
		bool bResolveTemplatedPathsPlaceholders;
	} savedState;

	void resetSavedState()
	{
		savedState.bResolveTemplatedPathsPlaceholders = gConfig.doomseeker.bResolveTemplatedPathsPlaceholders;
	}
};

DPointered(CFGFilePaths)

CFGFilePaths::CFGFilePaths(QWidget *parent)
	: ConfigPage(parent)
{
	set_reject(&CFGFilePaths::reject_);
	d->setupUi(this);

	QStringList labels;
	labels << CFGFilePaths::tr("Path") << CFGFilePaths::tr("Recurse");
	d->tblFilePaths->setColumnCount(NUM_COLS);
	d->tblFilePaths->setHorizontalHeaderLabels(labels);

	QHeaderView *header = d->tblFilePaths->horizontalHeader();
	header->setSectionResizeMode(COL_PATH, QHeaderView::Stretch);
	header->setSectionResizeMode(COL_RECURSE, QHeaderView::ResizeToContents);

	QString templatedPathsToolTip = QString("<p>%1</p>").arg(CFGFilePaths::tr(
		"Doomseeker supports special characters in user-configurable "
		"paths. This affects all paths everywhere, not only the file "
		"paths configured on this page. The placeholders are:"));
	templatedPathsToolTip += "<ul>";

	templatedPathsToolTip += QString("<li>%1</li>").arg(CFGFilePaths::tr(
		"<b>~</b> - if at the beginning of the path, resolved to "
		"the home directory of the current user"));
#ifdef DOOMSEEKER_TEMPLATED_PATH_RESOLVER_TILDEUSER
	templatedPathsToolTip += QString("<li>%1</li>").arg(CFGFilePaths::tr(
		"<b>~<i>user</i></b> - if at the beginning of the path, resolved to "
		"the home directory of the specified <i>user</i>"));
#endif
	templatedPathsToolTip += QString("<li>%1</li>").arg(CFGFilePaths::tr(
		"<b>$PROGDIR</b> - resolved to the directory where Doomseeker's "
		"executable is located; this may create invalid paths if "
		"used in the middle of the path"));
	templatedPathsToolTip += QString("<li>%1</li>").arg(CFGFilePaths::tr(
		"<b>$<i>NAME</i></b> - any <i>NAME</i> is resolved to the environment "
		"variable of the same <i>NAME</i>, or to empty if the environment "
		"variable is absent"));
	templatedPathsToolTip += "</ul>";

	templatedPathsToolTip += QString("<p>%1</p>").arg(CFGFilePaths::tr(
		"This option controls whether the <b>$PROGDIR</b> "
		"and <b>$<i>NAME</i></b> will be resolved or ignored. "
		"If this option is <b>checked</b>, both are resolved as explained. "
		"If this option is <b>unchecked</b>, both will be taken literally. "
		"The <b>~</b> placeholders are always resolved regardless."));

	d->cbResolveTemplatedPathsPlaceholders->setToolTip(templatedPathsToolTip);
	d->btnBrowseWadPath->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));

	connect(d->btnAddWadPath, SIGNAL(clicked()), this, SLOT(btnAddWadPath_Click()));
	connect(d->btnBrowseWadPath, &QPushButton::clicked, this, &CFGFilePaths::btnBrowseWadPath_Click);
	connect(d->btnRemoveWadPath, SIGNAL(clicked()), this, SLOT(btnRemoveWadPath_Click()));
	this->connect(d->tblFilePaths->itemDelegate(),
		SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
		SIGNAL(validationRequested()));
	connect(d->cbResolveTemplatedPathsPlaceholders, &QAbstractButton::toggled,
		this, &CFGFilePaths::cbResolveTemplatedPathsPlaceholders_Toggled);
	d->tblFilePaths->connect(d->tblFilePaths,
		&TableWidgetReorderable::rowsReordered,
		&QTableWidget::resizeRowsToContents);
}

CFGFilePaths::~CFGFilePaths()
{
}

void CFGFilePaths::addPath(const FileSearchPath &fileSearchPath)
{
	if (fileSearchPath.isValid() && !isPathAlreadyDefined(fileSearchPath.path()))
		addPathToTable(fileSearchPath);
}

void CFGFilePaths::addPathToTable(const FileSearchPath &fileSearchPath)
{
	d->tblFilePaths->setSortingEnabled(false);
	int newRow = d->tblFilePaths->rowCount();
	d->tblFilePaths->insertRow(newRow);
	auto path = new QTableWidgetItem(fileSearchPath.path());
	path->setData(Qt::ToolTipRole, fileSearchPath.path());
	auto recurse = new QTableWidgetItem();
	recurse->setCheckState(fileSearchPath.isRecursive() ? Qt::Checked : Qt::Unchecked);
	recurse->setData(Qt::TextAlignmentRole, Qt::AlignCenter);

	d->tblFilePaths->setItem(newRow, COL_PATH, path);
	d->tblFilePaths->setItem(newRow, COL_RECURSE, recurse);
	d->tblFilePaths->resizeRowsToContents();
	d->tblFilePaths->setSortingEnabled(true);
}

void CFGFilePaths::btnAddWadPath_Click()
{
	addPathToTable(QString());
	emit validationRequested();
}

void CFGFilePaths::btnBrowseWadPath_Click()
{
	QString strDir = QFileDialog::getExistingDirectory(this, tr("Doomseeker - Add game mod path"));
	addPath(gDefaultDataPaths->portablizePath(strDir));
	emit validationRequested();
}

void CFGFilePaths::btnRemoveWadPath_Click()
{
	QSet<int> uniqueRows;
	for (auto *item : d->tblFilePaths->selectedItems())
		uniqueRows.insert(item->row());
	QList<int> rows = uniqueRows.values();
	// Reverse the order of rows.
	std::sort(rows.begin(), rows.end(), [](int a, int b) { return b < a; });
	for (int row : rows)
		d->tblFilePaths->removeRow(row);
	emit validationRequested();
}

void CFGFilePaths::cbResolveTemplatedPathsPlaceholders_Toggled(bool state)
{
	gConfig.doomseeker.bResolveTemplatedPathsPlaceholders = state;
	emit validationRequested();
}

QIcon CFGFilePaths::icon() const
{
	return style()->standardIcon(QStyle::SP_DirOpenIcon);
}

bool CFGFilePaths::isPathAlreadyDefined(const QString &path) const
{
	Qt::CaseSensitivity caseSensitivity;

	#ifdef Q_OS_WIN32
	caseSensitivity = Qt::CaseInsensitive;
	#else
	caseSensitivity = Qt::CaseSensitive;
	#endif

	for (int i = 0; i < d->tblFilePaths->rowCount(); ++i)
	{
		const QTableWidgetItem *item = d->tblFilePaths->item(i, COL_PATH);
		QString dir = item->text();

		if (dir.compare(path, caseSensitivity) == 0)
			return true;
	}

	return false;
}

void CFGFilePaths::readSettings()
{
	d->resetSavedState();

	const QList<FileSearchPath> &wadPaths = gConfig.doomseeker.wadPaths;
	for (int i = 0; i < wadPaths.count(); ++i)
		addPath(wadPaths[i]);

	d->cbTellMeWhereAreMyWads->setChecked(gConfig.doomseeker.bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn);
	d->cbCheckTheIntegrityOfWads->setChecked(gConfig.doomseeker.bCheckTheIntegrityOfWads);
	d->cbResolveTemplatedPathsPlaceholders->setChecked(gConfig.doomseeker.bResolveTemplatedPathsPlaceholders);
}

void CFGFilePaths::saveSettings()
{
	QList<FileSearchPath> wadPaths;

	for (int i = 0; i < d->tblFilePaths->rowCount(); ++i)
	{
		QTableWidgetItem *itemPath = d->tblFilePaths->item(i, COL_PATH);
		QTableWidgetItem *itemRecurse = d->tblFilePaths->item(i, COL_RECURSE);
		FileSearchPath fileSearchPath(itemPath->text());
		fileSearchPath.setRecursive(itemRecurse->checkState() == Qt::Checked);
		wadPaths << fileSearchPath;
	}

	gConfig.doomseeker.wadPaths = wadPaths;
	gConfig.doomseeker.bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn = d->cbTellMeWhereAreMyWads->isChecked();
	gConfig.doomseeker.bCheckTheIntegrityOfWads = d->cbCheckTheIntegrityOfWads->isChecked();
	gConfig.doomseeker.bResolveTemplatedPathsPlaceholders = d->cbResolveTemplatedPathsPlaceholders->isChecked();

	d->resetSavedState();
}

void CFGFilePaths::reject_()
{
	gConfig.doomseeker.bResolveTemplatedPathsPlaceholders = d->savedState.bResolveTemplatedPathsPlaceholders;
}

ConfigPage::Validation CFGFilePaths::validate()
{
	bool allPathsValid = true;
	for (int i = 0; i < d->tblFilePaths->rowCount(); ++i)
	{
		QTableWidgetItem *itemPath = d->tblFilePaths->item(i, COL_PATH);

		QString validationError = validatePath(gDoomseekerTemplatedPathResolver().resolve(itemPath->text()));
		bool valid = validationError.isEmpty();
		allPathsValid = allPathsValid && valid;

		itemPath->setIcon(valid ? QIcon() : QIcon(":/icons/exclamation_16.png"));
		itemPath->setToolTip(validationError);
	}
	return allPathsValid ? VALIDATION_OK : VALIDATION_ERROR;
}

QString CFGFilePaths::validatePath(const QString &path) const
{
	if (path.trimmed().isEmpty())
		return tr("Path empty.");

	QFileInfo fileInfo(path.trimmed());
	if (!fileInfo.exists())
		return tr("Path doesn't exist.");

	if (!fileInfo.isDir())
		return tr("Path is not a directory.");

	return QString();
}
