//------------------------------------------------------------------------------
// flagsid.cpp
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
#include "flagsid.h"

#include "createserverdialogpages/flagspage.h"
#include "zandronumqgamesettings.h"
#include <ini/inisection.h>
#include <ini/inivariable.h>

#include <QCheckBox>

///////////////////////////////////////////////////////////////////////////

DClass<FlagsId>
{
public:
	FlagsPage *page;
};
DPointered(FlagsId)

FlagsId::FlagsId(FlagsPage *page)
{
	d->page = page;

	assign();
}

FlagsId::~FlagsId()
{
}

void FlagsId::save(IniSection &cfg)
{
	for (const QCheckBox *checkbox : d->page->findChildren<QCheckBox *>())
	{
		QVariant flagName = checkbox->property("flagname");
		if (flagName.isValid())
		{
			cfg[flagName.toString()] = checkbox->isChecked();
		}
	}
}

void FlagsId::load(const IniSection &cfg)
{
	for (QCheckBox *checkbox : d->page->findChildren<QCheckBox *>())
	{
		QVariant flagName = checkbox->property("flagname");
		if (flagName.isValid() && cfg.hasSetting(flagName.toString()))
		{
			checkbox->setChecked(static_cast<bool>(cfg[flagName.toString()]));
		}
	}
}

