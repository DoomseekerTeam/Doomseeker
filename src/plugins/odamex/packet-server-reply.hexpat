// ImHex pattern file for Odamex game server reply packet.
//
// This pattern supports protocol-7 information replies, version replies, and
// old-enquirer replies. Older information reply layouts are unsupported.
//
// Odamex source version: 12.3.0
// Source references at that version:
//   server/src/sv_sqp.cpp: IntQrySendResponse(), IntQryBuildInformation()
//   common/c_cvarlist.cpp: sv_gametype
//   common/c_cvars.h: cvar_t::asInt()
//   common/g_gametype.cpp: G_IsTeamGame()
//   common/teamdef.h: team_t
//   documentation/tech/query_protocol.odt
//
// This file is to be used with ImHex editor to analyze the replies
// of the game servers.
//
// ImHex: https://imhex.werwolv.net/

#pragma endian little

#include <std/io.pat>
#include <std/math.pat>
#include <std/mem.pat>
#include <std/string.pat>

enum ResponseTag : u32 {
    VERSION = 0xAD032001,
    OLD_ENQUIRER = 0xAD032002,
    INFORMATION = 0xAD032003,
};

// Base modes from sv_gametype.
// The derived modes are selected by inspecting other CVars.
enum GameMode : u8 {
    COOPERATIVE = 0,
    DEATHMATCH = 1,
    TEAM_DEATHMATCH = 2,
    CTF = 3,
    HORDE = 4,
};

enum CVarType : u8 {
    BOOL = 1,
    BYTE = 2,
    WORD = 3,
    INT = 4,
    FLOAT = 5,
    STRING = 6,
};

struct CVar {
    char name[];
    CVarType type;

    // A BOOL entry means true and has no value bytes.
    // The server omits zero-valued numeric CVars and empty strings.
    if (type == CVarType::BYTE) {
        // The name includes the terminating NUL.
        if (std::string::substr(name, 0, sizeof(name) - 1) == "sv_gametype")
            GameMode value_byte;
        else
            u8 value_byte;
    }
    else if (type == CVarType::WORD)
        u16 value_word;
    else if (type == CVarType::INT)
        s32 value_int;
    else if (type == CVarType::FLOAT || type == CVarType::STRING)
        char value_string[]; // FLOAT is decimal text, not IEEE-754 bytes.
    else if (type != CVarType::BOOL)
        std::error("Unknown CVar type. Cannot determine the value length.");
};

// The caller uses this function for numeric CVars that determine the layout.
// Signedness depends on the CVar. BYTE and WORD use unsigned wire values.
// INT follows the server's signed int conversion, including for bitmasks.
fn numeric_cvar(auto cvar) {
    if (cvar.type == CVarType::BOOL)
        return 1;
    if (cvar.type == CVarType::BYTE)
        return u8(cvar.value_byte);
    if (cvar.type == CVarType::WORD)
        return cvar.value_word;
    if (cvar.type == CVarType::INT)
        return cvar.value_int;
    return float(std::string::parse_float(cvar.value_string));
};

fn find_numeric_cvar(auto cvars, u8 count, str name) {
    for (u32 i = 0, i < count, i += 1) {
        // ImHex's char name[] includes the terminating NUL in its value.
        if (std::string::substr(cvars[i].name, 0, sizeof(cvars[i].name) - 1) == name)
            return numeric_cvar(cvars[i]);
    }
    return 0;
};

struct BinaryHash {
    // The protocol document calls this field "Hexadecimal String".
    // The field contains binary bytes, not hex text or a NUL terminator.
    // An MD5 digest has 16 bytes. An unset password has length 0.
    u8 length;
    u8 bytes[length];
};

struct Team {
    char name[];
    u32 color;
    s16 score;
};

struct Patch {
    char filename[];
};

struct Wad {
    char filename[];
    BinaryHash md5;
};

fn format_color_argb(ref auto c) {
    return std::format("#{:02X}{:02X}{:02X}{:02X}",
                       c.alpha, c.red, c.green, c.blue);
};

// userinfo.color is ARGB; the query sends indices 3, 2, 1, 0 (BGRA).
struct ColorARGB {
    u8 blue;
    u8 green;
    u8 red;
    u8 alpha;
} [[format("format_color_argb"),
    color(std::format("{:02X}{:02X}{:02X}", red, green, blue))]];

struct Player<auto team_game> {
    char name[];
    ColorARGB color;
    if (team_game)
        u8 team; // userinfo.team uses team_t from common/teamdef.h.
    u16 ping;
    u16 minutes_in_game;
    u8 spectator; // Boolean: 0 or 1.
    s16 frags;
    u16 kills;
    u16 deaths;
};

struct Information {
    u32 server_protocol_version; // The server's supported protocol version.
    char revision[]; // Protocol 7 uses the string from NiceVersionDetails().

    u8 cvar_count;
    CVar cvars[cvar_count];

    // The current reply determines the layout. Absent numeric CVars mean zero.
    // g_lives and g_sides change mode names, but not the presence of team data.
    double game_type = find_numeric_cvar(cvars, cvar_count, "sv_gametype");
    float time_limit = find_numeric_cvar(cvars, cvar_count, "sv_timelimit");
    // G_IsTeamGame() is true for Team Deathmatch and CTF.
    // Horde and Survival Horde have no team fields.
    bool team_game = game_type == GameMode::TEAM_DEATHMATCH || game_type == GameMode::CTF;

    BinaryHash password_md5;
    char map[];
    // The server includes time_left when sv_timelimit.asInt() is nonzero.
    // asInt() uses std::round() on the float value, so 0.5 includes the field
    // and 0.1 omits the field. time_left is measured in whole minutes.
    if (std::math::round(time_limit) != 0)
        u16 time_left;

    if (team_game) {
        u8 team_count;
        Team teams[team_count];
    }

    u8 patch_count;
    Patch patches[patch_count];

    u8 wad_count;
    // Index 0 is odamex.wad. Index 1 is the IWAD. Indices 2+ are PWADs.
    Wad wads[wad_count];

    u8 player_count;
    Player<team_game> players[player_count];
};

struct Packet {
    ResponseTag tag;
    if (tag != ResponseTag::VERSION && tag != ResponseTag::OLD_ENQUIRER &&
        tag != ResponseTag::INFORMATION)
        std::error("Not a supported Odamex server-browser response tag");

    // GAMEVER = major * 256 + minor * 10 + patch.
    // The engine version is separate from the query protocol version.
    u32 engine_version;
    // INFORMATION uses the negotiated protocol version. Short replies use
    // the server's supported protocol version.
    u32 protocol_version;
    u32 token; // The server echoes the request token. Doomseeker sends 0.

    if (tag == ResponseTag::INFORMATION) {
        if (protocol_version != 7)
            std::error("Only protocol-7 information replies are supported");
        Information information;
    }
    // VERSION and OLD_ENQUIRER replies end after the token and total 16 bytes.
};

Packet packet @ 0x00;
if ($ != std::mem::size())
    std::error("Reply length mismatch: truncated data or unexplained trailing bytes");
