//------------------------------------------------------------------------------
// wadspicker.h
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
#ifndef idcb0c6116_a0f7_4c45_bbe4_42e4b1944363
#define idcb0c6116_a0f7_4c45_bbe4_42e4b1944363

#include "dptr.h"

#include <QtContainerFwd>
#include <QStandardItem>
#include <QString>
#include <QWidget>

struct PickedGameFile
{
	QString path;
	bool optional = false;

	PickedGameFile(QString path = QString(), bool optional = false)
		: path(path), optional(optional) {}
};

class WadsPicker : public QWidget
{
	Q_OBJECT

public:
	WadsPicker(QWidget *parent);
	~WadsPicker() override;

	QList<PickedGameFile> files() const;
	void setFiles(const QList<PickedGameFile> &files);

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void addEmptyPath();
	void addWadPath(const QString &wadPath, bool required = true);
	void addPathToTable(const QString &path, bool required);
	void browseAndAdd();
	void checkItemPathExists(QStandardItem *item);
	void removeAll();
	void removeSelected();

private:
	DPtr<WadsPicker> d;

	void checkAllItemsPathsExist();
	/**
	 * Find path on the list or return `nullptr` if not found.
	 */
	QStandardItem *findPath(const QString &path);
};

#endif
