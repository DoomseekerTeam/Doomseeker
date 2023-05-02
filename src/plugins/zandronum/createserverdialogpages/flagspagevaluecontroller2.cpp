//------------------------------------------------------------------------------
// flagspagevaluecontroller2.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "flagspagevaluecontroller2.h"

#include "createserverdialogpages/flagspage.h"
#include "zandronum2dmflags.h"
#include "zandronumgameinfo.h"

namespace Zandronum2
{
FlagsPageValueController::FlagsPageValueController(FlagsPage *flagsPage)
{
	this->flagsPage = flagsPage;
	this->compatflags = 0;
	this->zandronumCompatflags = 0;
	this->dmflags = 0;
	this->dmflags2 = 0;
	this->zandronumDmflags = 0;
	this->lmsAllowedWeapons = 0;
	this->lmsSpectatorSettings = 0;

	FlagsPage *f = flagsPage;

	// Compat flags
	{
		QMap<unsigned, QAbstractButton *> &m = compatflagsCheckboxes;

		// Compatibility
		// - Oldschool
		m.insert(Dmflags::COMPATF_MAGICSILENCE,
			f->cbCompatAllowSilentBFGTrick);
		m.insert(Dmflags::COMPATF_WALLRUN,
			f->cbCompatEnableWallRunning);
		m.insert(Dmflags::COMPATF_SILENTPICKUP,
			f->cbCompatDontLetOthersHearPickups);
		m.insert(Dmflags::COMPATF_INVISIBILITY,
			f->cbCompatMonstersSeeSemiInvisiblePlayers);
		// - General, left panel
		m.insert(Dmflags::COMPATF_DROPOFF,
			f->cbCompatNoMonstersDropoffMove);
		m.insert(Dmflags::COMPATF_BOOMSCROLL,
			f->cbCompatScrollingSectorsAreAdditive);
		m.insert(Dmflags::COMPATF_SILENT_INSTANT_FLOORS,
			f->cbCompatInstantlyMovingFloorsArentSilent);
		m.insert(Dmflags::COMPATF_SECTORSOUNDS,
			f->cbCompatSectorSoundsUseOriginalMethod);
		m.insert(Dmflags::COMPATF_ANYBOSSDEATH,
			f->cbCompatAnyBossDeathActivatesMapSpecials);
		m.insert(Dmflags::COMPATF_MBFMONSTERMOVE,
			f->cbCompatFrictionPushersPullersAffectMonsters);
		m.insert(Dmflags::COMPATF_CORPSEGIBS,
			f->cbCompatCrusherGibsByMorphingNotReplacement);
		m.insert(Dmflags::COMPATF_NOBLOCKFRIENDS,
			f->cbCompatBlockMonsterLinesIgnoreFriendlyMonsters);
		m.insert(Dmflags::COMPATF_LIGHT,
			f->cbCompatFindNeighboringLightLevelLikeDoom);

		// - General, right panel
		m.insert(Dmflags::COMPATF_SHORTTEX,
			f->cbCompatFindShortestTexturesLikeDoom);
		m.insert(Dmflags::COMPATF_STAIRINDEX,
			f->cbCompatUseBuggierStairBuilding);
		m.insert(Dmflags::COMPATF_LIMITPAIN,
			f->cbCompatLimitPainElementals);
		m.insert(Dmflags::COMPATF_NO_PASSMOBJ,
			f->cbCompatActorsAreInfinitelyTall);
		m.insert(Dmflags::COMPATF_NOTOSSDROPS,
			f->cbCompatSpawnItemDropsOnTheFloor);
		m.insert(Dmflags::COMPATF_USEBLOCKING,
			f->cbCompatAllSpecialLinesCanDropUseLines);
		m.insert(Dmflags::COMPATF_NODOORLIGHT,
			f->cbCompatDisableBoomDoorLightEffect);
		m.insert(Dmflags::COMPATF_RAVENSCROLL,
			f->cbCompatRavensScrollersUseOriginalSpeed);
		m.insert(Dmflags::COMPATF_SOUNDTARGET,
			f->cbCompatUseSectorBasedSoundTargetCode);
		m.insert(Dmflags::COMPATF_DEHHEALTH,
			f->cbCompatLimitDehMaxHealthToHealthBonus);
		m.insert(Dmflags::COMPATF_TRACE,
			f->cbCompatTraceIgnoreLinesWithoutSameSectorOnBothSides);
		m.insert(Dmflags::COMPATF_CROSSDROPOFF,
			f->cbCompatMonstersCantBePushedOffCliffs);
		m.insert(Dmflags::COMPATF_MISSILECLIP,
			f->cbCompatUseOriginalMissileClippingHeight);
		m.insert(Dmflags::COMPATF_MINOTAUR,
			f->cbCompatMinotaur);
		m.insert(Dmflags::COMPATF_MUSHROOM,
			f->cbCompatOriginalVelocityCalcForMushroomInDehacked);
		m.insert(Dmflags::COMPATF_SPRITESORT,
			f->cbCompatSpriteSortOrderInverted);
		m.insert(Dmflags::COMPATF_HITSCAN,
			f->cbCompatHitscansOriginalBlockmap);
		m.insert(Dmflags::COMPATF_POLYOBJ,
			f->cbCompatDrawPolyobjectsOld);
	}

	// Zandronum compatflags
	{
		QMap<unsigned, QAbstractButton *> &m = zandronumCompatflagsCheckboxes;

		// Players
		m.insert(Dmflags::ZACOMPATF_DISABLETAUNTS,
			f->cbNoTaunt);
		m.insert(Dmflags::ZACOMPATF_NO_CROSSHAIR,
			f->cbNoUseCrosshairs);
		// - Disallow
		m.insert(Dmflags::ZACOMPATF_NO_LAND,
			f->cbNoUseLandConsoleCommand);
		// Compatibility
		m.insert(Dmflags::ZACOMPATF_PLASMA_BUMP_BUG,
			f->cbCompatPlasmaBumpBug);
		m.insert(Dmflags::ZACOMPATF_OLD_WEAPON_SWITCH,
			f->cbCompatOriginalWeaponSwitch);
		m.insert(Dmflags::ZACOMPATF_LIMITED_AIRMOVEMENT,
			f->cbCompatLimitedMovementInTheAir);
		m.insert(Dmflags::ZACOMPATF_INSTANTRESPAWN,
			f->cbCompatAllowInstantRespawn);
		m.insert(Dmflags::ZACOMPATF_OLDINTERMISSION,
			f->cbCompatUseOldIntermissionScreensMusic);
		m.insert(Dmflags::ZACOMPATF_DISABLESTEALTHMONSTERS,
			f->cbCompatDisableStealthMonsters);
		m.insert(Dmflags::ZACOMPATF_OLDRADIUSDMG,
			f->cbCompatOldDamageRadiusInfiniteHeight);
		m.insert(Dmflags::ZACOMPATF_CLIENTS_SEND_FULL_BUTTON_INFO,
			f->cbCompatClientsSendFullButtonInfo);
		m.insert(Dmflags::ZACOMPATF_OLD_RANDOM_GENERATOR,
			f->cbCompatOldRandomNumberGenerator);
		m.insert(Dmflags::ZACOMPATF_NETSCRIPTS_ARE_CLIENTSIDE,
			f->cbCompatNETScriptsAreClientside);
		m.insert(Dmflags::ZACOMPATF_NOGRAVITY_SPHERES,
			f->cbCompatAddNOGRAVITYFlagToSpheres);
		m.insert(Dmflags::ZACOMPATF_DONT_STOP_PLAYER_SCRIPTS_ON_DISCONNECT,
			f->cbDontStopPlayerScriptsOnDisconnect);
		m.insert(Dmflags::ZACOMPATF_OLD_EXPLOSION_THRUST,
			f->cbOldZDoomHorizontalThrust);
		m.insert(Dmflags::ZACOMPATF_OLD_BRIDGE_DROPS,
			f->cbOldZDoomBridgeDrops);
		m.insert(Dmflags::ZACOMPATF_OLD_ZDOOM_ZMOVEMENT,
			f->cbOldZDoomJumpPhysics);
		m.insert(Dmflags::ZACOMPATF_FULL_WEAPON_LOWER,
			f->cbCompatFullWeaponLower);
		m.insert(Dmflags::ZACOMPATF_ORIGINALSOUNDCURVE,
			f->cbCompatOriginalSoundCurve);
		m.insert(Dmflags::ZACOMPATF_AUTOAIM,
			f->cbCompatUseVanillaAutoaimTracerBehavior);
		m.insert(Dmflags::ZACOMPATF_SILENT_WEST_SPAWNS,
			f->cbCompatWestSpawnsAreSilent);
	}

	// DMFlags
	{
		QMap<unsigned, QAbstractButton *> &m = dmflagsCheckboxes;

		// General
		m.insert(Dmflags::DF_NO_MONSTERS,
			f->cbNoMonsters);
		m.insert(Dmflags::DF_ITEMS_RESPAWN,
			f->cbItemsRespawn);
		m.insert(Dmflags::DF_RESPAWN_SUPER,
			f->cbMegaPowerupsRespawn);

		// Players
		m.insert(Dmflags::DF_NO_FOV,
			f->cbNoUseFOV);
		m.insert(Dmflags::DF_NO_FREELOOK,
			f->cbNoUseFreelook);
		m.insert(Dmflags::DF_INFINITE_AMMO,
			f->cbInfiniteAmmo);

		// Cooperative
		// - Environment
		m.insert(Dmflags::DF_NO_COOP_WEAPON_SPAWN,
			f->cbDontSpawnDeathmatchWeapons);
		// - Monsters
		m.insert(Dmflags::DF_FAST_MONSTERS,
			f->cbMonstersAreFast);
		m.insert(Dmflags::DF_MONSTERS_RESPAWN,
			f->cbMonstersRespawn);
		// - Player death
		m.insert(Dmflags::DF_COOP_LOSE_INVENTORY,
			f->cbLoseAllInventory);
		m.insert(Dmflags::DF_COOP_LOSE_ARMOR,
			f->cbLoseArmor);
		m.insert(Dmflags::DF_COOP_LOSE_KEYS,
			f->cbLoseKeys);
		m.insert(Dmflags::DF_COOP_LOSE_POWERUPS,
			f->cbLosePowerups);
		m.insert(Dmflags::DF_COOP_LOSE_WEAPONS,
			f->cbLoseWeapons);
		m.insert(Dmflags::DF_COOP_LOSE_AMMO,
			f->cbLoseAllAmmo);
		m.insert(Dmflags::DF_COOP_HALVE_AMMO,
			f->cbLoseHalfAmmo);

		// Deathmatch
		// - When players die
		m.insert(Dmflags::DF_FORCE_RESPAWN,
			f->cbRespawnAutomatically);
		m.insert(Dmflags::DF_SPAWN_FARTHEST,
			f->cbRespawnFarthestAwayFromOthers);
		// - Weapons&ammo
		m.insert(Dmflags::DF_WEAPONS_STAY,
			f->cbWeaponsStayAfterPickup);
		// - Don't spawn
		m.insert(Dmflags::DF_NO_HEALTH,
			f->cbDontSpawnHealth);
		m.insert(Dmflags::DF_NO_ARMOR,
			f->cbDontSpawnArmor);
	}

	// DMFlags 2
	{
		QMap<unsigned, QAbstractButton *> &m = dmflags2Checkboxes;

		// General
		m.insert(Dmflags::DF2_BARRELS_RESPAWN,
			f->cbBarrelsRespawn);
		m.insert(Dmflags::DF2_NO_TEAM_SELECT,
			f->cbServerPicksTeams);
		m.insert(Dmflags::DF2_NO_TEAM_SWITCH,
			f->cbPlayersCantSwitchTeams);
		m.insert(Dmflags::ZADF_YES_KEEP_TEAMS,
			f->cbKeepTeamsAfterAMapChange);
		m.insert(Dmflags::DF2_NO_AUTOMAP_ALLIES,
			f->cbHideAlliesOnTheAutomap);
		m.insert(Dmflags::DF2_DISALLOW_SPYING,
			f->cbDontLetPlayersSpyOnAllies);
		m.insert(Dmflags::DF2_INSTANT_RETURN,
			f->cbInstantFlagSkullReturn);

		// Players
		// - Disallow
		m.insert(Dmflags::DF2_NOSUICIDE,
			f->cbNoSuicide);
		m.insert(Dmflags::DF2_NO_RESPAWN,
			f->cbNoRespawn);
		m.insert(Dmflags::DF2_NO_AUTOMAP,
			f->cbNoUseAutomap);
		m.insert(Dmflags::DF2_NOAUTOAIM,
			f->cbNoUseAutoaim);
		// - Abilities
		m.insert(Dmflags::DF2_INFINITE_INVENTORY,
			f->cbInfiniteInventory);
		m.insert(Dmflags::DF2_YES_DEGENERATION,
			f->cbSlowlyLoseHealthWhenOver100);
		m.insert(Dmflags::DF2_CHASECAM,
			f->cbCanUseChasecam);
		m.insert(Dmflags::DF2_YES_FREEAIMBFG,
			f->cbAllowBFGFreeaiming);
		m.insert(Dmflags::DF2_DONTCHECKAMMO,
			f->cbDontCheckAmmoWhenSwitchingWeapons);

		// Cooperative
		// - Monsters
		m.insert(Dmflags::DF2_KILL_MONSTERS,
			f->cbMonstersMustBeKilledToExit);
		m.insert(Dmflags::DF2_KILLBOSSMONST,
			f->cbKillBossMonsters);
		// - Player death
		m.insert(Dmflags::DF2_SAME_SPAWN_SPOT,
			f->cbRespawnWhereDied);

		// Deathmatch
		// - When players die
		m.insert(Dmflags::DF2_COOP_SHOTGUNSTART,
			f->cbRespawnWithAShotgun);
		m.insert(Dmflags::DF2_YES_WEAPONDROP,
			f->cbDropWeaponOnDeath);
		m.insert(Dmflags::DF2_YES_LOSEFRAG,
			f->cbLoseAFragOnDeath);
		m.insert(Dmflags::DF2_NO_RESPAWN_INVUL,
			f->cbNoRespawnProtection);
		// - When someone exits
		m.insert(Dmflags::DF2_YES_KEEPFRAGS,
			f->cbKeepFragsAfterMapChange);
		// - Weapons&ammo
		m.insert(Dmflags::DF2_YES_DOUBLEAMMO,
			f->cbDoubleAmmo);
		// - Don't spawn
		m.insert(Dmflags::DF2_NO_RUNES,
			f->cbDontSpawnRunes);
	}

	// Zandronum DMFlags
	{
		QMap<unsigned, QAbstractButton *> &m = zandronumDmflagsCheckboxes;

		// General
		m.insert(Dmflags::ZADF_NOUNLAGGED,
			f->cbNoUnlagged);
		m.insert(Dmflags::ZADF_ALWAYS_APPLY_LMS_SPECTATORSETTINGS,
			f->cbAlwaysApplyLMSSpectatorSettings);
		m.insert(Dmflags::ZADF_NO_MEDALS,
			f->cbNoMedals);
		m.insert(Dmflags::ZADF_AWARD_DAMAGE_INSTEAD_KILLS,
			f->cbScoreDamageNotKills);
		// - Environment
		m.insert(Dmflags::ZADF_COOP_SP_ACTOR_SPAWN,
			f->cbDontSpawnAnyMultiplayerItem);

		// Players
		// - Disallow 2
		m.insert(Dmflags::ZADF_NO_IDENTIFY_TARGET,
			f->cbNoTargetIdentify);
		m.insert(Dmflags::ZADF_NO_COOP_INFO,
			f->cbNoDisplayCoopInfo);
		m.insert(Dmflags::ZADF_MAX_BLOOD_SCALAR,
			f->cbNoMaxBloodScalar);
		// - Behavior
		m.insert(Dmflags::ZADF_UNBLOCK_PLAYERS,
			f->rbPlayersCanWalkThroughEachOther);
		m.insert(Dmflags::ZADF_UNBLOCK_ALLIES,
			f->rbAlliesCanWalkThroughEachOther);

		m.insert(Dmflags::ZADF_NO_ROCKET_JUMPING,
			f->cbNoRocketJump);
		m.insert(Dmflags::ZADF_FORCE_GL_DEFAULTS,
			f->cbNoUseCustomGLLightingSettings);
		m.insert(Dmflags::ZADF_FORCE_ALPHA,
			f->cbNoTurnOffTranslucency);

		// Cooperative
		m.insert(Dmflags::ZADF_SHARE_KEYS,
			f->cbShareKeys);
	}

	// LMSAllowedWeapons
	{
		QMap<unsigned, QAbstractButton *> &m = lmsAllowedWeaponsCheckboxes;

		m.insert(ZandronumGameInfo::LMSAW_Chainsaw,
			f->cbLMSChainsaw);
		m.insert(ZandronumGameInfo::LMSAW_Pistol,
			f->cbLMSPistol);
		m.insert(ZandronumGameInfo::LMSAW_Shotgun,
			f->cbLMSShotgun);
		m.insert(ZandronumGameInfo::LMSAW_SuperShotgun,
			f->cbLMSSuperShotgun);
		m.insert(ZandronumGameInfo::LMSAW_Chaingun,
			f->cbLMSChaingun);
		m.insert(ZandronumGameInfo::LMSAW_Minigun,
			f->cbLMSMinigun);
		m.insert(ZandronumGameInfo::LMSAW_RocketLauncher,
			f->cbLMSRocketLauncher);
		m.insert(ZandronumGameInfo::LMSAW_GrenadeLauncher,
			f->cbLMSGrenadeLauncher);
		m.insert(ZandronumGameInfo::LMSAW_PlasmaRifle,
			f->cbLMSPlasmaRifle);
		m.insert(ZandronumGameInfo::LMSAW_Railgun,
			f->cbLMSRailgun);
	}

	// LMSSpectatorSettings
	{
		QMap<unsigned, QAbstractButton *> &m = lmsSpectatorSettingsCheckboxes;

		m.insert(ZandronumGameInfo::LMSSS_TalkToActivePlayers,
			f->cbLMSSpectatorsCanTalkToActivePlayers);
		m.insert(ZandronumGameInfo::LMSSS_ViewTheGame,
			f->cbLMSSpectatorsCanViewTheGame);
	}
}

void FlagsPageValueController::convertNumericalToWidgets()
{
	// Get values from widgets.
	readFlagsFromTextInputs();

	convertNumericalToCheckboxes(compatflagsCheckboxes, compatflags);
	convertNumericalToCheckboxes(zandronumCompatflagsCheckboxes, zandronumCompatflags);
	convertNumericalToCheckboxes(dmflagsCheckboxes, dmflags);
	convertNumericalToCheckboxes(dmflags2Checkboxes, dmflags2);
	convertNumericalToCheckboxes(zandronumDmflagsCheckboxes, zandronumDmflags);
	convertNumericalToCheckboxes(lmsAllowedWeaponsCheckboxes, lmsAllowedWeapons);
	convertNumericalToCheckboxes(lmsSpectatorSettingsCheckboxes, lmsSpectatorSettings);

	convertToWidgetGeneral();
	convertToWidgetPlayers();
	convertToWidgetCooperative();
	convertToWidgetDeathmatch();
}

void FlagsPageValueController::convertWidgetsToNumerical()
{
	// Reset internal values.
	readFlagsFromTextInputs();

	convertCheckboxesToNumerical(compatflagsCheckboxes, compatflags);
	convertCheckboxesToNumerical(zandronumCompatflagsCheckboxes, zandronumCompatflags);
	convertCheckboxesToNumerical(dmflagsCheckboxes, dmflags);
	convertCheckboxesToNumerical(dmflags2Checkboxes, dmflags2);
	convertCheckboxesToNumerical(zandronumDmflagsCheckboxes, zandronumDmflags);
	convertCheckboxesToNumerical(lmsAllowedWeaponsCheckboxes, lmsAllowedWeapons);
	convertCheckboxesToNumerical(lmsSpectatorSettingsCheckboxes, lmsSpectatorSettings);

	convertToNumericalGeneral();
	convertToNumericalPlayers();
	convertToNumericalCooperative();
	convertToNumericalDeathmatch();

	// Apply values to widgets.
	flagsPage->leDmflags->setText(QString::number(dmflags));
	flagsPage->leDmflags2->setText(QString::number(dmflags2));
	flagsPage->leZandronumDmflags->setText(QString::number(zandronumDmflags));
	flagsPage->leCompatflags->setText(QString::number(compatflags));
	flagsPage->leZandronumCompatflags->setText(QString::number(zandronumCompatflags));
	flagsPage->leLMSAllowedWeapons->setText(QString::number(lmsAllowedWeapons));
	flagsPage->leLMSSpectatorSettings->setText(QString::number(lmsSpectatorSettings));
}

void FlagsPageValueController::convertCheckboxesToNumerical(
	const QMap<unsigned, QAbstractButton *> &checkboxMap,
	unsigned &flagsValue)
{
	for (unsigned flag : checkboxMap.keys())
	{
		if (checkboxMap[flag]->isChecked())
		{
			flagsValue |= flag;
		}
		else
		{
			// We are subtracting the disabled flags from the current value,
			// because the flagsValue variable preserves user input flags which
			// are not supported by the GUI.
			flagsValue &= ~flag;
		}
	}
}

void FlagsPageValueController::convertNumericalToCheckboxes(
	QMap<unsigned, QAbstractButton *> &checkboxMap,
	unsigned flagsValue)
{
	for (unsigned flag : checkboxMap.keys())
	{
		checkboxMap[flag]->setChecked(flagsValue & flag);
	}
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToNumericalGeneral()
{
	FlagsPage *f = flagsPage;

	// Environment

	// Reset the value.
	dmflags &= ~(Dmflags::DF_FORCE_FALLINGZD
		| Dmflags::DF_FORCE_FALLINGHX
		| Dmflags::DF_FORCE_FALLINGST);
	switch (f->cboFallingDamage->currentIndex())
	{
	case FlagsPage::FDT_None:
		// pass
		break;

	case FlagsPage::FDT_Old:
		dmflags |= Dmflags::DF_FORCE_FALLINGZD;
		break;

	case FlagsPage::FDT_Hexen:
		dmflags |= Dmflags::DF_FORCE_FALLINGHX;
		break;

	case FlagsPage::FDT_Strife:
		dmflags |= Dmflags::DF_FORCE_FALLINGST;
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToNumericalPlayers()
{
	FlagsPage *f = flagsPage;

	// Abilities

	// Reset the value.
	dmflags &= ~(Dmflags::DF_YES_JUMP
		| Dmflags::DF_NO_JUMP);
	switch (f->cboJumping->currentIndex())
	{
	case FlagsPage::JCA_Default:
		// pass
		break;
	case FlagsPage::JCA_Yes:
		dmflags |= Dmflags::DF_YES_JUMP;
		break;
	case FlagsPage::JCA_No:
		dmflags |= Dmflags::DF_NO_JUMP;
		break;
	}

	// Reset the value.
	dmflags &= ~(Dmflags::DF_YES_CROUCH
		| Dmflags::DF_NO_CROUCH);
	switch (f->cboCrouching->currentIndex())
	{
	case FlagsPage::JCA_Default:
		// pass
		break;
	case FlagsPage::JCA_Yes:
		dmflags |= Dmflags::DF_YES_CROUCH;
		break;
	case FlagsPage::JCA_No:
		dmflags |= Dmflags::DF_NO_CROUCH;
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToNumericalCooperative()
{
	// Placeholder method for future use.
}

////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToNumericalDeathmatch()
{
	FlagsPage *f = flagsPage;

	// Reset the value.
	dmflags &= ~(Dmflags::DF_NO_EXIT
		| Dmflags::DF_SAME_LEVEL);
	if (f->rbKillThePlayer->isChecked())
	{
		dmflags |= Dmflags::DF_NO_EXIT;
	}
	else if (f->rbRestartTheCurrentLevel->isChecked())
	{
		dmflags |= Dmflags::DF_SAME_LEVEL;
	}
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToWidgetGeneral()
{
	FlagsPage *f = flagsPage;

	// Environment

	// This one is tricky because FALLINGST flag is actually defined
	// as a combination of two other flags. We need to check for it first and
	// we also need to check if the result of 'AND' operation is equal to the
	// value of the flag.
	if ((dmflags & Dmflags::DF_FORCE_FALLINGST)
		== Dmflags::DF_FORCE_FALLINGST)
	{
		f->cboFallingDamage->setCurrentIndex(FlagsPage::FDT_Strife);
	}
	else if (dmflags & Dmflags::DF_FORCE_FALLINGZD)
	{
		f->cboFallingDamage->setCurrentIndex(FlagsPage::FDT_Old);
	}
	else if (dmflags & Dmflags::DF_FORCE_FALLINGHX)
	{
		f->cboFallingDamage->setCurrentIndex(FlagsPage::FDT_Hexen);
	}
	else
	{
		f->cboFallingDamage->setCurrentIndex(FlagsPage::FDT_None);
	}
}
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToWidgetPlayers()
{
	FlagsPage *f = flagsPage;

	// Abilities
	if (dmflags & Dmflags::DF_YES_JUMP)
	{
		f->cboJumping->setCurrentIndex(FlagsPage::JCA_Yes);
	}
	else if (dmflags & Dmflags::DF_NO_JUMP)
	{
		f->cboJumping->setCurrentIndex(FlagsPage::JCA_No);
	}
	else
	{
		f->cboJumping->setCurrentIndex(FlagsPage::JCA_Default);
	}

	if (dmflags & Dmflags::DF_YES_CROUCH)
	{
		f->cboCrouching->setCurrentIndex(FlagsPage::JCA_Yes);
	}
	else if (dmflags & Dmflags::DF_NO_CROUCH)
	{
		f->cboCrouching->setCurrentIndex(FlagsPage::JCA_No);
	}
	else
	{
		f->cboCrouching->setCurrentIndex(FlagsPage::JCA_Default);
	}

	if ((zandronumDmflags & Dmflags::ZADF_UNBLOCK_PLAYERS) == 0 &&
		(zandronumDmflags & Dmflags::ZADF_UNBLOCK_ALLIES) == 0)
	{
		f->rbPlayersBlockEachOtherNormally->setChecked(true);
	}
}
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToWidgetCooperative()
{
	// Placeholder method for future use.
}
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::convertToWidgetDeathmatch()
{
	FlagsPage *f = flagsPage;

	if (dmflags & Dmflags::DF_NO_EXIT)
	{
		f->rbKillThePlayer->setChecked(true);
	}
	else if (dmflags & Dmflags::DF_SAME_LEVEL)
	{
		f->rbRestartTheCurrentLevel->setChecked(true);
	}
	else
	{
		f->rbContinueToTheNextMap->setChecked(true);
	}
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void FlagsPageValueController::readFlagsFromTextInputs()
{
	dmflags = flagsPage->leDmflags->text().toUInt();
	dmflags2 = flagsPage->leDmflags2->text().toUInt();
	zandronumDmflags = flagsPage->leZandronumDmflags->text().toUInt();
	compatflags = flagsPage->leCompatflags->text().toUInt();
	zandronumCompatflags = flagsPage->leZandronumCompatflags->text().toUInt();
	lmsAllowedWeapons = flagsPage->leLMSAllowedWeapons->text().toUInt();
	lmsSpectatorSettings = flagsPage->leLMSSpectatorSettings->text().toUInt();
}
}