// Let's always keep this horror below at the bottom of this file,
// where it is out of sight.
void FlagsId::assign()
{
	using namespace ZandronumqGameSettingsNames;

	FlagsPage *f = d->page;

	f->cbCompatAllowSilentBFGTrick->setProperty("flagname", COMPAT_ALLOW_SILENT_BFGTRICK);
	f->cbCompatEnableWallRunning->setProperty("flagname", COMPAT_ENABLE_WALL_RUNNING);
	f->cbCompatDontLetOthersHearPickups->setProperty("flagname", COMPAT_DONT_LET_OTHERS_HEAR_PICKUPS);
	f->cbCompatMonstersSeeSemiInvisiblePlayers->setProperty("flagname", COMPAT_MONSTERS_SEE_SEMI_INVISIBLE_PLAYERS);
	f->cbCompatNoMonstersDropoffMove->setProperty("flagname", COMPAT_NO_MONSTERS_DROPOFF_MOVE);
	f->cbCompatScrollingSectorsAreAdditive->setProperty("flagname", COMPAT_SCROLLING_SECTORS_ARE_ADDITIVE);
	f->cbCompatInstantlyMovingFloorsArentSilent->setProperty("flagname", COMPAT_INSTANTLY_MOVING_FLOORS_ARENT_SILENT);
	f->cbCompatSectorSoundsUseOriginalMethod->setProperty("flagname", COMPAT_SECTOR_SOUNDS_USE_ORIGINAL_METHOD);
	f->cbCompatAnyBossDeathActivatesMapSpecials->setProperty("flagname", COMPAT_ANY_BOSS_DEATH_ACTIVATES_MAP_SPECIALS);
	f->cbCompatFrictionPushersPullersAffectMonsters->setProperty("flagname", COMPAT_FRICTION_PUSHERS_PULLERS_AFFECT_MONSTERS);
	f->cbCompatCrusherGibsByMorphingNotReplacement->setProperty("flagname", COMPAT_CRUSHER_GIBS_BY_MORPHING_NOT_REPLACEMENT);
	f->cbCompatBlockMonsterLinesIgnoreFriendlyMonsters->setProperty("flagname", COMPAT_BLOCK_MONSTER_LINES_IGNORE_FRIENDLY_MONSTERS);
	f->cbCompatFindNeighboringLightLevelLikeDoom->setProperty("flagname", COMPAT_FIND_NEIGHBORING_LIGHT_LEVEL_LIKE_DOOM);
	f->cbCompatFindShortestTexturesLikeDoom->setProperty("flagname", COMPAT_FIND_SHORTEST_TEXTURES_LIKE_DOOM);
	f->cbCompatUseBuggierStairBuilding->setProperty("flagname", COMPAT_USE_BUGGIER_STAIR_BUILDING);
	f->cbCompatLimitPainElementals->setProperty("flagname", COMPAT_LIMIT_PAIN_ELEMENTALS);
	f->cbCompatActorsAreInfinitelyTall->setProperty("flagname", COMPAT_ACTORS_ARE_INFINITELY_TALL);
	f->cbCompatSpawnItemDropsOnTheFloor->setProperty("flagname", COMPAT_SPAWN_ITEM_DROPS_ON_THE_FLOOR);
	f->cbCompatAllSpecialLinesCanDropUseLines->setProperty("flagname", COMPAT_ALL_SPECIAL_LINES_CAN_DROP_USE_LINES);
	f->cbCompatDisableBoomDoorLightEffect->setProperty("flagname", COMPAT_DISABLE_BOOM_DOOR_LIGHT_EFFECT);
	f->cbCompatRavensScrollersUseOriginalSpeed->setProperty("flagname", COMPAT_RAVENS_SCROLLERS_USE_ORIGINAL_SPEED);
	f->cbCompatUseSectorBasedSoundTargetCode->setProperty("flagname", COMPAT_USE_SECTOR_BASED_SOUND_TARGET_CODE);
	f->cbCompatLimitDehMaxHealthToHealthBonus->setProperty("flagname", COMPAT_LIMIT_DEH_MAX_HEALTH_TO_HEALTH_BONUS);
	f->cbCompatTraceIgnoreLinesWithoutSameSectorOnBothSides->setProperty("flagname", COMPAT_TRACE_IGNORE_LINES_WITHOUT_SAME_SECTOR_ON_BOTH_SIDES);
	f->cbCompatMonstersCantBePushedOffCliffs->setProperty("flagname", COMPAT_MONSTERS_CANT_BE_PUSHED_OFF_CLIFFS);
	f->cbCompatUseOriginalMissileClippingHeight->setProperty("flagname", COMPAT_USE_ORIGINAL_MISSILE_CLIPPING_HEIGHT);
	f->cbCompatMinotaur->setProperty("flagname", COMPAT_MINOTAUR);
	f->cbCompatOriginalVelocityCalcForMushroomInDehacked->setProperty("flagname", COMPAT_ORIGINAL_VELOCITY_CALC_FOR_MUSHROOM_IN_DEHACKED);
	f->cbCompatSpriteSortOrderInverted->setProperty("flagname", COMPAT_SPRITE_SORT_ORDER_INVERTED);
	f->cbCompatHitscansOriginalBlockmap->setProperty("flagname", COMPAT_HITSCANS_ORIGINAL_BLOCKMAP);
	f->cbCompatDrawPolyobjectsOld->setProperty("flagname", COMPAT_DRAW_POLYOBJECTS_OLD);
	f->cbNoTaunt->setProperty("flagname", NO_TAUNT);
	f->cbNoUseCrosshairs->setProperty("flagname", NO_USE_CROSSHAIRS);
	f->cbNoUseLandConsoleCommand->setProperty("flagname", NO_USE_LAND_CONSOLE_COMMAND);
	f->cbCompatPlasmaBumpBug->setProperty("flagname", COMPAT_PLASMA_BUMP_BUG);
	f->cbCompatOriginalWeaponSwitch->setProperty("flagname", COMPAT_ORIGINAL_WEAPON_SWITCH);
	f->cbCompatLimitedMovementInTheAir->setProperty("flagname", COMPAT_LIMITED_MOVEMENT_IN_THE_AIR);
	f->cbCompatAllowInstantRespawn->setProperty("flagname", COMPAT_ALLOW_INSTANT_RESPAWN);
	f->cbCompatUseOldIntermissionScreensMusic->setProperty("flagname", COMPAT_USE_OLD_INTERMISSION_SCREENS_MUSIC);
	f->cbCompatDisableStealthMonsters->setProperty("flagname", COMPAT_DISABLE_STEALTH_MONSTERS);
	f->cbCompatOldDamageRadiusInfiniteHeight->setProperty("flagname", COMPAT_OLD_DAMAGE_RADIUS_INFINITE_HEIGHT);
	f->cbCompatOldRandomNumberGenerator->setProperty("flagname", COMPAT_OLD_RANDOM_NUMBER_GENERATOR);
	f->cbCompatNETScriptsAreClientside->setProperty("flagname", COMPAT_NETSCRIPTS_ARE_CLIENTSIDE);
	f->cbCompatNoPredictionACS->setProperty("flagname", COMPAT_NO_PREDICTION_ACS);
	f->cbCompatAddNOGRAVITYFlagToSpheres->setProperty("flagname", COMPAT_ADD_NOGRAVITYFLAG_TO_SPHERES);
	f->cbDontStopPlayerScriptsOnDisconnect->setProperty("flagname", DONT_STOP_PLAYER_SCRIPTS_ON_DISCONNECT);
	f->cbOldZDoomHorizontalThrust->setProperty("flagname", OLD_ZDOOM_HORIZONTAL_THRUST);
	f->cbOldZDoomBridgeDrops->setProperty("flagname", OLD_ZDOOM_BRIDGE_DROPS);
	f->cbOldZDoomJumpPhysics->setProperty("flagname", OLD_ZDOOM_JUMP_PHYSICS);
	f->cbCompatFullWeaponLower->setProperty("flagname", COMPAT_FULL_WEAPON_LOWER);
	f->cbCompatOriginalSoundCurve->setProperty("flagname", COMPAT_ORIGINAL_SOUND_CURVE);
	f->cbCompatUseVanillaAutoaimTracerBehavior->setProperty("flagname", COMPAT_USE_VANILLA_AUTOAIM_TRACER_BEHAVIOR);
	f->cbCompatWestSpawnsAreSilent->setProperty("flagname", COMPAT_WEST_SPAWNS_ARE_SILENT);
	f->cbCompatMaskedMidtex->setProperty("flagname", COMPAT_MASKED_MIDTEX);
	f->cbCompatBadAngles->setProperty("flagname", COMPAT_BAD_ANGLES);
	f->cbCompatDisableCrosshairAccurate->setProperty("flagname", COMPAT_DISABLE_CROSSHAIR_ACCURATE);
	f->cbCompatDisableWallFriction->setProperty("flagname", COMPAT_DISABLE_WALL_FRICTION);
	f->cbCompatFloorMove->setProperty("flagname", COMPAT_FLOOR_MOVE);
	f->cbNoMonsters->setProperty("flagname", NO_MONSTERS);
	f->cbItemsRespawn->setProperty("flagname", ITEMS_RESPAWN);
	f->cbMegaPowerupsRespawn->setProperty("flagname", MEGA_POWERUPS_RESPAWN);
	f->cbNoUseFreelook->setProperty("flagname", NO_USE_FREELOOK);
	f->cbInfiniteAmmo->setProperty("flagname", INFINITE_AMMO);
	f->cbDontSpawnDeathmatchWeapons->setProperty("flagname", DONT_SPAWN_DEATHMATCH_WEAPONS);
	f->cbMonstersAreFast->setProperty("flagname", MONSTERS_ARE_FAST);
	f->cbMonstersRespawn->setProperty("flagname", MONSTERS_RESPAWN);
	f->cbLoseAllInventory->setProperty("flagname", LOSE_ALL_INVENTORY);
	f->cbLoseArmor->setProperty("flagname", LOSE_ARMOR);
	f->cbLoseKeys->setProperty("flagname", LOSE_KEYS);
	f->cbLosePowerups->setProperty("flagname", LOSE_POWERUPS);
	f->cbLoseWeapons->setProperty("flagname", LOSE_WEAPONS);
	f->cbLoseAllAmmo->setProperty("flagname", LOSE_ALL_AMMO);
	f->cbLoseHalfAmmo->setProperty("flagname", LOSE_HALF_AMMO);
	f->cbRespawnAutomatically->setProperty("flagname", RESPAWN_AUTOMATICALLY);
	f->cbRespawnFarthestAwayFromOthers->setProperty("flagname", RESPAWN_FARTHEST_AWAY_FROM_OTHERS);
	f->cbWeaponsStayAfterPickup->setProperty("flagname", WEAPONS_STAY_AFTER_PICKUP);
	f->cbDontSpawnHealth->setProperty("flagname", DONT_SPAWN_HEALTH);
	f->cbDontSpawnArmor->setProperty("flagname", DONT_SPAWN_ARMOR);
	f->cbBarrelsRespawn->setProperty("flagname", BARRELS_RESPAWN);
	f->cbServerPicksTeams->setProperty("flagname", SERVER_PICKS_TEAMS);
	f->cbPlayersCantSwitchTeams->setProperty("flagname", PLAYERS_CANT_SWITCH_TEAMS);
	f->cbKeepTeamsAfterAMapChange->setProperty("flagname", KEEP_TEAMS_AFTER_AMAP_CHANGE);
	f->cbHideAlliesOnTheAutomap->setProperty("flagname", HIDE_ALLIES_ON_THE_AUTOMAP);
	f->cbDontLetPlayersSpyOnAllies->setProperty("flagname", DONT_LET_PLAYERS_SPY_ON_ALLIES);
	f->cbInstantFlagSkullReturn->setProperty("flagname", INSTANT_FLAG_SKULL_RETURN);
	f->cbNoSuicide->setProperty("flagname", NO_SUICIDE);
	f->cbNoRespawn->setProperty("flagname", NO_RESPAWN);
	f->cbNoUseAutomap->setProperty("flagname", NO_USE_AUTOMAP);
	f->cbNoUseAutoaim->setProperty("flagname", NO_USE_AUTOAIM);
	f->cbInfiniteInventory->setProperty("flagname", INFINITE_INVENTORY);
	f->cbSlowlyLoseHealthWhenOver100->setProperty("flagname", SLOWLY_LOSE_HEALTH_WHEN_OVER100);
	f->cbCanUseChasecam->setProperty("flagname", CAN_USE_CHASECAM);
	f->cbAllowBFGFreeaiming->setProperty("flagname", ALLOW_BFGFREEAIMING);
	f->cbDontCheckAmmoWhenSwitchingWeapons->setProperty("flagname", DONT_CHECK_AMMO_WHEN_SWITCHING_WEAPONS);
	f->cbMonstersMustBeKilledToExit->setProperty("flagname", MONSTERS_MUST_BE_KILLED_TO_EXIT);
	f->cbKillBossMonsters->setProperty("flagname", KILL_BOSS_MONSTERS);
	f->cbRespawnWhereDied->setProperty("flagname", RESPAWN_WHERE_DIED);
	f->cbRespawnWithAShotgun->setProperty("flagname", RESPAWN_WITH_ASHOTGUN);
	f->cbDropWeaponOnDeath->setProperty("flagname", DROP_WEAPON_ON_DEATH);
	f->cbLoseAFragOnDeath->setProperty("flagname", LOSE_AFRAG_ON_DEATH);
	f->cbNoRespawnProtection->setProperty("flagname", NO_RESPAWN_PROTECTION);
	f->cbKeepFragsAfterMapChange->setProperty("flagname", KEEP_FRAGS_AFTER_MAP_CHANGE);
	f->cbDoubleAmmo->setProperty("flagname", DOUBLE_AMMO);
	f->cbDontSpawnRunes->setProperty("flagname", DONT_SPAWN_RUNES);
	f->cbNoUnlagged->setProperty("flagname", NO_UNLAGGED);
	f->cbAlwaysApplyLMSSpectatorSettings->setProperty("flagname", ALWAYS_APPLY_LMSSPECTATOR_SETTINGS);
	f->cbNoMedals->setProperty("flagname", NO_MEDALS);
	f->cbScoreDamageNotKills->setProperty("flagname", SCORE_DAMAGE_NOT_KILLS);
	f->cbDontSpawnAnyMultiplayerItem->setProperty("flagname", DONT_SPAWN_ANY_MULTIPLAYER_ITEM);
	f->cbNoTargetIdentify->setProperty("flagname", NO_TARGET_IDENTIFY);
	f->cbNoDisplayCoopInfo->setProperty("flagname", NO_DISPLAY_COOP_INFO);
	f->cbNoMaxBloodScalar->setProperty("flagname", NO_MAX_BLOOD_SCALAR);
	f->cbNoItemDrop->setProperty("flagname", NO_ITEM_DROP);
	f->cbNoRocketJump->setProperty("flagname", NO_ROCKET_JUMP);
	f->cbNoUseCustomGLLightingSettings->setProperty("flagname", NO_USE_CUSTOM_GLLIGHTING_SETTINGS);
	f->cbNoTurnOffTranslucency->setProperty("flagname", NO_TURN_OFF_TRANSLUCENCY);
	f->cbShareKeys->setProperty("flagname", SHARE_KEYS);
	f->cbSurvivalNoMapResetOnDeath->setProperty("flagname", SURVIVAL_NO_MAP_RESET_ON_DEATH);
	f->cbDeadPlayersCanKeepInventory->setProperty("flagname", DEAD_PLAYERS_CAN_KEEP_INVENTORY);
	f->cbLMSChainsaw->setProperty("flagname", LMSCHAINSAW);
	f->cbLMSPistol->setProperty("flagname", LMSPISTOL);
	f->cbLMSShotgun->setProperty("flagname", LMSSHOTGUN);
	f->cbLMSSuperShotgun->setProperty("flagname", LMSSUPER_SHOTGUN);
	f->cbLMSChaingun->setProperty("flagname", LMSCHAINGUN);
	f->cbLMSMinigun->setProperty("flagname", LMSMINIGUN);
	f->cbLMSRocketLauncher->setProperty("flagname", LMSROCKET_LAUNCHER);
	f->cbLMSGrenadeLauncher->setProperty("flagname", LMSGRENADE_LAUNCHER);
	f->cbLMSPlasmaRifle->setProperty("flagname", LMSPLASMA_RIFLE);
	f->cbLMSRailgun->setProperty("flagname", LMSRAILGUN);
	f->cbLMSSpectatorsCanTalkToActivePlayers->setProperty("flagname", LMSSPECTATORS_CAN_TALK_TO_ACTIVE_PLAYERS);
	f->cbLMSSpectatorsCanViewTheGame->setProperty("flagname", LMSSPECTATORS_CAN_VIEW_THE_GAME);
}
