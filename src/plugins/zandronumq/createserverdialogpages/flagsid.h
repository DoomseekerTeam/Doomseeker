//------------------------------------------------------------------------------
// flagsid.h
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id0bc86735_418c_47d6_a765_374dfc391a5e
#define id0bc86735_418c_47d6_a765_374dfc391a5e

#include <dptr.h>

class FlagsPage;
class IniSection;

class FlagsId
{
public:
	FlagsId(FlagsPage *flagsPage);
	~FlagsId();

	void assign();
	void save(IniSection &cfg);
	void load(const IniSection &cfg);

private:
	DPtr<FlagsId> d;
};

#endif
