//------------------------------------------------------------------------------
// packet-server-reply.hexpat
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------

// ImHex pattern file for Zandronum Game server reply packet
//
// This file is to be used with ImHex editor to analyze the replies
// of the game servers.
//
// This doesn't handle the Zandronum 3.2's segmented packets. It assumes
// that the segments were already reassembled into the full packet.
//
// See: https://imhex.werwolv.net/

#pragma endian little

bitfield SQFlag {
    SQF_NAME : 1;
    SQF_URL : 1;
    SQF_EMAIL : 1;
    SQF_MAPNAME : 1;
    SQF_MAXCLIENTS : 1;
    SQF_MAXPLAYERS : 1;
    SQF_PWADS : 1;
    SQF_GAMETYPE : 1;
    SQF_GAMENAME : 1;
    SQF_IWAD : 1;
    SQF_FORCEPASSWORD : 1;
    SQF_FORCEJOINPASSWORD : 1;
    SQF_GAMESKILL : 1;
    SQF_BOTSKILL : 1;
    SQF_DMFLAGS : 1; // Deprecated
    reserved0 : 1;
    SQF_LIMITS : 1;
    SQF_TEAMDAMAGE : 1;
    SQF_TEAMSCORES : 1; // Deprecated
    SQF_NUMPLAYERS : 1;
    SQF_PLAYERDATA : 1;
    SQF_TEAMINFO_NUMBER : 1;
    SQF_TEAMINFO_NAME : 1;
    SQF_TEAMINFO_COLOR : 1;
    SQF_TEAMINFO_SCORE : 1;
    SQF_TESTING_SERVER : 1;
    SQF_DATA_MD5SUM : 1; // Deprecated
    SQF_ALL_DMFLAGS : 1;
    SQF_SECURITY_SETTINGS : 1;
    SQF_OPTIONAL_WADS : 1;
    SQF_DEH : 1;
    SQF_EXTENDED_INFO : 1;
};

bitfield SQFlag2 {
    SQF2_PWAD_HASHES : 1;
    SQF2_COUNTRY : 1;
    SQF2_GAMEMODE_NAME : 1;
    SQF2_GAMEMODE_SHORTNAME : 1;
    SQF2_VOICECHAT : 1;
    reserved0 : 32 - 5;
};

enum GameMode : u8 {
    COOPERATIVE = 0,
    SURVIVAL = 1,
    INVASION = 2,
    DEATHMATCH = 3,
    TEAMPLAY = 4,
    DUEL = 5,
    TERMINATOR = 6,
    LASTMANSTANDING = 7,
    TEAMLMS = 8,
    POSSESSION = 9,
    TEAMPOSSESSION = 10,
    TEAMGAME = 11,
    CTF = 12,
    ONEFLAGCTF = 13,
    SKULLTAG = 14,
    DOMINATION = 15,
};

struct Player<auto flags> {
    char name[];
    u16 points;
    u16 ping;
    bool spectator;
    bool bot;
    if (flags.SQF_TEAMINFO_NUMBER)
        u8 team;
    u8 minutes;
};

struct PacketHeader {
    u32 time;
    char gameversion[];
    SQFlag flags;
};

struct Str {
    char sz[];
};

struct SQF<auto flags> {
    if (flags.SQF_NAME)
        char name[];
    if (flags.SQF_URL)
        char url[];
    if (flags.SQF_EMAIL)
        char email[];
    if (flags.SQF_MAPNAME)
        char mapname[];
    if (flags.SQF_MAXCLIENTS)
        u8 maxclients;
    if (flags.SQF_MAXPLAYERS)
        u8 maxplayers;
    if (flags.SQF_PWADS)
    {
        u8 numpwads;
        Str pwads[numpwads];
    }
    if (flags.SQF_GAMETYPE)
    {
        GameMode gametype;
        bool instagib;
        bool buckshot;
    }
    if (flags.SQF_GAMENAME)
        char gamename[];
    if (flags.SQF_IWAD)
        char iwad[];
    if (flags.SQF_FORCEPASSWORD)
        bool password;
    if (flags.SQF_FORCEJOINPASSWORD)
        bool joinpassword;
    if (flags.SQF_GAMESKILL)
        u8 gameskill;
    if (flags.SQF_BOTSKILL)
        u8 botskill;
    if (flags.SQF_DMFLAGS)
    {
        u32 dmflags1;
        u32 dmflags2;
        u32 compatflags;
    }
    if (flags.SQF_LIMITS)
    {
        u16 fraglimit;
        u16 timelimit;
        if (timelimit > 0)
            u16 timeleft;
        u16 duellimit;
        u16 pointlimit;
        u16 winlimit;
    }
    if (flags.SQF_TEAMDAMAGE)
        float teamdamage;
    if (flags.SQF_TEAMSCORES)
    {
        u16 bluescore;
        u16 redscore;
    }
    if (flags.SQF_NUMPLAYERS)
        u8 numplayers;
    // SQF_PLAYERDATA is as independent as any other flag, but without
    // SQF_NUMPLAYERS, we don't know how many PLAYERDATA structs there
    // is, and without SQF_GAMETYPE we don't know if this is a team game
    // or not.
    if (flags.SQF_PLAYERDATA)
    {
        Player<flags> players[numplayers];
    }
    if (flags.SQF_TEAMINFO_NUMBER)
    {
        u8 numteams;
        if (flags.SQF_TEAMINFO_NAME)
            Str teamname[numteams];
        if (flags.SQF_TEAMINFO_COLOR)
            u32 teamcolor[numteams];
        if (flags.SQF_TEAMINFO_SCORE)
            u16 teamscore[numteams];
    }
    if (flags.SQF_TESTING_SERVER)
    {
        bool istesting;
        char testingarchive[];
    }
    if (flags.SQF_DATA_MD5SUM)
        char md5sum[];
    if (flags.SQF_ALL_DMFLAGS)
    {
        u8 numdmflags;
        u32 dmflags[numdmflags];
    }
    if (flags.SQF_SECURITY_SETTINGS)
        bool masterbanlist;
    if (flags.SQF_OPTIONAL_WADS)
    {
        u8 numoptional;
        u8 optionalindices[numoptional];
    }
    if (flags.SQF_DEH)
    {
        u8 numdeh;
        Str dehs[numdeh];
    }
};

struct SQF2<auto flags> {
    if (flags.SQF2_PWAD_HASHES)
    {
        u8 nwadhashes;
        Str wadhashes[nwadhashes];
    }
    if (flags.SQF2_COUNTRY)
        char country[3];
    if (flags.SQF2_GAMEMODE_NAME)
        char gamemodename[];
    if (flags.SQF2_GAMEMODE_SHORTNAME)
        char gamemodeshortname[];
    if (flags.SQF2_VOICECHAT)
        bool voicechat;
};

struct Packet {
    PacketHeader header;

    SQF<header.flags> sqf;
    if (header.flags.SQF_EXTENDED_INFO)
    {
        SQFlag2 sqf2flags;
        SQF2<sqf2flags> sqf2;
    }
};

Packet packet @0x00;
