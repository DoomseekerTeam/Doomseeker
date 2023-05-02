//------------------------------------------------------------------------------
// testfilesearchpath.cpp
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
// Copyright (C) 2023 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "testfilesearchpath.h"

#include "pathfinder/filesearchpath.h"

#define ASSERT(x) { if (!(x)) { testLog << "Failed: ASSERT(" #x ")"; return false; }}
#define ASSERT_FALSE(x) { if (x) { testLog << "Failed: ASSERT_FALSE(" #x ")"; return false;}}

bool TestFileSearchPathContains::executeTest()
{
	FileSearchPath simplePath("avo/ca");
	ASSERT(simplePath.contains("avo/ca"));
	ASSERT(simplePath.contains("avo/ca/"));
	ASSERT(simplePath.contains("avo//ca//"));
	ASSERT_FALSE(simplePath.contains("avo/c"));
	ASSERT_FALSE(simplePath.contains("avo/cado"));
	ASSERT_FALSE(simplePath.contains("avo/ca/do"));
	ASSERT_FALSE(simplePath.contains("avo"));
	#ifdef Q_OS_WIN
	ASSERT(simplePath.contains("avo\\ca"));
	ASSERT(simplePath.contains("avo\\ca\\"));
	ASSERT_FALSE(simplePath.contains("avo\\ca\\do"));
	ASSERT_FALSE(simplePath.contains("avo\\cado"));
	#endif

	FileSearchPath recursivePath("avo/ca");
	recursivePath.setRecursive(true);
	ASSERT(recursivePath.contains("avo/ca"));
	ASSERT(recursivePath.contains("avo/ca/"));
	ASSERT(recursivePath.contains("avo//ca//"));
	ASSERT_FALSE(recursivePath.contains("avo/c"));
	ASSERT_FALSE(recursivePath.contains("avo/cado"));
	ASSERT(recursivePath.contains("avo/ca/do"));
	ASSERT_FALSE(recursivePath.contains("avo"));
	#ifdef Q_OS_WIN
	ASSERT(recursivePath.contains("avo\\ca"));
	ASSERT(recursivePath.contains("avo\\ca\\"));
	ASSERT(recursivePath.contains("avo\\ca\\do"));
	ASSERT_FALSE(recursivePath.contains("avo\\cado"));
	#endif

	return true;
}

bool TestFileSearchPathMerge::executeTest()
{
	// Given
	QList<FileSearchPath> paths;
	paths << FileSearchPath("avo//ca");
	paths << FileSearchPath("avo/ca/");
	paths << FileSearchPath("avo/ca/do");
	paths << FileSearchPath("ba/na/na");
	paths << []() { FileSearchPath p("ba/na"); p.setRecursive(true); return p; }();
	paths << []() { FileSearchPath p("ap"); p.setRecursive(true); return p; }();
	paths << []() { FileSearchPath p("ap/ple"); p.setRecursive(true); return p; }();
	paths << FileSearchPath("recuA");
	paths << []() { FileSearchPath p("recuA"); p.setRecursive(true); return p; }();
	paths << []() { FileSearchPath p("recuB"); p.setRecursive(true); return p; }();
	paths << FileSearchPath("recuB");

	// When
	FileSearchPath::merge(paths);

	// Then
	testLog << QString("paths.length() == %1").arg(paths.length());
	for (int idx = 0; idx < paths.length(); ++idx)
	{
		testLog << QString("paths[%1]: %2%3")
			.arg(idx)
			.arg(paths[idx].path())
			.arg(paths[idx].isRecursive() ? " (R)" : "");
	}
	ASSERT(paths.length() == 6);
	ASSERT(paths[0].path() == "avo/ca/");
	ASSERT_FALSE(paths[0].isRecursive());
	ASSERT(paths[1].path() == "avo/ca/do");
	ASSERT_FALSE(paths[1].isRecursive());
	ASSERT(paths[2].path() == "ba/na");
	ASSERT(paths[2].isRecursive());
	ASSERT(paths[3].path() == "ap");
	ASSERT(paths[3].isRecursive());
	ASSERT(paths[4].path() == "recuA");
	ASSERT(paths[4].isRecursive());
	ASSERT(paths[5].path() == "recuB");
	ASSERT(paths[5].isRecursive());

	return true;
}
