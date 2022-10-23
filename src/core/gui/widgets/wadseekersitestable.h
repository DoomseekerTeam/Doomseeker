//------------------------------------------------------------------------------
// wadseekersitestable.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __WADSEEKERSITESTABLE_H__
#define __WADSEEKERSITESTABLE_H__

#include "gui/widgets/tablewidgetmouseaware.h"

class WadseekerSitesTable : public TableWidgetMouseAware
{
	Q_OBJECT

public:
	static const int IDX_URL_COLUMN = 0;
	static const int IDX_PROGRESS_COLUMN = 1;
	static const int IDX_ABORT_COLUMN = 2;

	WadseekerSitesTable(QWidget *pParent = nullptr);

	void addUrl(const QUrl &url);
	void removeUrl(const QUrl &url);

public slots:
	void addService(const QString &service);
	void removeService(const QString &service);
	void setUrlProgress(const QUrl &url, qint64 current, qint64 total);

signals:
	void serviceAbortRequested(const QString &service);
	void urlAbortRequested(const QUrl &url);

protected:
	void showEvent(QShowEvent *pEvent) override;

private:
	class PrivData
	{
	public:
		bool bAlreadyShownOnce;
	};

	PrivData d;

	int findRow(const QString &text);
	int findRow(const QUrl &url);

private slots:
	void requestUrlAbort(const QString &urlAsString);
};

#endif
