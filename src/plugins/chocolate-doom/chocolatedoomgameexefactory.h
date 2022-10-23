//------------------------------------------------------------------------------
// chocolatedoomgameexefactory.h
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
#ifndef id8fc2daff_3e44_42a2_8d0f_24d807b9a407
#define id8fc2daff_3e44_42a2_8d0f_24d807b9a407

#include <dptr.h>
#include <serverapi/gameexefactory.h>

class GameFile;
class GameFileList;

class ChocolateDoomGameExeFactory : public GameExeFactory
{
	Q_OBJECT
public:
	static GameFile executableForIwad(const QString &iwad);
	static GameFileList gameFiles();

	ChocolateDoomGameExeFactory(EnginePlugin *plugin);
	~ChocolateDoomGameExeFactory() override;

private:
	DPtr<ChocolateDoomGameExeFactory> d;

	GameFileList gameFiles_() const;
};

#endif
