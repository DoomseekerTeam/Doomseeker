//------------------------------------------------------------------------------
// zandronumqgamesettings.h
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
#ifndef id50f6b4ec_858f_41ef_bb82_4892db6d51b3
#define id50f6b4ec_858f_41ef_bb82_4892db6d51b3

#include <dptr.h>

#include <QString>

namespace ZandronumqGameSettingsNames
{
extern const QString COMPAT_ALLOW_SILENT_BFGTRICK;
extern const QString COMPAT_ENABLE_WALL_RUNNING;
extern const QString COMPAT_DONT_LET_OTHERS_HEAR_PICKUPS;
extern const QString COMPAT_MONSTERS_SEE_SEMI_INVISIBLE_PLAYERS;
extern const QString COMPAT_NO_MONSTERS_DROPOFF_MOVE;
extern const QString COMPAT_SCROLLING_SECTORS_ARE_ADDITIVE;
extern const QString COMPAT_INSTANTLY_MOVING_FLOORS_ARENT_SILENT;
extern const QString COMPAT_SECTOR_SOUNDS_USE_ORIGINAL_METHOD;
extern const QString COMPAT_ANY_BOSS_DEATH_ACTIVATES_MAP_SPECIALS;
extern const QString COMPAT_FRICTION_PUSHERS_PULLERS_AFFECT_MONSTERS;
extern const QString COMPAT_CRUSHER_GIBS_BY_MORPHING_NOT_REPLACEMENT;
extern const QString COMPAT_BLOCK_MONSTER_LINES_IGNORE_FRIENDLY_MONSTERS;
extern const QString COMPAT_FIND_NEIGHBORING_LIGHT_LEVEL_LIKE_DOOM;
extern const QString COMPAT_FIND_SHORTEST_TEXTURES_LIKE_DOOM;
extern const QString COMPAT_USE_BUGGIER_STAIR_BUILDING;
extern const QString COMPAT_LIMIT_PAIN_ELEMENTALS;
extern const QString COMPAT_ACTORS_ARE_INFINITELY_TALL;
extern const QString COMPAT_SPAWN_ITEM_DROPS_ON_THE_FLOOR;
extern const QString COMPAT_ALL_SPECIAL_LINES_CAN_DROP_USE_LINES;
extern const QString COMPAT_DISABLE_BOOM_DOOR_LIGHT_EFFECT;
extern const QString COMPAT_RAVENS_SCROLLERS_USE_ORIGINAL_SPEED;
extern const QString COMPAT_USE_SECTOR_BASED_SOUND_TARGET_CODE;
extern const QString COMPAT_LIMIT_DEH_MAX_HEALTH_TO_HEALTH_BONUS;
extern const QString COMPAT_TRACE_IGNORE_LINES_WITHOUT_SAME_SECTOR_ON_BOTH_SIDES;
extern const QString COMPAT_MONSTERS_CANT_BE_PUSHED_OFF_CLIFFS;
extern const QString COMPAT_USE_ORIGINAL_MISSILE_CLIPPING_HEIGHT;
extern const QString COMPAT_MINOTAUR;
extern const QString COMPAT_ORIGINAL_VELOCITY_CALC_FOR_MUSHROOM_IN_DEHACKED;
extern const QString COMPAT_SPRITE_SORT_ORDER_INVERTED;
extern const QString COMPAT_HITSCANS_ORIGINAL_BLOCKMAP;
extern const QString COMPAT_DRAW_POLYOBJECTS_OLD;
extern const QString NO_TAUNT;
extern const QString NO_USE_CROSSHAIRS;
extern const QString NO_USE_LAND_CONSOLE_COMMAND;
extern const QString COMPAT_PLASMA_BUMP_BUG;
extern const QString COMPAT_ORIGINAL_WEAPON_SWITCH;
extern const QString COMPAT_LIMITED_MOVEMENT_IN_THE_AIR;
extern const QString COMPAT_ALLOW_INSTANT_RESPAWN;
extern const QString COMPAT_USE_OLD_INTERMISSION_SCREENS_MUSIC;
extern const QString COMPAT_DISABLE_STEALTH_MONSTERS;
extern const QString COMPAT_OLD_DAMAGE_RADIUS_INFINITE_HEIGHT;
extern const QString COMPAT_OLD_RANDOM_NUMBER_GENERATOR;
extern const QString COMPAT_NETSCRIPTS_ARE_CLIENTSIDE;
extern const QString COMPAT_NO_PREDICTION_ACS;
extern const QString COMPAT_ADD_NOGRAVITYFLAG_TO_SPHERES;
extern const QString DONT_STOP_PLAYER_SCRIPTS_ON_DISCONNECT;
extern const QString OLD_ZDOOM_HORIZONTAL_THRUST;
extern const QString OLD_ZDOOM_BRIDGE_DROPS;
extern const QString OLD_ZDOOM_JUMP_PHYSICS;
extern const QString COMPAT_FULL_WEAPON_LOWER;
extern const QString COMPAT_ORIGINAL_SOUND_CURVE;
extern const QString COMPAT_USE_VANILLA_AUTOAIM_TRACER_BEHAVIOR;
extern const QString COMPAT_WEST_SPAWNS_ARE_SILENT;
extern const QString COMPAT_MASKED_MIDTEX;
extern const QString COMPAT_BAD_ANGLES;
extern const QString COMPAT_DISABLE_CROSSHAIR_ACCURATE;
extern const QString COMPAT_DISABLE_WALL_FRICTION;
extern const QString COMPAT_FLOOR_MOVE;
extern const QString NO_MONSTERS;
extern const QString ITEMS_RESPAWN;
extern const QString MEGA_POWERUPS_RESPAWN;
extern const QString NO_USE_FREELOOK;
extern const QString INFINITE_AMMO;
extern const QString DONT_SPAWN_DEATHMATCH_WEAPONS;
extern const QString MONSTERS_ARE_FAST;
extern const QString MONSTERS_RESPAWN;
extern const QString LOSE_ALL_INVENTORY;
extern const QString LOSE_ARMOR;
extern const QString LOSE_KEYS;
extern const QString LOSE_POWERUPS;
extern const QString LOSE_WEAPONS;
extern const QString LOSE_ALL_AMMO;
extern const QString LOSE_HALF_AMMO;
extern const QString RESPAWN_AUTOMATICALLY;
extern const QString RESPAWN_FARTHEST_AWAY_FROM_OTHERS;
extern const QString WEAPONS_STAY_AFTER_PICKUP;
extern const QString DONT_SPAWN_HEALTH;
extern const QString DONT_SPAWN_ARMOR;
extern const QString BARRELS_RESPAWN;
extern const QString SERVER_PICKS_TEAMS;
extern const QString PLAYERS_CANT_SWITCH_TEAMS;
extern const QString KEEP_TEAMS_AFTER_AMAP_CHANGE;
extern const QString HIDE_ALLIES_ON_THE_AUTOMAP;
extern const QString DONT_LET_PLAYERS_SPY_ON_ALLIES;
extern const QString INSTANT_FLAG_SKULL_RETURN;
extern const QString NO_SUICIDE;
extern const QString NO_RESPAWN;
extern const QString NO_USE_AUTOMAP;
extern const QString NO_USE_AUTOAIM;
extern const QString INFINITE_INVENTORY;
extern const QString SLOWLY_LOSE_HEALTH_WHEN_OVER100;
extern const QString CAN_USE_CHASECAM;
extern const QString ALLOW_BFGFREEAIMING;
extern const QString DONT_CHECK_AMMO_WHEN_SWITCHING_WEAPONS;
extern const QString MONSTERS_MUST_BE_KILLED_TO_EXIT;
extern const QString KILL_BOSS_MONSTERS;
extern const QString RESPAWN_WHERE_DIED;
extern const QString RESPAWN_WITH_ASHOTGUN;
extern const QString DROP_WEAPON_ON_DEATH;
extern const QString LOSE_AFRAG_ON_DEATH;
extern const QString NO_RESPAWN_PROTECTION;
extern const QString KEEP_FRAGS_AFTER_MAP_CHANGE;
extern const QString DOUBLE_AMMO;
extern const QString DONT_SPAWN_RUNES;
extern const QString NO_UNLAGGED;
extern const QString ALWAYS_APPLY_LMSSPECTATOR_SETTINGS;
extern const QString NO_MEDALS;
extern const QString SCORE_DAMAGE_NOT_KILLS;
extern const QString DONT_SPAWN_ANY_MULTIPLAYER_ITEM;
extern const QString NO_TARGET_IDENTIFY;
extern const QString NO_DISPLAY_COOP_INFO;
extern const QString NO_MAX_BLOOD_SCALAR;
extern const QString NO_ITEM_DROP;
extern const QString NO_ROCKET_JUMP;
extern const QString NO_USE_CUSTOM_GLLIGHTING_SETTINGS;
extern const QString NO_TURN_OFF_TRANSLUCENCY;
extern const QString SHARE_KEYS;
extern const QString SURVIVAL_NO_MAP_RESET_ON_DEATH;
extern const QString DEAD_PLAYERS_CAN_KEEP_INVENTORY;
extern const QString LMSCHAINSAW;
extern const QString LMSPISTOL;
extern const QString LMSSHOTGUN;
extern const QString LMSSUPER_SHOTGUN;
extern const QString LMSCHAINGUN;
extern const QString LMSMINIGUN;
extern const QString LMSROCKET_LAUNCHER;
extern const QString LMSGRENADE_LAUNCHER;
extern const QString LMSPLASMA_RIFLE;
extern const QString LMSRAILGUN;
extern const QString LMSSPECTATORS_CAN_TALK_TO_ACTIVE_PLAYERS;
extern const QString LMSSPECTATORS_CAN_VIEW_THE_GAME;
}


class ZandronumqGameSettings
{
public:
	ZandronumqGameSettings();
	~ZandronumqGameSettings();

private:
	DPtr<ZandronumqGameSettings> d;
};

#endif
