//------------------------------------------------------------------------------
// filefilter.cpp
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
#include "filefilter.h"

#include "gamedemo.h"

QString FileFilter::allFiles()
{
	return tr("All files (*)");
}

QString FileFilter::demos()
{
	QStringList demoFilters = DemoStore::demoFileFilters();
	return tr("Demos (%1)").arg(demoFilters.join(" "));
}

QString FileFilter::demosWithIni()
{
	QStringList demoFilters = DemoStore::demoFileFilters();
	demoFilters << "*.ini";
	demoFilters.removeDuplicates();
	return tr("Demos (%1)").arg(demoFilters.join(" "));
}

QString FileFilter::executables()
{
	#if defined(Q_OS_WIN32)
	return tr("Executables (*.exe *.bat *.com)");
	#else
	// Other platforms do not have an extension for their binary files.
	return allFiles();
	#endif
}
