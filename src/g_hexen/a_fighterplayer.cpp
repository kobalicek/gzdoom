/*
#include "actor.h"
#include "gi.h"
#include "m_random.h"
#include "s_sound.h"
#include "d_player.h"
#include "a_action.h"
#include "p_local.h"
#include "a_action.h"
#include "a_hexenglobal.h"
#include "thingdef/thingdef.h"
*/

IMPLEMENT_CLASS (AFighterWeapon)
IMPLEMENT_CLASS (AClericWeapon)
IMPLEMENT_CLASS (AMageWeapon)

static FRandom pr_fpatk ("FPunchAttack");

//============================================================================
//
//	AdjustPlayerAngle
//
//============================================================================

#define MAX_ANGLE_ADJUST (5.)

void AdjustPlayerAngle (AActor *pmo, FTranslatedLineTarget *t)
{
	DAngle difference = deltaangle(pmo->Angles.Yaw, t->angleFromSource);
	if (fabs(difference) > MAX_ANGLE_ADJUST)
	{
		if (difference > 0)
		{
			pmo->Angles.Yaw += MAX_ANGLE_ADJUST;
		}
		else
		{
			pmo->Angles.Yaw -= MAX_ANGLE_ADJUST;
		}
	}
	else
	{
		pmo->Angles.Yaw = t->angleFromSource;
	}
}

//============================================================================
//
// TryPunch
//
// Returns true if an actor was punched, false if not.
//
//============================================================================

static bool TryPunch(APlayerPawn *pmo, angle_t angle, int damage, fixed_t power)
{
	PClassActor *pufftype;
	FTranslatedLineTarget t;
	int slope;

	slope = P_AimLineAttack (pmo, angle, 2*MELEERANGE, &t);
	if (t.linetarget != NULL)
	{
		if (++pmo->weaponspecial >= 3)
		{
			damage <<= 1;
			power *= 3;
			pufftype = PClass::FindActor("HammerPuff");
		}
		else
		{
			pufftype = PClass::FindActor("PunchPuff");
		}
		P_LineAttack (pmo, angle, 2*MELEERANGE, slope, damage, NAME_Melee, pufftype, true, &t);
		if (t.linetarget != NULL)
		{
			if (t.linetarget->player != NULL || 
				(t.linetarget->Mass != INT_MAX && (t.linetarget->flags3 & MF3_ISMONSTER)))
			{
				P_ThrustMobj (t.linetarget, t.angleFromSource, power);
			}
			AdjustPlayerAngle (pmo, &t);
			return true;
		}
	}
	return false;
}

//============================================================================
//
// A_FPunchAttack
//
//============================================================================

DEFINE_ACTION_FUNCTION(AActor, A_FPunchAttack)
{
	PARAM_ACTION_PROLOGUE;

	int damage;
	fixed_t power;
	int i;
	player_t *player;

	if (NULL == (player = self->player))
	{
		return 0;
	}
	APlayerPawn *pmo = player->mo;

	damage = 40+(pr_fpatk()&15);
	power = 2*FRACUNIT;
	for (i = 0; i < 16; i++)
	{
		if (TryPunch(pmo, pmo->_f_angle() + i*(ANG45/16), damage, power) ||
			TryPunch(pmo, pmo->_f_angle() - i*(ANG45/16), damage, power))
		{ // hit something
			if (pmo->weaponspecial >= 3)
			{
				pmo->weaponspecial = 0;
				P_SetPsprite (player, ps_weapon, player->ReadyWeapon->FindState("Fire2"));
				S_Sound (pmo, CHAN_VOICE, "*fistgrunt", 1, ATTN_NORM);
			}
			return 0;
		}
	}
	// didn't find any creatures, so try to strike any walls
	pmo->weaponspecial = 0;

	int slope = P_AimLineAttack (pmo, pmo->_f_angle(), MELEERANGE);
	P_LineAttack (pmo, pmo->_f_angle(), MELEERANGE, slope, damage, NAME_Melee, PClass::FindActor("PunchPuff"), true);
	return 0;
}
