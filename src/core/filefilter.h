//------------------------------------------------------------------------------
// filefilter.h
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
#ifndef id0def88e2_fe74_468f_8468_2345e252e005
#define id0def88e2_fe74_468f_8468_2345e252e005

#include <QObject>
#include <QString>
#include <QStringList>

class FileFilter : public QObject
{
	Q_OBJECT

public:
	static QString allFiles();
	static QString demos();
	static QString demosWithIni();
	static QString executables();
};

class FileFilterList
{
public:
	FileFilterList(const QStringList &filters = {})
	{
		this->filters = filters;
	}

	FileFilterList(const QString &filter)
	{
		if (!filter.isEmpty())
			this->filters << filter;
	}

	static FileFilterList executableOrAll()
	{
		return FileFilterList()
			.add(FileFilter::executables())
			.add(FileFilter::allFiles());
	}

	FileFilterList &add(const QString &filter)
	{
		this->filters << filter;
		return *this;
	}

	FileFilterList &all() { return add(FileFilter::allFiles()); }

	QStringList toStringList() const
	{
		QStringList out = this->filters;
		out.removeAll(QString(""));
		out.removeDuplicates();
		return out;
	}

	QString toSemicolonFilter() const
	{
		return toStringList().join(";;");
	}

private:
	QStringList filters;
};

#endif
