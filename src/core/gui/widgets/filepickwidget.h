//------------------------------------------------------------------------------
// filepickwidget.h
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
#ifndef id36ca3802_cdbb_4c12_b4fe_eafa3bc1f790
#define id36ca3802_cdbb_4c12_b4fe_eafa3bc1f790

#include "dptr.h"
#include <QSharedPointer>
#include <QWidget>

class GameFile;
class IniSection;

class FilePickWidget : public QWidget
{
	Q_OBJECT

public:
	class NeighbourStrategy
	{
	public:
		virtual ~NeighbourStrategy() {}
		virtual QStringList neighbours()
		{
			return QStringList();
		}
	};


	FilePickWidget(QWidget *parent);
	~FilePickWidget() override;

	bool isEmpty() const;
	void setFile(const GameFile &file);
	void setNeighbourStrategy(QSharedPointer<NeighbourStrategy> strategy);
	QString path() const;

	void load(const IniSection &cfg);
	void save(IniSection &cfg);
	bool validate();

public slots:
	void findPath();

signals:
	void findFailed();
	void pathChanged();

private:
	DPtr<FilePickWidget> d;

	bool canSearch() const;

private slots:
	void browsePath();
	void emitPathChangedIfChanged();
	void trackEdit();
};

#endif
