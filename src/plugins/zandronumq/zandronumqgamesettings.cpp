//------------------------------------------------------------------------------
// zandronumqgamesettings.cpp
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
#include "zandronumqgamesettings.h"

namespace ZandronumqGameSettingsNames
{
const QString COMPAT_ALLOW_SILENT_BFGTRICK = "CompatAllowSilentBFGTrick";
const QString COMPAT_ENABLE_WALL_RUNNING = "CompatEnableWallRunning";
const QString COMPAT_DONT_LET_OTHERS_HEAR_PICKUPS = "CompatDontLetOthersHearPickups";
const QString COMPAT_MONSTERS_SEE_SEMI_INVISIBLE_PLAYERS = "CompatMonstersSeeSemiInvisiblePlayers";
const QString COMPAT_NO_MONSTERS_DROPOFF_MOVE = "CompatNoMonstersDropoffMove";
const QString COMPAT_SCROLLING_SECTORS_ARE_ADDITIVE = "CompatScrollingSectorsAreAdditive";
const QString COMPAT_INSTANTLY_MOVING_FLOORS_ARENT_SILENT = "CompatInstantlyMovingFloorsArentSilent";
const QString COMPAT_SECTOR_SOUNDS_USE_ORIGINAL_METHOD = "CompatSectorSoundsUseOriginalMethod";
const QString COMPAT_ANY_BOSS_DEATH_ACTIVATES_MAP_SPECIALS = "CompatAnyBossDeathActivatesMapSpecials";
const QString COMPAT_FRICTION_PUSHERS_PULLERS_AFFECT_MONSTERS = "CompatFrictionPushersPullersAffectMonsters";
const QString COMPAT_CRUSHER_GIBS_BY_MORPHING_NOT_REPLACEMENT = "CompatCrusherGibsByMorphingNotReplacement";
const QString COMPAT_BLOCK_MONSTER_LINES_IGNORE_FRIENDLY_MONSTERS = "CompatBlockMonsterLinesIgnoreFriendlyMonsters";
const QString COMPAT_FIND_NEIGHBORING_LIGHT_LEVEL_LIKE_DOOM = "CompatFindNeighboringLightLevelLikeDoom";
const QString COMPAT_FIND_SHORTEST_TEXTURES_LIKE_DOOM = "CompatFindShortestTexturesLikeDoom";
const QString COMPAT_USE_BUGGIER_STAIR_BUILDING = "CompatUseBuggierStairBuilding";
const QString COMPAT_LIMIT_PAIN_ELEMENTALS = "CompatLimitPainElementals";
const QString COMPAT_ACTORS_ARE_INFINITELY_TALL = "CompatActorsAreInfinitelyTall";
const QString COMPAT_SPAWN_ITEM_DROPS_ON_THE_FLOOR = "CompatSpawnItemDropsOnTheFloor";
const QString COMPAT_ALL_SPECIAL_LINES_CAN_DROP_USE_LINES = "CompatAllSpecialLinesCanDropUseLines";
const QString COMPAT_DISABLE_BOOM_DOOR_LIGHT_EFFECT = "CompatDisableBoomDoorLightEffect";
const QString COMPAT_RAVENS_SCROLLERS_USE_ORIGINAL_SPEED = "CompatRavensScrollersUseOriginalSpeed";
const QString COMPAT_USE_SECTOR_BASED_SOUND_TARGET_CODE = "CompatUseSectorBasedSoundTargetCode";
const QString COMPAT_LIMIT_DEH_MAX_HEALTH_TO_HEALTH_BONUS = "CompatLimitDehMaxHealthToHealthBonus";
const QString COMPAT_TRACE_IGNORE_LINES_WITHOUT_SAME_SECTOR_ON_BOTH_SIDES = "CompatTraceIgnoreLinesWithoutSameSectorOnBothSides";
const QString COMPAT_MONSTERS_CANT_BE_PUSHED_OFF_CLIFFS = "CompatMonstersCantBePushedOffCliffs";
const QString COMPAT_USE_ORIGINAL_MISSILE_CLIPPING_HEIGHT = "CompatUseOriginalMissileClippingHeight";
const QString COMPAT_MINOTAUR = "CompatMinotaur";
const QString COMPAT_ORIGINAL_VELOCITY_CALC_FOR_MUSHROOM_IN_DEHACKED = "CompatOriginalVelocityCalcForMushroomInDehacked";
const QString COMPAT_SPRITE_SORT_ORDER_INVERTED = "CompatSpriteSortOrderInverted";
const QString COMPAT_HITSCANS_ORIGINAL_BLOCKMAP = "CompatHitscansOriginalBlockmap";
const QString COMPAT_DRAW_POLYOBJECTS_OLD = "CompatDrawPolyobjectsOld";
const QString NO_TAUNT = "NoTaunt";
const QString NO_USE_CROSSHAIRS = "NoUseCrosshairs";
const QString NO_USE_LAND_CONSOLE_COMMAND = "NoUseLandConsoleCommand";
const QString COMPAT_PLASMA_BUMP_BUG = "CompatPlasmaBumpBug";
const QString COMPAT_ORIGINAL_WEAPON_SWITCH = "CompatOriginalWeaponSwitch";
const QString COMPAT_LIMITED_MOVEMENT_IN_THE_AIR = "CompatLimitedMovementInTheAir";
const QString COMPAT_ALLOW_INSTANT_RESPAWN = "CompatAllowInstantRespawn";
const QString COMPAT_USE_OLD_INTERMISSION_SCREENS_MUSIC = "CompatUseOldIntermissionScreensMusic";
const QString COMPAT_DISABLE_STEALTH_MONSTERS = "CompatDisableStealthMonsters";
const QString COMPAT_OLD_DAMAGE_RADIUS_INFINITE_HEIGHT = "CompatOldDamageRadiusInfiniteHeight";
const QString COMPAT_OLD_RANDOM_NUMBER_GENERATOR = "CompatOldRandomNumberGenerator";
const QString COMPAT_NETSCRIPTS_ARE_CLIENTSIDE = "CompatNETScriptsAreClientside";
const QString COMPAT_NO_PREDICTION_ACS = "CompatNoPredictionACS";
const QString COMPAT_ADD_NOGRAVITYFLAG_TO_SPHERES = "CompatAddNOGRAVITYFlagToSpheres";
const QString DONT_STOP_PLAYER_SCRIPTS_ON_DISCONNECT = "DontStopPlayerScriptsOnDisconnect";
const QString OLD_ZDOOM_HORIZONTAL_THRUST = "OldZDoomHorizontalThrust";
const QString OLD_ZDOOM_BRIDGE_DROPS = "OldZDoomBridgeDrops";
const QString OLD_ZDOOM_JUMP_PHYSICS = "OldZDoomJumpPhysics";
const QString COMPAT_FULL_WEAPON_LOWER = "CompatFullWeaponLower";
const QString COMPAT_ORIGINAL_SOUND_CURVE = "CompatOriginalSoundCurve";
const QString COMPAT_USE_VANILLA_AUTOAIM_TRACER_BEHAVIOR = "CompatUseVanillaAutoaimTracerBehavior";
const QString COMPAT_WEST_SPAWNS_ARE_SILENT = "CompatWestSpawnsAreSilent";
const QString COMPAT_MASKED_MIDTEX = "CompatMaskedMidtex";
const QString COMPAT_BAD_ANGLES = "CompatBadAngles";
const QString COMPAT_DISABLE_CROSSHAIR_ACCURATE = "CompatDisableCrosshairAccurate";
const QString COMPAT_DISABLE_WALL_FRICTION = "CompatDisableWallFriction";
const QString COMPAT_FLOOR_MOVE = "CompatFloorMove";
const QString NO_MONSTERS = "NoMonsters";
const QString ITEMS_RESPAWN = "ItemsRespawn";
const QString MEGA_POWERUPS_RESPAWN = "MegaPowerupsRespawn";
const QString NO_USE_FREELOOK = "NoUseFreelook";
const QString INFINITE_AMMO = "InfiniteAmmo";
const QString DONT_SPAWN_DEATHMATCH_WEAPONS = "DontSpawnDeathmatchWeapons";
const QString MONSTERS_ARE_FAST = "MonstersAreFast";
const QString MONSTERS_RESPAWN = "MonstersRespawn";
const QString LOSE_ALL_INVENTORY = "LoseAllInventory";
const QString LOSE_ARMOR = "LoseArmor";
const QString LOSE_KEYS = "LoseKeys";
const QString LOSE_POWERUPS = "LosePowerups";
const QString LOSE_WEAPONS = "LoseWeapons";
const QString LOSE_ALL_AMMO = "LoseAllAmmo";
const QString LOSE_HALF_AMMO = "LoseHalfAmmo";
const QString RESPAWN_AUTOMATICALLY = "RespawnAutomatically";
const QString RESPAWN_FARTHEST_AWAY_FROM_OTHERS = "RespawnFarthestAwayFromOthers";
const QString WEAPONS_STAY_AFTER_PICKUP = "WeaponsStayAfterPickup";
const QString DONT_SPAWN_HEALTH = "DontSpawnHealth";
const QString DONT_SPAWN_ARMOR = "DontSpawnArmor";
const QString BARRELS_RESPAWN = "BarrelsRespawn";
const QString SERVER_PICKS_TEAMS = "ServerPicksTeams";
const QString PLAYERS_CANT_SWITCH_TEAMS = "PlayersCantSwitchTeams";
const QString KEEP_TEAMS_AFTER_AMAP_CHANGE = "KeepTeamsAfterAMapChange";
const QString HIDE_ALLIES_ON_THE_AUTOMAP = "HideAlliesOnTheAutomap";
const QString DONT_LET_PLAYERS_SPY_ON_ALLIES = "DontLetPlayersSpyOnAllies";
const QString INSTANT_FLAG_SKULL_RETURN = "InstantFlagSkullReturn";
const QString NO_SUICIDE = "NoSuicide";
const QString NO_RESPAWN = "NoRespawn";
const QString NO_USE_AUTOMAP = "NoUseAutomap";
const QString NO_USE_AUTOAIM = "NoUseAutoaim";
const QString INFINITE_INVENTORY = "InfiniteInventory";
const QString SLOWLY_LOSE_HEALTH_WHEN_OVER100 = "SlowlyLoseHealthWhenOver100";
const QString CAN_USE_CHASECAM = "CanUseChasecam";
const QString ALLOW_BFGFREEAIMING = "AllowBFGFreeaiming";
const QString DONT_CHECK_AMMO_WHEN_SWITCHING_WEAPONS = "DontCheckAmmoWhenSwitchingWeapons";
const QString MONSTERS_MUST_BE_KILLED_TO_EXIT = "MonstersMustBeKilledToExit";
const QString KILL_BOSS_MONSTERS = "KillBossMonsters";
const QString RESPAWN_WHERE_DIED = "RespawnWhereDied";
const QString RESPAWN_WITH_ASHOTGUN = "RespawnWithAShotgun";
const QString DROP_WEAPON_ON_DEATH = "DropWeaponOnDeath";
const QString LOSE_AFRAG_ON_DEATH = "LoseAFragOnDeath";
const QString NO_RESPAWN_PROTECTION = "NoRespawnProtection";
const QString KEEP_FRAGS_AFTER_MAP_CHANGE = "KeepFragsAfterMapChange";
const QString DOUBLE_AMMO = "DoubleAmmo";
const QString DONT_SPAWN_RUNES = "DontSpawnRunes";
const QString NO_UNLAGGED = "NoUnlagged";
const QString ALWAYS_APPLY_LMSSPECTATOR_SETTINGS = "AlwaysApplyLMSSpectatorSettings";
const QString NO_MEDALS = "NoMedals";
const QString SCORE_DAMAGE_NOT_KILLS = "ScoreDamageNotKills";
const QString DONT_SPAWN_ANY_MULTIPLAYER_ITEM = "DontSpawnAnyMultiplayerItem";
const QString NO_TARGET_IDENTIFY = "NoTargetIdentify";
const QString NO_DISPLAY_COOP_INFO = "NoDisplayCoopInfo";
const QString NO_MAX_BLOOD_SCALAR = "NoMaxBloodScalar";
const QString NO_ITEM_DROP = "NoItemDrop";
const QString NO_ROCKET_JUMP = "NoRocketJump";
const QString NO_USE_CUSTOM_GLLIGHTING_SETTINGS = "NoUseCustomGLLightingSettings";
const QString NO_TURN_OFF_TRANSLUCENCY = "NoTurnOffTranslucency";
const QString SHARE_KEYS = "ShareKeys";
const QString SURVIVAL_NO_MAP_RESET_ON_DEATH = "SurvivalNoMapResetOnDeath";
const QString DEAD_PLAYERS_CAN_KEEP_INVENTORY = "DeadPlayersCanKeepInventory";
const QString LMSCHAINSAW = "LMSChainsaw";
const QString LMSPISTOL = "LMSPistol";
const QString LMSSHOTGUN = "LMSShotgun";
const QString LMSSUPER_SHOTGUN = "LMSSuperShotgun";
const QString LMSCHAINGUN = "LMSChaingun";
const QString LMSMINIGUN = "LMSMinigun";
const QString LMSROCKET_LAUNCHER = "LMSRocketLauncher";
const QString LMSGRENADE_LAUNCHER = "LMSGrenadeLauncher";
const QString LMSPLASMA_RIFLE = "LMSPlasmaRifle";
const QString LMSRAILGUN = "LMSRailgun";
const QString LMSSPECTATORS_CAN_TALK_TO_ACTIVE_PLAYERS = "LMSSpectatorsCanTalkToActivePlayers";
const QString LMSSPECTATORS_CAN_VIEW_THE_GAME = "LMSSpectatorsCanViewTheGame";
}

DClass<ZandronumqGameSettings>
{
public:
};
DPointered(ZandronumqGameSettings)

ZandronumqGameSettings::ZandronumqGameSettings()
{
}

ZandronumqGameSettings::~ZandronumqGameSettings()
{
}
