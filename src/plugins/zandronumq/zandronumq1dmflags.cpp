//------------------------------------------------------------------------------
// zandronumq1dmflags.cpp
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
#include "zandronumq1dmflags.h"

#include "zandronumqgamesettings.h"
#include <QMutex>
#include <serverapi/serverstructs.h>

namespace zandronumq1
{
static QList<DMFlagsSection> cachedFlags;
static QMutex cacheMutex;

DMFlagsSection Dmflags::compatFlags()
{
	using namespace ZandronumqGameSettingsNames;

	DMFlagsSection section("Compat. flags");
	section << DMFlag(COMPAT_FIND_SHORTEST_TEXTURES_LIKE_DOOM, COMPATF_SHORTTEX,
		tr("Use Doom's shortest texture behavior"));
	section << DMFlag(COMPAT_USE_BUGGIER_STAIR_BUILDING, COMPATF_STAIRINDEX,
		tr("Don't fix loop index for stair building"));
	section << DMFlag(COMPAT_LIMIT_PAIN_ELEMENTALS, COMPATF_LIMITPAIN,
		tr("Pain elemental is limited to 20 lost souls"));
	section << DMFlag(COMPAT_DONT_LET_OTHERS_HEAR_PICKUPS, COMPATF_SILENTPICKUP,
		tr("Pickups are only heard locally"));
	section << DMFlag(COMPAT_ACTORS_ARE_INFINITELY_TALL, COMPATF_NO_PASSMOBJ,
		tr("Infinitely tall actors"));
	section << DMFlag(COMPAT_ALLOW_SILENT_BFGTRICK, COMPATF_MAGICSILENCE,
		tr("Limit actors to only one sound"));
	section << DMFlag(COMPAT_ENABLE_WALL_RUNNING, COMPATF_WALLRUN,
		tr("Enable wallrunning"));
	section << DMFlag(COMPAT_SPAWN_ITEM_DROPS_ON_THE_FLOOR, COMPATF_NOTOSSDROPS,
		tr("Dropped items spawn on floor"));
	section << DMFlag(COMPAT_ALL_SPECIAL_LINES_CAN_DROP_USE_LINES, COMPATF_USEBLOCKING,
		tr("Special lines block use line"));
	section << DMFlag(COMPAT_DISABLE_BOOM_DOOR_LIGHT_EFFECT, COMPATF_NODOORLIGHT,
		tr("Disable BOOM local door light effect"));
	section << DMFlag(COMPAT_RAVENS_SCROLLERS_USE_ORIGINAL_SPEED, COMPATF_RAVENSCROLL,
		tr("Raven's scrollers use their original speed"));
	section << DMFlag(COMPAT_USE_SECTOR_BASED_SOUND_TARGET_CODE, COMPATF_SOUNDTARGET,
		tr("Use sector based sound target code"));
	section << DMFlag(COMPAT_LIMIT_DEH_MAX_HEALTH_TO_HEALTH_BONUS, COMPATF_DEHHEALTH,
		tr("Limit dehacked MaxHealth to health bonus"));
	section << DMFlag(COMPAT_TRACE_IGNORE_LINES_WITHOUT_SAME_SECTOR_ON_BOTH_SIDES, COMPATF_TRACE,
		tr("Trace ignores lines with the same sector on both sides"));
	section << DMFlag(COMPAT_NO_MONSTERS_DROPOFF_MOVE, COMPATF_DROPOFF,
		tr("Monsters can not move when hanging over a drop off"));
	section << DMFlag(COMPAT_SCROLLING_SECTORS_ARE_ADDITIVE, COMPATF_BOOMSCROLL,
		tr("Scrolling sectors are additive like Boom"));
	section << DMFlag(COMPAT_MONSTERS_SEE_SEMI_INVISIBLE_PLAYERS, COMPATF_INVISIBILITY,
		tr("Monsters can see semi-invisible players"));
	section << DMFlag(COMPAT_INSTANTLY_MOVING_FLOORS_ARENT_SILENT, COMPATF_SILENT_INSTANT_FLOORS,
		tr("Instantly moving floors are not silent"));
	section << DMFlag(COMPAT_SECTOR_SOUNDS_USE_ORIGINAL_METHOD, COMPATF_SECTORSOUNDS,
		tr("Sector sounds use original method for sound origin"));
	section << DMFlag(COMPAT_USE_ORIGINAL_MISSILE_CLIPPING_HEIGHT, COMPATF_MISSILECLIP,
		tr("Use original Doom heights for clipping against projectiles"));
	section << DMFlag(COMPAT_MONSTERS_CANT_BE_PUSHED_OFF_CLIFFS, COMPATF_CROSSDROPOFF,
		tr("Monsters can't be pushed over dropoffs"));
	section << DMFlag(COMPAT_ANY_BOSS_DEATH_ACTIVATES_MAP_SPECIALS, COMPATF_ANYBOSSDEATH,
		tr("Any monster which calls BOSSDEATH counts for level specials"));
	section << DMFlag(COMPAT_MINOTAUR, COMPATF_MINOTAUR,
		tr("Minotaur's floor flame is exploded immediately when feet are clipped"));
	section << DMFlag(COMPAT_ORIGINAL_VELOCITY_CALC_FOR_MUSHROOM_IN_DEHACKED, COMPATF_MUSHROOM,
		tr("Force original velocity calculations for A_Mushroom in Dehacked mods"));
	section << DMFlag(COMPAT_FRICTION_PUSHERS_PULLERS_AFFECT_MONSTERS, COMPATF_MBFMONSTERMOVE,
		tr("Monsters are affected by friction and pushers/pullers"));
	section << DMFlag(COMPAT_CRUSHER_GIBS_BY_MORPHING_NOT_REPLACEMENT, COMPATF_CORPSEGIBS,
		tr("Crushed monsters are turned into gibs, rather than replaced by gibs"));
	section << DMFlag(COMPAT_BLOCK_MONSTER_LINES_IGNORE_FRIENDLY_MONSTERS, COMPATF_NOBLOCKFRIENDS,
		tr("Friendly monsters aren't blocked by monster-blocking lines"));
	section << DMFlag(COMPAT_SPRITE_SORT_ORDER_INVERTED, COMPATF_SPRITESORT,
		tr("Invert sprite sorting order for sprites of equal distance"));
	section << DMFlag(COMPAT_HITSCANS_ORIGINAL_BLOCKMAP, COMPATF_HITSCAN,
		tr("Hitscans use original blockmap and hit check code"));
	section << DMFlag(COMPAT_FIND_NEIGHBORING_LIGHT_LEVEL_LIKE_DOOM, COMPATF_LIGHT,
		tr("Find neighboring light level like like Doom"));
	section << DMFlag(COMPAT_DRAW_POLYOBJECTS_OLD, COMPATF_POLYOBJ,
		tr("Draw polyobjects the old fashioned way"));
	section << DMFlag("Ignore compositing when drawing masked midtextures", COMPATF_MASKEDMIDTEX,
		tr("Ignore compositing when drawing masked midtextures"));
	return section;
}

DMFlagsSection Dmflags::compatFlags2()
{
	DMFlagsSection section("Compat. flags 2");
	section << DMFlag("It is impossible to directly face cardinal direction", COMPATF2_BADANGLES,
		tr("It is impossible to directly face cardinal direction"));
	section << DMFlag("Use the same floor motion behavior as Doom", COMPATF2_FLOORMOVE,
		tr("Use the same floor motion behavior as Doom"));
	return section;
}

DMFlagsSection Dmflags::zandronumqCompatFlags()
{
	using namespace ZandronumqGameSettingsNames;

	DMFlagsSection section("ZA Compat. flags");
	section << DMFlag(COMPAT_NETSCRIPTS_ARE_CLIENTSIDE, ZACOMPATF_NETSCRIPTS_ARE_CLIENTSIDE,
		tr("Net scripts are client side"));
	section << DMFlag(COMPAT_NO_PREDICTION_ACS, ZACOMPATF_NO_PREDICTION_ACS,
		tr("Don't execute prediction ACS on client"));
	section << DMFlag(NO_USE_LAND_CONSOLE_COMMAND, ZACOMPATF_NO_LAND,
		tr("Players can't use 'land' CCMD"));
	section << DMFlag(COMPAT_OLD_RANDOM_NUMBER_GENERATOR,
		ZACOMPATF_OLD_RANDOM_GENERATOR,
		tr("Use Doom's original random number generator"));
	section << DMFlag(COMPAT_ADD_NOGRAVITYFLAG_TO_SPHERES, ZACOMPATF_NOGRAVITY_SPHERES,
		tr("Spheres have NOGRAVITY flag"));
	section << DMFlag(DONT_STOP_PLAYER_SCRIPTS_ON_DISCONNECT,
		ZACOMPATF_DONT_STOP_PLAYER_SCRIPTS_ON_DISCONNECT,
		tr("Don't stop player scripts on disconnect"));
	section << DMFlag(OLD_ZDOOM_HORIZONTAL_THRUST,
		ZACOMPATF_OLD_EXPLOSION_THRUST,
		tr("Use horizontal explosion thrust of old ZDoom versions"));
	section << DMFlag(OLD_ZDOOM_BRIDGE_DROPS,
		ZACOMPATF_OLD_BRIDGE_DROPS,
		tr("Non-SOLID things fall through invisible bridges"));
	section << DMFlag(OLD_ZDOOM_JUMP_PHYSICS, ZACOMPATF_OLD_ZDOOM_ZMOVEMENT,
		tr("Use old ZDoom jump physics"));
	section << DMFlag(COMPAT_FULL_WEAPON_LOWER,
		ZACOMPATF_FULL_WEAPON_LOWER,
		tr("Disallow weapon change when in mid raise/lower"));
	section << DMFlag(COMPAT_USE_VANILLA_AUTOAIM_TRACER_BEHAVIOR, ZACOMPATF_AUTOAIM,
		tr("Use vanilla's autoaim tracer behavior"));
	section << DMFlag(COMPAT_WEST_SPAWNS_ARE_SILENT, ZACOMPATF_SILENT_WEST_SPAWNS,
		tr("West spawns are silent"));
	section << DMFlag(COMPAT_LIMITED_MOVEMENT_IN_THE_AIR, ZACOMPATF_LIMITED_AIRMOVEMENT,
		tr("Limited movement in the air"));
	section << DMFlag(COMPAT_PLASMA_BUMP_BUG, ZACOMPATF_PLASMA_BUMP_BUG,
		tr("Allow map01 \"plasma bump\" bug"));
	section << DMFlag(COMPAT_ALLOW_INSTANT_RESPAWN, ZACOMPATF_INSTANTRESPAWN,
		tr("Allow instant respawn after death"));
	section << DMFlag(NO_TAUNT, ZACOMPATF_DISABLETAUNTS,
		tr("Disable taunting"));
	section << DMFlag(COMPAT_ORIGINAL_SOUND_CURVE, ZACOMPATF_ORIGINALSOUNDCURVE,
		tr("Use doom2.exe's original sound curve"));
	section << DMFlag(COMPAT_USE_OLD_INTERMISSION_SCREENS_MUSIC, ZACOMPATF_OLDINTERMISSION,
		tr("Use original doom2 intermission music"));
	section << DMFlag(COMPAT_DISABLE_STEALTH_MONSTERS, ZACOMPATF_DISABLESTEALTHMONSTERS,
		tr("Disable stealth monsters"));
	section << DMFlag(COMPAT_OLD_DAMAGE_RADIUS_INFINITE_HEIGHT, ZACOMPATF_OLDRADIUSDMG,
		tr("Radius damage has infinite height"));
	section << DMFlag(NO_USE_CROSSHAIRS, ZACOMPATF_NO_CROSSHAIR,
		tr("Disable crosshair"));
	section << DMFlag(COMPAT_ORIGINAL_WEAPON_SWITCH, ZACOMPATF_OLD_WEAPON_SWITCH,
		tr("Force weapon switch"));
	section << DMFlag(COMPAT_DISABLE_CROSSHAIR_ACCURATE, ZACOMPATF_DISABLE_CROSSHAIR_ACCURATE,
		tr("Disable shooting where crosshair is"));
	section << DMFlag(COMPAT_DISABLE_WALL_FRICTION, ZACOMPATF_DISABLE_WALL_FRICTION,
		tr("Disable wall friction"));
	return section;
}

DMFlagsSection Dmflags::dmflags()
{
	DMFlagsSection section("DMFlags");
	section << DMFlag("Do not spawn health items (DM)", DF_NO_HEALTH,
		tr("Do not spawn health items (DM)"));
	section << DMFlag("Do not spawn powerups (DM)", DF_NO_ITEMS,
		tr("Do not spawn powerups (DM)"));
	section << DMFlag("Weapons remain after pickup (DM)", DF_WEAPONS_STAY,
		tr("Weapons remain after pickup (DM)"));
	section << DMFlag("Falling damage (old ZDoom)", DF_FORCE_FALLINGZD,
		tr("Falling damage (old ZDoom)"));
	section << DMFlag("Falling damage (Hexen)", DF_FORCE_FALLINGHX,
		tr("Falling damage (Hexen)"));
	section << DMFlag("Falling damage (Strife)", DF_FORCE_FALLINGST,
		tr("Falling damage (Strife)"));
	section << DMFlag("Stay on same map when someone exits (DM)", DF_SAME_LEVEL,
		tr("Stay on same map when someone exits (DM)"));
	section << DMFlag("Spawn players as far as possible (DM)", DF_SPAWN_FARTHEST,
		tr("Spawn players as far as possible (DM)"));
	section << DMFlag("Automatically respawn dead players (DM)", DF_FORCE_RESPAWN,
		tr("Automatically respawn dead players (DM)"));
	section << DMFlag("Don't spawn armor (DM)", DF_NO_ARMOR,
		tr("Don't spawn armor (DM)"));
	section << DMFlag("Kill anyone who tries to exit the level (DM)", DF_NO_EXIT,
		tr("Kill anyone who tries to exit the level (DM)"));
	section << DMFlag("Infinite ammo", DF_INFINITE_AMMO,
		tr("Infinite ammo"));
	section << DMFlag("No monsters", DF_NO_MONSTERS,
		tr("No monsters"));
	section << DMFlag("Monsters respawn", DF_MONSTERS_RESPAWN,
		tr("Monsters respawn"));
	section << DMFlag("Items other than invuln. and invis. respawn", DF_ITEMS_RESPAWN,
		tr("Items other than invuln. and invis. respawn"));
	section << DMFlag("Fast monsters", DF_FAST_MONSTERS,
		tr("Fast monsters"));
	section << DMFlag("No jumping", DF_NO_JUMP,
		tr("No jumping"));
	section << DMFlag("No freelook", DF_NO_FREELOOK,
		tr("No freelook"));
	section << DMFlag("Respawn invulnerability and invisibility", DF_RESPAWN_SUPER,
		tr("Respawn invulnerability and invisibility"));
	section << DMFlag("No multiplayer weapons in cooperative", DF_NO_COOP_WEAPON_SPAWN,
		tr("No multiplayer weapons in cooperative"));
	section << DMFlag("No crouching", DF_NO_CROUCH,
		tr("No crouching"));
	section << DMFlag("Lose all old inventory on respawn (COOP)", DF_COOP_LOSE_INVENTORY,
		tr("Lose all old inventory on respawn (COOP)"));
	section << DMFlag("Lose keys on respawn (COOP)", DF_COOP_LOSE_KEYS,
		tr("Lose keys on respawn (COOP)"));
	section << DMFlag("Lose weapons on respawn (COOP)", DF_COOP_LOSE_WEAPONS,
		tr("Lose weapons on respawn (COOP)"));
	section << DMFlag("Lose armor on respawn (COOP)", DF_COOP_LOSE_ARMOR,
		tr("Lose armor on respawn (COOP)"));
	section << DMFlag("Lose powerups on respawn (COOP)", DF_COOP_LOSE_POWERUPS,
		tr("Lose powerups on respawn (COOP)"));
	section << DMFlag("Lose ammo on respawn (COOP)", DF_COOP_LOSE_AMMO,
		tr("Lose ammo on respawn (COOP)"));
	section << DMFlag("Lose half your ammo on respawn (COOP)", DF_COOP_HALVE_AMMO,
		tr("Lose half your ammo on respawn (COOP)"));
	section << DMFlag("Jumping allowed", DF_YES_JUMP,
		tr("Jumping allowed"));
	section << DMFlag("Crouching allowed", DF_YES_CROUCH,
		tr("Crouching allowed"));
	return section;
}

DMFlagsSection Dmflags::dmflags2()
{
	DMFlagsSection section("DMFlags2");
	section << DMFlag("Drop weapons upon death", DF2_YES_WEAPONDROP,
		tr("Drop weapons upon death"));
	section << DMFlag("Don't spawn runes", DF2_NO_RUNES,
		tr("Don't spawn runes"));
	section << DMFlag("Instantly return flags (ST/CTF)", DF2_INSTANT_RETURN,
		tr("Instantly return flags (ST/CTF)"));
	section << DMFlag("Don't allow players to switch teams", DF2_NO_TEAM_SWITCH,
		tr("Don't allow players to switch teams"));
	section << DMFlag("Players are automatically assigned teams", DF2_NO_TEAM_SELECT,
		tr("Players are automatically assigned teams"));
	section << DMFlag("Double the amount of ammo given", DF2_YES_DOUBLEAMMO,
		tr("Double the amount of ammo given"));
	section << DMFlag("Players slowly lose health over 100% like Quake", DF2_YES_DEGENERATION,
		tr("Players slowly lose health over 100% like Quake"));
	section << DMFlag("Allow BFG freeaiming", DF2_YES_FREEAIMBFG,
		tr("Allow BFG freeaiming"));
	section << DMFlag("Barrels respawn", DF2_BARRELS_RESPAWN,
		tr("Barrels respawn"));
	section << DMFlag("No respawn protection", DF2_NO_RESPAWN_INVUL,
		tr("No respawn protection"));
	section << DMFlag("All players start with a shotgun", DF2_COOP_SHOTGUNSTART,
		tr("All players start with a shotgun"));
	section << DMFlag("Players respawn where they died (COOP)", DF2_SAME_SPAWN_SPOT,
		tr("Players respawn where they died (COOP)"));
	section << DMFlag("Don't clear frags after each level", DF2_YES_KEEPFRAGS,
		tr("Don't clear frags after each level"));
	section << DMFlag("Player can't respawn", DF2_NO_RESPAWN,
		tr("Player can't respawn"));
	section << DMFlag("Lose a frag when killed", DF2_YES_LOSEFRAG,
		tr("Lose a frag when killed"));
	section << DMFlag("Infinite inventory", DF2_INFINITE_INVENTORY,
		tr("Infinite inventory"));
	section << DMFlag("All monsters must be killed before exiting", DF2_KILL_MONSTERS,
		tr("All monsters must be killed before exiting"));
	section << DMFlag("Players can't see the automap", DF2_NO_AUTOMAP,
		tr("Players can't see the automap"));
	section << DMFlag("Allies can't be seen on the automap", DF2_NO_AUTOMAP_ALLIES,
		tr("Allies can't be seen on the automap"));
	section << DMFlag("You can't spy allies", DF2_DISALLOW_SPYING,
		tr("You can't spy allies"));
	section << DMFlag("Players can use chase cam", DF2_CHASECAM,
		tr("Players can use chase cam"));
	section << DMFlag("Players can't suicide", DF2_NOSUICIDE,
		tr("Players can't suicide"));
	section << DMFlag("Players can't use autoaim", DF2_NOAUTOAIM,
		tr("Players can't use autoaim"));
	section << DMFlag("Don't check ammo when switching weapons", DF2_DONTCHECKAMMO,
		tr("Don't check ammo when switching weapons"));
	section << DMFlag("Kill all monsters spawned by a boss cube when the boss dies", DF2_KILLBOSSMONST,
		tr("Kill all monsters spawned by a boss cube when the boss dies"));
	section << DMFlag("Do not count monsters in 'end level when dying' sectors towards kill count",
		DF2_NOCOUNTENDMONST,
		tr("Do not count monsters in 'end level when dying' sectors towards kill count"));
	return section;
}

DMFlagsSection Dmflags::zandronumqDmflags()
{
	DMFlagsSection section("Q-Zandronum Flags");
	section << DMFlag("Clients can't identify targets", ZADF_NO_IDENTIFY_TARGET,
		tr("Clients can't identify targets"));
	section << DMFlag("lmsspectatorsettings applied in all game modes",
		ZADF_ALWAYS_APPLY_LMS_SPECTATORSETTINGS,
		tr("lmsspectatorsettings applied in all game modes"));
	section << DMFlag("Clients can't draw coop info", ZADF_NO_COOP_INFO,
		tr("Clients can't draw coop info"));
	section << DMFlag("Unlagged is disabled", ZADF_NOUNLAGGED,
		tr("Unlagged is disabled"));
	section << DMFlag("Players don't block each other", ZADF_UNBLOCK_PLAYERS,
		tr("Players don't block each other"));
	section << DMFlag("Clients don't show medals", ZADF_NO_MEDALS,
		tr("Clients don't show medals"));
	section << DMFlag("Keys are shared between players", ZADF_SHARE_KEYS,
		tr("Keys are shared between players"));
	section << DMFlag("Player teams are preserved between maps", ZADF_YES_KEEP_TEAMS,
		tr("Player teams are preserved between maps"));
	section << DMFlag("Force OpenGL defaults", ZADF_FORCE_GL_DEFAULTS,
		tr("Force OpenGL defaults"));
	section << DMFlag("No rocket jumping", ZADF_NO_ROCKET_JUMPING,
		tr("No rocket jumping"));
	section << DMFlag("Award damage instead of kills", ZADF_AWARD_DAMAGE_INSTEAD_KILLS,
		tr("Award damage instead of kills"));
	section << DMFlag("Force drawing alpha", ZADF_FORCE_ALPHA,
		tr("Force drawing alpha"));
	section << DMFlag("Don't spawn multiplayer things", ZADF_COOP_SP_ACTOR_SPAWN,
		tr("Don't spawn multiplayer things"));
	section << DMFlag("Force blood screen brightness on clients to emulate vanilla",
		ZADF_MAX_BLOOD_SCALAR,
		tr("Force blood screen brightness on clients to emulate vanilla"));
	section << DMFlag("Teammates don't block each other", ZADF_UNBLOCK_ALLIES,
		tr("Teammates don't block each other"));
	section << DMFlag("No dropping allowed", ZADF_NODROP,
		tr("No dropping allowed"));
	section << DMFlag("No map reset on death in survival", ZADF_SURVIVAL_NO_MAP_RESET_ON_DEATH,
		tr("No map reset on death in survival"));
	section << DMFlag("Dead players can keep inventory", ZADF_DEAD_PLAYERS_CAN_KEEP_INVENTORY,
		tr("Dead players can keep inventory"));
	return section;
}

QList<DMFlagsSection> Dmflags::flags()
{
	if (cachedFlags.isEmpty())
	{
		cacheMutex.lock();
		if (cachedFlags.isEmpty())
		{
			cachedFlags << dmflags();
			cachedFlags << dmflags2();
			cachedFlags << zandronumqDmflags();
			cachedFlags << compatFlags();
			cachedFlags << zandronumqCompatFlags();
			cachedFlags << compatFlags2();
		}
		cacheMutex.unlock();
	}
	return cachedFlags;
}
}
