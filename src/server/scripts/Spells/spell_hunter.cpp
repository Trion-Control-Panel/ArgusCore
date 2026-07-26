/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Scripts for spells with SPELLFAMILY_HUNTER, SPELLFAMILY_PET and SPELLFAMILY_GENERIC spells used by hunter players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_hun_".
 */

#include "ScriptMgr.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Pet.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum HunterSpells
{
    SPELL_HUNTER_A_MURDER_OF_CROWS_DAMAGE           = 131900,
    SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_1         = 131637,
    SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_2         = 131951,
    SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_3         = 131952,
    SPELL_HUNTER_AIMED_SHOT                         = 19434,
    SPELL_HUNTER_ANIMAL_INSTINCTS                   = 204315,
    SPELL_HUNTER_ARCANE_SHOT                        = 185358,
    SPELL_HUNTER_ASPECT_CHEETAH_SLOW                = 186258,
    SPELL_HUNTER_ASPECT_OF_THE_EAGLE                = 186289,
    SPELL_HUNTER_ASPECT_OF_THE_FOX                  = 1219162,
    SPELL_HUNTER_ASPECT_OF_THE_TURTLE_PACIFY_AURA   = 205769,
    SPELL_HUNTER_AUTO_SHOT                          = 75,
    SPELL_HUNTER_BARRAGE                            = 120360,
    SPELL_HUNTER_BEAST_CLEAVE_AURA                  = 115939,
    SPELL_HUNTER_BEAST_CLEAVE_DAMAGE                = 118459,
    SPELL_HUNTER_BEAST_CLEAVE_PROC                  = 118455,
    SPELL_HUNTER_BESTIAL_WRATH                      = 19574,
    SPELL_HUNTER_BINDING_SHOT                       = 109248,
    SPELL_HUNTER_CAMOUFLAGE                         = 199483,
    SPELL_HUNTER_CHIMAERA_SHOT_HEAD_1               = 171454,
    SPELL_HUNTER_CHIMAERA_SHOT_HEAD_2               = 171457,
    SPELL_HUNTER_COBRA_SHOT                         = 193455,
    SPELL_HUNTER_CONCUSSIVE_SHOT                    = 5116,
    SPELL_HUNTER_DIRE_BEAST_GENERIC                 = 120679,
    SPELL_HUNTER_DIRE_FRENZY                        = 217200,
    SPELL_HUNTER_DISENGAGE                          = 781,
    SPELL_HUNTER_EMERGENCY_SALVE_TALENT             = 459517,
    SPELL_HUNTER_EMERGENCY_SALVE_DISPEL             = 459521,
    SPELL_HUNTER_EXHILARATION                       = 109304,
    SPELL_HUNTER_EXHILARATION_PET                   = 128594,
    SPELL_HUNTER_EXHILARATION_R2                    = 231546,
    SPELL_HUNTER_EXPLOSIVE_SHOT_DAMAGE              = 212680,
    SPELL_HUNTER_FLANKING_STRIKE                    = 202800,
    SPELL_HUNTER_FLANKING_STRIKE_PROC               = 204740,
    SPELL_HUNTER_FLANKING_STRIKE_PROC_UP            = 206933,
    SPELL_HUNTER_GREVIOUS_INJURY                    = 1217789,
    SPELL_HUNTER_HARPOON                            = 190925,
    SPELL_HUNTER_HARPOON_ROOT                       = 190927,
    SPELL_HUNTER_HIGH_EXPLOSIVE_TRAP                = 236775,
    SPELL_HUNTER_HIGH_EXPLOSIVE_TRAP_DAMAGE         = 236777,
    SPELL_HUNTER_HUNTERS_MARK                       = 185987,
    SPELL_HUNTER_HUNTERS_MARK_AURA                  = 185365,
    SPELL_HUNTER_HUNTERS_MARK_AURA_2                = 185743,
    SPELL_HUNTER_IMPLOSIVE_TRAP                     = 462032,
    SPELL_HUNTER_IMPLOSIVE_TRAP_DAMAGE              = 462033,
    SPELL_HUNTER_INTIMIDATION                       = 19577,
    SPELL_HUNTER_INTIMIDATION_MARKSMANSHIP          = 474421,
    SPELL_HUNTER_INTIMIDATION_STUN                  = 24394,
    SPELL_HUNTER_KILL_COMMAND                       = 34026,
    SPELL_HUNTER_KILL_COMMAND_CHARGE                = 118171,
    SPELL_HUNTER_KILL_COMMAND_TRIGGER               = 83381,
    SPELL_HUNTER_LACERATE                           = 185855,
    SPELL_HUNTER_LATENT_POISON_STACK                = 378015,
    SPELL_HUNTER_LATENT_POISON_DAMAGE               = 378016,
    SPELL_HUNTER_LATENT_POISON_INJECTORS_STACK      = 336903,
    SPELL_HUNTER_LATENT_POISON_INJECTORS_DAMAGE     = 336904,
    SPELL_HUNTER_LONE_WOLF                          = 155228,
    SPELL_HUNTER_MARKED_SHOT                        = 185901,
    SPELL_HUNTER_MARKED_SHOT_DAMAGE                 = 212621,
    SPELL_HUNTER_MARKING_TARGETS                    = 223138,
    SPELL_HUNTER_MARKSMANSHIP_HUNTER_AURA           = 137016,
    SPELL_HUNTER_MASTERS_CALL_TRIGGERED             = 62305,
    SPELL_HUNTER_MISDIRECTION                       = 34477,
    SPELL_HUNTER_MISDIRECTION_PROC                  = 35079,
    SPELL_HUNTER_MONGOOSE_BITE                      = 190928,
    SPELL_HUNTER_MONGOOSE_FURY                      = 190931,
    SPELL_HUNTER_MULTI_SHOT_FOCUS                   = 213363,
    SPELL_HUNTER_MULTISHOT                          = 2643,
    SPELL_HUNTER_PET_LAST_STAND_TRIGGERED           = 53479,
    SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED = 54114,
    SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF    = 55711,
    SPELL_HUNTER_POSTHASTE_INCREASE_SPEED           = 118922,
    SPELL_HUNTER_POSTHASTE_TALENT                   = 109215,
    SPELL_HUNTER_RANGERS_NET_INCREASE_SPEED         = 206755,
    SPELL_HUNTER_RAPID_FIRE                         = 257044,
    SPELL_HUNTER_RAPID_FIRE_DAMAGE                  = 257045,
    SPELL_HUNTER_RAPID_FIRE_ENERGIZE                = 263585,
    SPELL_HUNTER_RAPTOR_STRIKE                      = 186270,
    SPELL_HUNTER_REJUVENATING_WIND_HEAL             = 385540,
    SPELL_HUNTER_SCOUTS_INSTINCTS                   = 459455,
    SPELL_HUNTER_SENTINEL                           = 206817,
    SPELL_HUNTER_SERPENT_STING                      = 87935,
    SPELL_HUNTER_SERPENT_STING_DAMAGE               = 118253,
    SPELL_HUNTER_STEADY_SHOT                        = 56641,
    SPELL_HUNTER_STEADY_SHOT_FOCUS                  = 77443,
    SPELL_HUNTER_STICKY_BOMB_PROC                   = 191244,
    SPELL_HUNTER_STREAMLINE_TALENT                  = 260367,
    SPELL_HUNTER_STREAMLINE_BUFF                    = 342076,
    SPELL_HUNTER_T9_4P_GREATNESS                    = 68130,
    SPELL_HUNTER_T29_2P_MARKSMANSHIP_DAMAGE         = 394371,
    SPELL_HUNTER_TAR_TRAP                           = 187699,
    SPELL_HUNTER_TAR_TRAP_AREATRIGGER               = 187700,
    SPELL_HUNTER_TAR_TRAP_SLOW                      = 135299,
    SPELL_HUNTER_VULNERABLE                         = 187131,
    SPELL_HUNTER_WILDERNESS_MEDICINE_TALENT         = 343242,
    SPELL_HUNTER_WILDERNESS_MEDICINE_DISPEL         = 384784,
    SPELL_ROAR_OF_SACRIFICE_TRIGGERED               = 67481
};

enum MiscSpells
{
    SPELL_DRAENEI_GIFT_OF_THE_NAARU                 = 59543,
};

// 131894 - A Murder of Crows
class spell_hun_a_murder_of_crows : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_HUNTER_A_MURDER_OF_CROWS_DAMAGE,
            SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_1,
            SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_2,
            SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_3
        });
    }

    void HandleDummyTick(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();

        if (Unit* caster = GetCaster())
            caster->CastSpell(target, SPELL_HUNTER_A_MURDER_OF_CROWS_DAMAGE, true);

        target->CastSpell(target, SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_1, true);
        target->CastSpell(target, SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_2, true);
        target->CastSpell(target, SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_3, true);
        target->CastSpell(target, SPELL_HUNTER_A_MURDER_OF_CROWS_VISUAL_3, true); // not a mistake, it is intended to cast twice
    }

    void RemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
            if (Unit* caster = GetCaster())
                caster->GetSpellHistory()->ResetCooldown(GetId(), true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_a_murder_of_crows::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectRemove += AuraEffectRemoveFn(spell_hun_a_murder_of_crows::RemoveEffect, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 185358 - Arcane Shot
// Part of Legion Marksmanship's Hunter's Mark/Vulnerable kit: if the Hunter currently has the
// Marking Targets buff (granted by Hunter's Mark's own DB2 proc data, gated to Auto Shot by
// spell_hun_hunters_mark below), the next Arcane Shot applies Hunter's Mark to its target and
// consumes the buff.
class spell_hun_arcane_shot : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (caster->HasAura(SPELL_HUNTER_MARKING_TARGETS))
        {
            caster->CastSpell(target, SPELL_HUNTER_HUNTERS_MARK_AURA, true);
            caster->CastSpell(caster, SPELL_HUNTER_HUNTERS_MARK_AURA_2, true);
            caster->RemoveAurasDueToSpell(SPELL_HUNTER_MARKING_TARGETS);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_arcane_shot::HandleOnHit);
    }
};

// 186257 - Aspect of the Cheetah
class spell_hun_aspect_cheetah : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_HUNTER_ASPECT_CHEETAH_SLOW
        });
    }

    void HandleOnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            GetTarget()->CastSpell(GetTarget(), SPELL_HUNTER_ASPECT_CHEETAH_SLOW, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hun_aspect_cheetah::HandleOnRemove, EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED, AURA_EFFECT_HANDLE_REAL);
    }
};

// 1219162 - Aspect of the Fox (attached to 186257 - Aspect of the Cheetah)
class spell_hun_aspect_of_the_fox : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_ASPECT_OF_THE_FOX })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } })
            && spellInfo->GetEffect(EFFECT_2).IsAura(SPELL_AURA_CAST_WHILE_WALKING);
    }

    bool Load() override
    {
        return !GetCaster()->HasAura(SPELL_HUNTER_ASPECT_OF_THE_FOX);
    }

    static void HandleCastWhileWalking(SpellScript const&, WorldObject*& target)
    {
        target = nullptr;
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_hun_aspect_of_the_fox::HandleCastWhileWalking, EFFECT_2, TARGET_UNIT_CASTER);
    }
};

// 186265 - Aspect of the Turtle
class spell_hun_aspect_of_the_turtle : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_ASPECT_OF_THE_TURTLE_PACIFY_AURA });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_HUNTER_ASPECT_OF_THE_TURTLE_PACIFY_AURA, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_HUNTER_ASPECT_OF_THE_TURTLE_PACIFY_AURA);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_hun_aspect_of_the_turtle::OnApply, EFFECT_0, SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_hun_aspect_of_the_turtle::OnRemove, EFFECT_0, SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 120360 - Barrage
class spell_hun_barrage : public SpellScript
{
    void CheckLOS(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        targets.remove_if([caster](WorldObject* object) -> bool
        {
            if (!object)
                return true;

            if (!object->IsWithinLOSInMap(caster))
                return true;

            if (object->ToUnit() && !caster->IsValidAttackTarget(object->ToUnit()))
                return true;

            return false;
        });
    }

    void HandleOnHit()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!player || !target)
            return;

        float minDamage = 0.0f;
        float maxDamage = 0.0f;
        player->CalculateMinMaxDamage(RANGED_ATTACK, true, true, minDamage, maxDamage);

        int32 dmg = int32((minDamage + maxDamage) / 2.0f * 0.8f);

        if (!target->HasAura(SPELL_HUNTER_BARRAGE, player->GetGUID()))
            dmg /= 2;

        dmg = player->SpellDamageBonusDone(target, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE, GetEffectInfo());
        dmg = target->SpellDamageBonusTaken(player, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE);

        // Barrage deals only 80% of normal damage against player-controlled targets.
        if (target->GetSpellModOwner())
            dmg = CalculatePct(dmg, 80);

        SetHitDamage(dmg);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_barrage::CheckLOS, EFFECT_0, TARGET_UNIT_CONE_ENEMY_24);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_barrage::CheckLOS, EFFECT_1, TARGET_UNIT_CONE_ENEMY_24);
        OnHit += SpellHitFn(spell_hun_barrage::HandleOnHit);
    }
};

// 115939 - Beast Cleave
// Called by Multi-Shot (2643): passes the cleave proc onto the active pet.
// NOTE: the reference also passes it to a second, Survival-only "Hati" companion wolf -
// Player::GetHati() doesn't exist anywhere in ArgusCore (same gap as Kill Command earlier this
// session), dropped for the same reason.
class spell_hun_beast_cleave : public SpellScript
{
    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !player->HasAura(SPELL_HUNTER_BEAST_CLEAVE_AURA))
            return;

        if (Pet* pet = player->GetPet())
            player->CastSpell(pet, SPELL_HUNTER_BEAST_CLEAVE_PROC, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_hun_beast_cleave::HandleAfterCast);
    }
};

// 118455 - Beast Cleave (proc)
// While the pet has this buff, its next non-cleave hit also strikes everyone else the owner is
// in combat with for 75% of that hit's damage.
class spell_hun_beast_cleave_proc : public AuraScript
{
    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (!eventInfo.GetActor() || eventInfo.GetActor()->GetGUID() != GetTarget()->GetGUID())
            return;

        if (eventInfo.GetDamageInfo()->GetSpellInfo() && eventInfo.GetDamageInfo()->GetSpellInfo()->Id == SPELL_HUNTER_BEAST_CLEAVE_DAMAGE)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!GetTarget()->HasAura(aurEff->GetSpellInfo()->Id, player->GetGUID()))
            return;

        int32 bp = int32(eventInfo.GetDamageInfo()->GetDamage() * 0.75f);
        GetTarget()->CastSpell(GetTarget(), SPELL_HUNTER_BEAST_CLEAVE_DAMAGE, CastSpellExtraArgs(SPELLVALUE_BASE_POINT0, bp).SetTriggerFlags(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_hun_beast_cleave_proc::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 204089 - Bullseye
// 35110 - Bombardment
// Gates the talent aura's own DB2 proc data to a critical Multi-Shot only.
class spell_hun_bombardment : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_MULTISHOT && (eventInfo.GetHitMask() & PROC_HIT_CRITICAL);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_bombardment::CheckProc);
    }
};

class spell_hun_bullseye : public AuraScript
{
    static bool CheckEffectProc(AuraScript const&, AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetActionTarget()->HealthBelowPct(aurEff->GetAmount());
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_hun_bullseye::CheckEffectProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 187708 - Carve
// Survival's melee cleave: if the Hunter knows Serpent Sting, also applies its DoT to the
// target. Same interaction as spell_hun_raptor_strike below (Survival's single-target builder).
// 199483 - Camouflage
// Mirrors the stealth aura onto the active pet so it also benefits from Camouflage while the
// Hunter is stealthed.
class spell_hun_camouflage : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_CAMOUFLAGE });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (caster && caster->IsPlayer())
            if (Unit* pet = caster->GetGuardianPet())
                pet->CastSpell(pet, SPELL_HUNTER_CAMOUFLAGE, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (caster && caster->IsPlayer())
            if (Unit* pet = caster->GetGuardianPet())
                pet->RemoveAurasDueToSpell(SPELL_HUNTER_CAMOUFLAGE);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_hun_camouflage::OnApply, EFFECT_0, SPELL_AURA_MOD_STEALTH, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_hun_camouflage::OnRemove, EFFECT_0, SPELL_AURA_MOD_STEALTH, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_hun_carve : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!player || !target)
            return;

        if (player->HasSpell(SPELL_HUNTER_SERPENT_STING))
            player->CastSpell(target, SPELL_HUNTER_SERPENT_STING_DAMAGE, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_carve::HandleOnHit);
    }
};

// 53209 - Chimaera Shot
// Beast Mastery talent during Legion (confirmed active through patch 7.2.5, redesigned for
// Marksmanship only in Shadowlands - genuinely period-appropriate for 7.3.5). The two-headed
// shot randomly deals one of two damage types.
class spell_hun_chimaera_shot : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_CHIMAERA_SHOT_HEAD_1, SPELL_HUNTER_CHIMAERA_SHOT_HEAD_2 });
    }

    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        caster->CastSpell(target, urand(0, 1) ? SPELL_HUNTER_CHIMAERA_SHOT_HEAD_1 : SPELL_HUNTER_CHIMAERA_SHOT_HEAD_2, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_chimaera_shot::HandleOnHit);
    }
};

// 378750 - Cobra Sting
class spell_hun_cobra_sting : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    bool RollProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*procInfo*/)
    {
        return roll_chance_i(GetEffect(EFFECT_1)->GetAmount());
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_hun_cobra_sting::RollProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 5116 - Concussive Shot (attached to 193455 - Cobra Shot and 56641 - Steady Shot)
class spell_hun_concussive_shot : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_HUNTER_CONCUSSIVE_SHOT,  })
            && ValidateSpellEffect({ { SPELL_HUNTER_STEADY_SHOT, EFFECT_2 } });
    }

    void HandleDuration(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();

        if (Aura* concussiveShot = GetHitUnit()->GetAura(SPELL_HUNTER_CONCUSSIVE_SHOT, caster->GetGUID()))
        {
            SpellInfo const* steadyShot = sSpellMgr->AssertSpellInfo(SPELL_HUNTER_STEADY_SHOT, GetCastDifficulty());
            Milliseconds extraDuration = Seconds(steadyShot->GetEffect(EFFECT_2).CalcValue(caster) / 10);
            Milliseconds newDuration = Milliseconds(concussiveShot->GetDuration()) + extraDuration;
            concussiveShot->SetDuration(newDuration.count());
            concussiveShot->SetMaxDuration(newDuration.count());
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_concussive_shot::HandleDuration, EFFECT_FIRST_FOUND, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 459517 - Concussive Shot (attached to 186265 - Aspect of the Turtle and 5384 - Feign Death)
class spell_hun_emergency_salve : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_EMERGENCY_SALVE_TALENT, SPELL_HUNTER_EMERGENCY_SALVE_DISPEL });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_HUNTER_EMERGENCY_SALVE_TALENT);
    }

    void HandleAfterCast() const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_HUNTER_EMERGENCY_SALVE_DISPEL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_hun_emergency_salve::HandleAfterCast);
    }
};

// 109304 - Exhilaration
class spell_hun_exhilaration : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_EXHILARATION_R2, SPELL_HUNTER_LONE_WOLF });
    }

    void HandleOnHit()
    {
        if (GetCaster()->HasAura(SPELL_HUNTER_EXHILARATION_R2) && !GetCaster()->HasAura(SPELL_HUNTER_LONE_WOLF))
            GetCaster()->CastSpell(nullptr, SPELL_HUNTER_EXHILARATION_PET, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_exhilaration::HandleOnHit);
    }
};

// 212431 - Explosive Shot
class spell_hun_explosive_shot : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_EXPLOSIVE_SHOT_DAMAGE });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget(), SPELL_HUNTER_EXPLOSIVE_SHOT_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_explosive_shot::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 236775 - High Explosive Trap
// 9810 - AreatriggerId
struct areatrigger_hun_high_explosive_trap : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnInitialize() override
    {
        if (Unit* caster = at->GetCaster())
            for (AreaTrigger* other : caster->GetAreaTriggers(SPELL_HUNTER_HIGH_EXPLOSIVE_TRAP))
                other->SetDuration(0);
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (caster->IsValidAttackTarget(unit))
            {
                caster->CastSpell(at->GetPosition(), SPELL_HUNTER_HIGH_EXPLOSIVE_TRAP_DAMAGE, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
                at->Remove();
            }
        }
    }
};

// 5384 - Feign Death
// Snapshots health/focus the instant Feign Death applies and restores them when it's removed,
// so nothing that happens while "dead" (regen ticking, a lagging DoT, etc.) can leave the
// Hunter with the wrong values once they stand back up.
// Farstrider
// Proc-driven: a critical hit resets whichever gap-closer the Hunter actually has (Disengage
// or Harpoon).
// NOTE: the reference resets SPELL_HUNTER_DISENGAGE's cooldown in *both* branches - the second
// branch checks `HasSpell(SPELL_HUNTER_HARPOON)` but then calls
// `ResetCooldown(SPELL_HUNTER_DISENGAGE, ...)` again, an apparent copy-paste bug that would
// leave Harpoon's cooldown untouched even when the Hunter has Harpoon instead of Disengage.
// Fixed to reset Harpoon's own cooldown in that branch.
class spell_hun_farstrider : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return (eventInfo.GetHitMask() & PROC_HIT_CRITICAL) != 0;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        if (player->HasSpell(SPELL_HUNTER_DISENGAGE))
            player->GetSpellHistory()->ResetCooldown(SPELL_HUNTER_DISENGAGE, true);

        if (player->HasSpell(SPELL_HUNTER_HARPOON))
            player->GetSpellHistory()->ResetCooldown(SPELL_HUNTER_HARPOON, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_farstrider::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_hun_farstrider::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_hun_feign_death : public AuraScript
{
    uint64 health = 0;
    int32 focus = 0;

    void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        health = GetTarget()->GetHealth();
        focus = GetTarget()->GetPower(POWER_FOCUS);
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (health && focus)
        {
            GetTarget()->SetHealth(health);
            GetTarget()->SetPower(POWER_FOCUS, focus);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_hun_feign_death::HandleEffectApply, EFFECT_0, SPELL_AURA_FEIGN_DEATH, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_hun_feign_death::HandleEffectRemove, EFFECT_0, SPELL_AURA_FEIGN_DEATH, AURA_EFFECT_HANDLE_REAL);
    }
};

// 190925 - Harpoon
// Survival's gap-closer: roots the target, then jumps the Hunter to them.
// 202800 - Flanking Strike
// Trigger for Animal Instincts (204315). Sends the pet in to strike alongside the Hunter: if
// the Hunter isn't currently being attacked, deals bonus damage and the pet's own hit is
// stronger (Flanking Strike Proc); otherwise the pet's hit is the normal-strength variant
// (Flanking Strike Proc Up) and it also generates bonus threat.
// NOTE: drops the reference's "Aspect of the Beast" branch (bonus effect depends on the pet's
// talent specialization via a hardcoded Ferocity/Cunning/Tenacity enum) - same missing-API gap
// as Kill Command earlier this session (ArgusCore's Pet::GetSpecialization() returns a raw DB2
// id, not that enum, and the correct ids weren't available to verify).
class spell_hun_flanking_strike : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* pet = player ? player->GetGuardianPet() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!player || !pet || !target)
            return;

        int32 dmg = GetHitDamage();

        if (player->getAttackers().empty())
        {
            pet->ToCreature()->AI()->AttackStart(target);
            pet->CastSpell(target, SPELL_HUNTER_FLANKING_STRIKE_PROC, true);
            dmg = int32(dmg * 1.5f);
        }
        else
        {
            pet->ToCreature()->AI()->AttackStart(target);
            pet->CastSpell(target, SPELL_HUNTER_FLANKING_STRIKE_PROC_UP, true);
            pet->GetThreatManager().AddThreat(target, 400.0f);
        }

        SetHitDamage(dmg);
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (!caster->HasSpell(SPELL_HUNTER_ANIMAL_INSTINCTS))
            return;

        switch (rand() % 4)
        {
            case 0: caster->GetSpellHistory()->ModifyCooldown(SPELL_HUNTER_HARPOON, -3000); break;
            case 1: caster->GetSpellHistory()->ModifyCooldown(SPELL_HUNTER_ASPECT_OF_THE_EAGLE, -3000); break;
            case 2: caster->GetSpellHistory()->ModifyCooldown(SPELL_HUNTER_MONGOOSE_BITE, -3000); break;
            case 3: caster->GetSpellHistory()->ModifyCooldown(SPELL_HUNTER_FLANKING_STRIKE, -3000); break;
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_flanking_strike::HandleDummy, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
        AfterCast += SpellCastFn(spell_hun_flanking_strike::HandleAfterCast);
    }
};

// 204740 - Flanking Strike (pet damage)
class spell_hun_flanking_strike_proc : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* owner = caster ? caster->GetOwner() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!caster || !owner || !target)
            return;

        // (3.652 * ranged attack power * lowNerf), matching the reference's own formula
        int32 dmg = int32(3.652f * owner->GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER));
        dmg = int32(dmg * (std::min<int32>(owner->GetLevel(), 20) * 0.05f));

        dmg = caster->SpellDamageBonusDone(target, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE, GetEffectInfo());
        dmg = target->SpellDamageBonusTaken(caster, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE);

        SetHitDamage(dmg);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_flanking_strike_proc::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 206933 - Flanking Strike (pet damage, +50%)
class spell_hun_flanking_strike_proc_up : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* owner = caster ? caster->GetOwner() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!caster || !owner || !target)
            return;

        int32 dmg = int32(3.652f * owner->GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER));
        dmg = int32(dmg * (std::min<int32>(owner->GetLevel(), 20) * 0.05f));
        dmg = int32(dmg * 1.5f);

        dmg = caster->SpellDamageBonusDone(target, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE, GetEffectInfo());
        dmg = target->SpellDamageBonusTaken(caster, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE);

        SetHitDamage(dmg);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_flanking_strike_proc_up::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_hun_harpoon : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_HARPOON, SPELL_HUNTER_HARPOON_ROOT });
    }

    void CastHarpoon()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!player || !target)
            return;

        player->CastSpell(target, SPELL_HUNTER_HARPOON_ROOT, true);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!player || !target)
            return;

        WorldLocation pTarget = target->GetWorldLocation();

        float speedZ = 1.8f;
        float speedXY = player->GetExactDist2d(&pTarget) * 10.0f / speedZ;
        player->GetMotionMaster()->MoveJump(pTarget, speedXY, speedZ, EVENT_JUMP);
    }

    void HandleAfterCast()
    {
        // NOTE: matches ArgusCore's own native spell_hun_posthaste class (bound to Disengage),
        // which checks HasAura() rather than the reference's HasSpell() for this exact same
        // talent - kept consistent with that already-working implementation.
        Unit* caster = GetCaster();
        if (caster && caster->HasAura(SPELL_HUNTER_POSTHASTE_TALENT))
            caster->CastSpell(caster, SPELL_HUNTER_POSTHASTE_INCREASE_SPEED, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_hun_harpoon::CastHarpoon);
        OnEffectHitTarget += SpellEffectFn(spell_hun_harpoon::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        AfterCast += SpellCastFn(spell_hun_harpoon::HandleAfterCast);
    }
};

// 185987 - Hunter's Mark
// Gates the talent aura's own DB2 proc data to Auto Shot only - see spell_hun_arcane_shot and
// spell_hun_multi_shot_marking_targets below for where the resulting Marking Targets buff
// actually gets consumed.
class spell_hun_hunters_mark : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_AUTO_SHOT;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_hunters_mark::CheckProc);
    }
};

// 212658 - Hunting Party
class spell_hun_hunting_party : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_HUNTER_EXHILARATION,
            SPELL_HUNTER_EXHILARATION_PET
        });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_HUNTER_EXHILARATION, -Seconds(aurEff->GetAmount()));
        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_HUNTER_EXHILARATION_PET, -Seconds(aurEff->GetAmount()));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_hun_hunting_party::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 462032 - Implosive Trap
// 34378 - AreatriggerId
struct areatrigger_hun_implosive_trap : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnInitialize() override
    {
        if (Unit* caster = at->GetCaster())
            for (AreaTrigger* other : caster->GetAreaTriggers(SPELL_HUNTER_IMPLOSIVE_TRAP))
                other->SetDuration(0);
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (caster->IsValidAttackTarget(unit))
            {
                caster->CastSpell(at->GetPosition(), SPELL_HUNTER_IMPLOSIVE_TRAP_DAMAGE, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
                at->Remove();
            }
        }
    }
};

// 34026 - Kill Command
// NOTE: the reference also implements two Legion companion-pet mechanics ArgusCore has no
// support for, dropped here rather than guessed at: "Aspect of the Beast" (which branches on
// the pet's talent specialization - Ferocity/Cunning/Tenacity - via a hardcoded enum, but
// ArgusCore's Pet::GetSpecialization() returns a raw DB2 ChrSpecialization id instead, and the
// correct ids for those three pet specs weren't available to verify), and the Survival
// Hunter's "Hati" artifact companion wolf (a second, non-Guardian-pet summon entirely -
// Player::GetHati() doesn't exist anywhere in ArgusCore, no engine support for it at all). The
// core "redirect the pet's attack onto the target and have it strike" mechanic below is
// unaffected by either gap.
// 19577 - Intimidation
// Proc-driven: the pet's own melee hit stuns whatever it just struck.
class spell_hun_intimidation : public AuraScript
{
    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = eventInfo.GetDamageInfo() ? eventInfo.GetDamageInfo()->GetAttacker() : nullptr;
        Unit* target = eventInfo.GetDamageInfo() ? eventInfo.GetDamageInfo()->GetVictim() : nullptr;
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_HUNTER_INTIMIDATION_STUN, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_hun_intimidation::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_hun_kill_command : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_KILL_COMMAND, SPELL_HUNTER_KILL_COMMAND_TRIGGER, SPELL_HUNTER_KILL_COMMAND_CHARGE });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        Guardian* pet = caster ? caster->GetGuardianPet() : nullptr;
        Unit* target = GetExplTargetUnit();

        if (!pet || pet->isDead())
            return SPELL_FAILED_NO_PET;

        if (!target)
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (!pet->IsWithinDist(target, 40.0f, true))
            return SPELL_FAILED_OUT_OF_RANGE;

        if (!target->IsWithinLOSInMap(pet))
            return SPELL_FAILED_LINE_OF_SIGHT;

        if (pet->HasAuraType(SPELL_AURA_MOD_STUN) || pet->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
            pet->HasAuraType(SPELL_AURA_MOD_SILENCE) || pet->HasAuraType(SPELL_AURA_MOD_FEAR) ||
            pet->HasAuraType(SPELL_AURA_MOD_FEAR_2))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Guardian* pet = caster ? caster->GetGuardianPet() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!pet || !target)
            return;

        pet->CastSpell(target, SPELL_HUNTER_KILL_COMMAND_TRIGGER, true);

        if (pet->GetVictim())
            pet->AttackStop();
        pet->AI()->AttackStart(target);

        pet->CastSpell(target, SPELL_HUNTER_KILL_COMMAND_CHARGE, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_hun_kill_command::CheckCast);
        OnEffectHit += SpellEffectFn(spell_hun_kill_command::HandleDummy, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 83381 - Kill Command (damage)
// Damage formula ported verbatim from the reference's own "Patch 7.2.5" comment - a real
// Legion patch number, i.e. genuinely period-appropriate for 7.3.5, not independently
// re-derived here.
class spell_hun_kill_command_proc : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Player* owner = caster && caster->GetOwner() ? caster->GetOwner()->ToPlayer() : nullptr;
        if (!owner)
            return;

        float rawDamage = 1.5f * (1.0f + owner->GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) * 3.6f);
        rawDamage *= (0.5f + std::min<int32>(owner->GetLevel(), 20) * 0.025f) * (1.0f + owner->GetRatingBonusValue(CR_VERSATILITY_DAMAGE_DONE) / 100.0f);

        SetHitDamage(int32(rawDamage));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_kill_command_proc::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 199532 - Killer Cobra
// Proc-driven: while Bestial Wrath is active, Cobra Shot resets Kill Command's cooldown.
class spell_hun_killer_cobra : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_COBRA_SHOT;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        if (!caster || !caster->HasAura(SPELL_HUNTER_BESTIAL_WRATH))
            return;

        if (caster->GetSpellHistory()->HasCooldown(SPELL_HUNTER_KILL_COMMAND))
            caster->GetSpellHistory()->ResetCooldown(SPELL_HUNTER_KILL_COMMAND, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_killer_cobra::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_hun_killer_cobra::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 53478 - Last Stand Pet
class spell_hun_last_stand_pet : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_PET_LAST_STAND_TRIGGERED });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.AddSpellBP0(caster->CountPctFromMaxHealth(30));
        caster->CastSpell(caster, SPELL_HUNTER_PET_LAST_STAND_TRIGGERED, args);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_last_stand_pet::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 378016 - Latent Poison
class spell_hun_latent_poison_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_LATENT_POISON_STACK });
    }

    void CalculateDamage()
    {
        if (Aura* stack = GetHitUnit()->GetAura(SPELL_HUNTER_LATENT_POISON_STACK, GetCaster()->GetGUID()))
        {
            SetHitDamage(GetHitDamage() * stack->GetStackAmount());
            stack->Remove();
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_latent_poison_damage::CalculateDamage);
    }
};

// 19434 - Aimed Shot
// 186270 - Raptor Strike
// 217200 - Barbed Shot
// 259387 - Mongoose Bite
class spell_hun_latent_poison_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_LATENT_POISON_STACK, SPELL_HUNTER_LATENT_POISON_DAMAGE });
    }

    void TriggerDamage()
    {
        if (GetHitUnit()->HasAura(SPELL_HUNTER_LATENT_POISON_STACK, GetCaster()->GetGUID()))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_HUNTER_LATENT_POISON_DAMAGE, GetSpell());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_hun_latent_poison_trigger::TriggerDamage);
    }
};

// 336904 - Latent Poison Injectors
class spell_hun_latent_poison_injectors_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_LATENT_POISON_INJECTORS_STACK });
    }

    void CalculateDamage()
    {
        if (Aura* stack = GetHitUnit()->GetAura(SPELL_HUNTER_LATENT_POISON_INJECTORS_STACK, GetCaster()->GetGUID()))
        {
            SetHitDamage(GetHitDamage() * stack->GetStackAmount());
            stack->Remove();
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_latent_poison_injectors_damage::CalculateDamage);
    }
};

// 186270 - Raptor Strike
// 259387 - Mongoose Bite
class spell_hun_latent_poison_injectors_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_LATENT_POISON_INJECTORS_STACK, SPELL_HUNTER_LATENT_POISON_INJECTORS_DAMAGE });
    }

    void TriggerDamage()
    {
        if (GetHitUnit()->HasAura(SPELL_HUNTER_LATENT_POISON_INJECTORS_STACK, GetCaster()->GetGUID()))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_HUNTER_LATENT_POISON_INJECTORS_DAMAGE, GetSpell());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_hun_latent_poison_injectors_trigger::TriggerDamage);
    }
};

// 1217788 - Manhunter
// 194595 - Lock and Load
// Gates the talent aura's own DB2 proc data to Auto Shot only.
class spell_hun_lock_and_load : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_AUTO_SHOT;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_lock_and_load::CheckProc);
    }
};

class spell_hun_manhunter : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_GREVIOUS_INJURY });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetProcTarget()->IsPlayer();
    }

    static void HandleEffectProc(AuraScript const&, AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActionTarget(), SPELL_HUNTER_GREVIOUS_INJURY, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_manhunter::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_hun_manhunter::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 185901 - Marked Shot
// Consumes Hunter's Mark on any valid attack target in range, dealing bonus damage and applying
// Vulnerable. The reference used a Player::GetAttackableUnitListInRange() helper that doesn't
// exist in ArgusCore (same gap as Monk's Chi Torpedo earlier this session) - worked around with
// the same grid-search pattern used there.
class spell_hun_marked_shot : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_MARKED_SHOT, SPELL_HUNTER_MARKED_SHOT_DAMAGE, SPELL_HUNTER_HUNTERS_MARK_AURA, SPELL_HUNTER_HUNTERS_MARK_AURA_2, SPELL_HUNTER_VULNERABLE });
    }

    void HandleOnCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_HUNTER_MARKED_SHOT, GetCastDifficulty());
        float radius = spellInfo->GetEffect(EFFECT_0).CalcRadius(caster);

        std::list<Unit*> targets;
        Trinity::AnyUnitInObjectRangeCheck check(caster, radius);
        Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(caster, targets, check);
        Cell::VisitAllObjects(caster, searcher, radius);

        for (Unit* target : targets)
        {
            if (!caster->IsValidAttackTarget(target))
                continue;

            if (target->HasAura(SPELL_HUNTER_HUNTERS_MARK_AURA, caster->GetGUID()))
            {
                caster->CastSpell(target, SPELL_HUNTER_MARKED_SHOT_DAMAGE, true);
                caster->CastSpell(target, SPELL_HUNTER_VULNERABLE, true);
                caster->RemoveAura(SPELL_HUNTER_HUNTERS_MARK_AURA_2);
            }
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_hun_marked_shot::HandleOnCast);
    }
};

// 223138 - Marking Targets
// The reference's own DoCheckProc always returns false here, deliberately blocking this aura's
// generic proc path entirely - Arcane Shot/Multi-Shot instead check for its presence directly
// via HasAura() rather than through a proc. Ported as-is; the reference source's own comment
// confirms this is intentional, not dead code left behind by mistake.
class spell_hun_marking_targets : public AuraScript
{
    bool CheckProc(ProcEventInfo& /*eventInfo*/)
    {
        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_marking_targets::CheckProc);
    }
};

// 53271 - Masters Call
class spell_hun_masters_call : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_0 } })
            && ValidateSpellInfo({ SPELL_HUNTER_MASTERS_CALL_TRIGGERED, uint32(spellInfo->GetEffect(EFFECT_0).CalcValue()) });
    }

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult DoCheckCast()
    {
        Guardian* pet = GetCaster()->ToPlayer()->GetGuardianPet();
        ASSERT(pet); // checked in Spell::CheckCast

        if (!pet->IsPet() || !pet->IsAlive())
            return SPELL_FAILED_NO_PET;

        // Do a mini Spell::CheckCasterAuras on the pet, no other way of doing this
        SpellCastResult result = SPELL_CAST_OK;
        uint32 const unitflag = pet->GetUInt32Value(UNIT_FIELD_FLAGS);
        if (!pet->GetCharmerGUID().IsEmpty())
            result = SPELL_FAILED_CHARMED;
        else if (unitflag & UNIT_FLAG_STUNNED)
            result = SPELL_FAILED_STUNNED;
        else if (unitflag & UNIT_FLAG_FLEEING)
            result = SPELL_FAILED_FLEEING;
        else if (unitflag & UNIT_FLAG_CONFUSED)
            result = SPELL_FAILED_CONFUSED;

        if (result != SPELL_CAST_OK)
            return result;

        Unit* target = GetExplTargetUnit();
        if (!target)
            return SPELL_FAILED_BAD_TARGETS;

        if (!pet->IsWithinLOSInMap(target))
            return SPELL_FAILED_LINE_OF_SIGHT;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->ToPlayer()->GetPet()->CastSpell(GetHitUnit(), GetEffectValue(), true);
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        GetHitUnit()->CastSpell(nullptr, SPELL_HUNTER_MASTERS_CALL_TRIGGERED, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_hun_masters_call::DoCheckCast);

        OnEffectHitTarget += SpellEffectFn(spell_hun_masters_call::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnEffectHitTarget += SpellEffectFn(spell_hun_masters_call::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 34477 - Misdirection
class spell_hun_misdirection : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_MISDIRECTION_PROC });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEFAULT || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_INTERRUPT)
            return;

        if (!GetTarget()->HasAura(SPELL_HUNTER_MISDIRECTION_PROC))
            GetTarget()->GetThreatManager().UnregisterRedirectThreat(SPELL_HUNTER_MISDIRECTION);
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SPELL_HUNTER_MISDIRECTION_PROC, aurEff);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hun_misdirection::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectProc += AuraEffectProcFn(spell_hun_misdirection::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 35079 - Misdirection (Proc)
class spell_hun_misdirection_proc : public AuraScript
{
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->GetThreatManager().UnregisterRedirectThreat(SPELL_HUNTER_MISDIRECTION);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hun_misdirection_proc::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 2643 - Multi-Shot
// 190928 - Mongoose Bite
// Survival's finisher: reapplies Mongoose Fury, preserving its remaining duration if it was
// already active (so re-applying mid-window doesn't reset the stacking-speed-window timer).
class spell_hun_mongoose_bite : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_MONGOOSE_FURY });
    }

    void ApplyBuff()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 duration = 0;
        if (Aura* existing = caster->GetAura(SPELL_HUNTER_MONGOOSE_FURY))
            duration = existing->GetDuration();

        caster->CastSpell(caster, SPELL_HUNTER_MONGOOSE_FURY, true);

        if (duration)
            if (Aura* aur = caster->GetAura(SPELL_HUNTER_MONGOOSE_FURY))
                aur->SetDuration(duration);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_hun_mongoose_bite::ApplyBuff);
    }
};

// 201075 - Mortal Wounds
// Survival passive: Lacerate landing has a chance to restore a Mongoose Bite charge.
// NOTE: the reference's CheckProc gates on `eventInfo.GetHitMask() & PROC_HIT_NONE` -
// PROC_HIT_NONE is 0x0000000 (a "no flags" sentinel), so ANDing any hit mask against it is
// always 0/false, meaning this proc could never fire at all in the reference as written. Fixed
// to a bare triggering-spell-id check instead, matching the exact pattern its sibling class
// (spell_hun_hunters_mark above) already uses correctly.
// Also drops the reference's explicit charge-UI-update push after restoring the charge -
// ArgusCore's SpellHistory only exposes that as a private send helper, not a public method, so
// this relies on the charge state's normal sync path instead - a client-UI-latency
// simplification, not a functional gap (the charge is still restored either way).
class spell_hun_mortal_wounds : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_LACERATE;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player)
            return;

        if (SpellInfo const* mongooseBite = sSpellMgr->GetSpellInfo(SPELL_HUNTER_MONGOOSE_BITE, DIFFICULTY_NONE))
            player->GetSpellHistory()->RestoreCharge(mongooseBite->ChargeCategoryId);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_mortal_wounds::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_hun_mortal_wounds::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_hun_multi_shot : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_MULTI_SHOT_FOCUS });
    }

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleOnHit()
    {
        // We need to check hunter's spec because it doesn't generate focus on other specs than MM
        if (GetCaster()->ToPlayer()->GetPrimarySpecialization() == ChrSpecialization::HunterMarksmanship)
            GetCaster()->CastSpell(GetCaster(), SPELL_HUNTER_MULTI_SHOT_FOCUS, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_multi_shot::HandleOnHit);
    }
};

// 2643 - Multi-Shot (Marking Targets tie-in)
// A second, independent script bound to the same spell id as the existing spell_hun_multi_shot
// above (which handles unrelated Marksmanship focus generation) - this only adds the separate
// Hunter's Mark/Marking Targets interaction, without touching that already-working class.
class spell_hun_multi_shot_marking_targets : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->HasAura(SPELL_HUNTER_MARKING_TARGETS))
            return;

        caster->CastSpell(caster, SPELL_HUNTER_HUNTERS_MARK_AURA_2, true);
        caster->RemoveAurasDueToSpell(SPELL_HUNTER_MARKING_TARGETS);

        if (Unit* target = GetHitUnit())
            caster->CastSpell(target, SPELL_HUNTER_HUNTERS_MARK_AURA, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_multi_shot_marking_targets::HandleOnHit);
    }
};

// 55709 - Pet Heart of the Phoenix
// 206685 - Cobra Spit (pet)
class spell_hun_pet_cobra_spit : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* owner = caster ? caster->GetOwner() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!caster || !owner || !target)
            return;

        int32 dmg = int32(1 + owner->GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) * 0.2f);

        dmg = caster->SpellDamageBonusDone(target, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE, GetEffectInfo());
        dmg = target->SpellDamageBonusTaken(caster, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE);

        SetHitDamage(dmg);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_pet_cobra_spit::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_hun_pet_heart_of_the_phoenix : public SpellScript
{
    bool Load() override
    {
        if (!GetCaster()->IsPet())
            return false;
        return true;
    }

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED, SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (Unit* owner = caster->GetOwner())
        {
            if (!caster->HasAura(SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF))
            {
                CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
                args.AddSpellBP0(100);
                owner->CastSpell(caster, SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED, args);
                caster->CastSpell(caster, SPELL_HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF, true);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_pet_heart_of_the_phoenix::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 781 - Disengage
// 63900 - Thunderstomp (pet)
class spell_hun_pet_thunderstomp : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* owner = caster ? caster->GetOwner() : nullptr;
        Unit* target = GetHitUnit();
        if (!caster || !owner || !target)
            return;

        int32 dmg = int32(1.5f * (owner->GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) * 0.250f));

        dmg = caster->SpellDamageBonusDone(target, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE, GetEffectInfo());
        dmg = target->SpellDamageBonusTaken(caster, GetSpellInfo(), dmg, SPELL_DIRECT_DAMAGE);

        SetHitDamage(dmg);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_pet_thunderstomp::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_hun_posthaste : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_POSTHASTE_TALENT, SPELL_HUNTER_POSTHASTE_INCREASE_SPEED });
    }

    void HandleAfterCast()
    {
        if (GetCaster()->HasAura(SPELL_HUNTER_POSTHASTE_TALENT))
        {
            GetCaster()->RemoveMovementImpairingAuras(true);
            GetCaster()->CastSpell(GetCaster(), SPELL_HUNTER_POSTHASTE_INCREASE_SPEED, GetSpell());
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_hun_posthaste::HandleAfterCast);
    }
};

// 257044 - Rapid Fire
// 200108 - Ranger's Net
// When the root wears off, grants the target a burst of movement speed.
class spell_hun_rangers_net : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_RANGERS_NET_INCREASE_SPEED });
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(GetTarget(), SPELL_HUNTER_RANGERS_NET_INCREASE_SPEED, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hun_rangers_net::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_ROOT, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_hun_rapid_fire : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_RAPID_FIRE_DAMAGE });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget(), SPELL_HUNTER_RAPID_FIRE_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_rapid_fire::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 257045 - Rapid Fire Damage
class spell_hun_rapid_fire_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_RAPID_FIRE_ENERGIZE });
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(nullptr, SPELL_HUNTER_RAPID_FIRE_ENERGIZE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_rapid_fire_damage::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 186270 - Raptor Strike
// Survival's melee single-target builder: if the Hunter knows Serpent Sting, also applies its
// DoT to the target.
class spell_hun_raptor_strike : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!player || !target)
            return;

        if (player->HasSpell(SPELL_HUNTER_SERPENT_STING))
            player->CastSpell(target, SPELL_HUNTER_SERPENT_STING_DAMAGE, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_raptor_strike::HandleOnHit);
    }
};

// 385539 - Rejuvenating Wind
class spell_hun_rejuvenating_wind : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_REJUVENATING_WIND_HEAL })
            && sSpellMgr->AssertSpellInfo(SPELL_HUNTER_REJUVENATING_WIND_HEAL, DIFFICULTY_NONE)->GetMaxTicks() > 0;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& /*procEvent*/)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();

        uint32 ticks = sSpellMgr->AssertSpellInfo(SPELL_HUNTER_REJUVENATING_WIND_HEAL, DIFFICULTY_NONE)->GetMaxTicks();
        int32 heal = CalculatePct(caster->GetMaxHealth(), aurEff->GetAmount()) / ticks;

        caster->CastSpell(caster, SPELL_HUNTER_REJUVENATING_WIND_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, heal } }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_hun_rejuvenating_wind::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 53480 - Roar of Sacrifice
class spell_hun_roar_of_sacrifice : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROAR_OF_SACRIFICE_TRIGGERED });
    }

    bool CheckProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !(damageInfo->GetSchoolMask() & aurEff->GetMiscValue()))
            return false;

        if (!GetCaster())
            return false;

        return true;
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), aurEff->GetAmount()));
        eventInfo.GetActor()->CastSpell(GetCaster(), SPELL_ROAR_OF_SACRIFICE_TRIGGERED, args);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_hun_roar_of_sacrifice::CheckProc, EFFECT_1, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_hun_roar_of_sacrifice::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 37506 - Scatter Shot
class spell_hun_scatter_shot : public SpellScript
{
    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        // break Auto Shot and autohit
        caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
        caster->AttackStop();
        caster->SendAttackSwingCancelAttack();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_scatter_shot::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 459455 - Scout's Instincts (attached to 186257 - Aspect of the Cheetah)
class spell_hun_scouts_instincts : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_SCOUTS_INSTINCTS })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } })
            && spellInfo->GetEffect(EFFECT_1).IsAura(SPELL_AURA_MOD_MINIMUM_SPEED);
    }

    bool Load() override
    {
        return !GetCaster()->HasAura(SPELL_HUNTER_SCOUTS_INSTINCTS);
    }

    static void HandleMinSpeed(SpellScript const&, WorldObject*& target)
    {
        target = nullptr;
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_hun_scouts_instincts::HandleMinSpeed, EFFECT_1, TARGET_UNIT_CASTER);
    }
};

// 459533 - Scrappy
class spell_hun_scrappy : public AuraScript
{
    static constexpr std::array<uint32, 3> AffectedSpellIds = { SPELL_HUNTER_BINDING_SHOT, SPELL_HUNTER_INTIMIDATION, SPELL_HUNTER_INTIMIDATION_MARKSMANSHIP };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(AffectedSpellIds);
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/) const
    {
        for (uint32 spellId : AffectedSpellIds)
            GetTarget()->GetSpellHistory()->ModifyCooldown(spellId, -Milliseconds(aurEff->GetAmount()));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_hun_scrappy::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 56641 - Steady Shot
// 201078 - Snake Hunter
// Passive: resets Mongoose Bite's charges when this dummy effect fires.
class spell_hun_snake_hunter : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_MONGOOSE_BITE });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        if (SpellInfo const* mongooseBite = sSpellMgr->GetSpellInfo(SPELL_HUNTER_MONGOOSE_BITE, DIFFICULTY_NONE))
            player->GetSpellHistory()->ResetCharges(mongooseBite->ChargeCategoryId);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hun_snake_hunter::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 206817 - Sentinel
// Applies Hunter's Mark to every attackable target in range. Same missing-engine-helper
// workaround as Marked Shot above (Player::GetAttackableUnitListInRange() doesn't exist).
class spell_hun_sentinel : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_SENTINEL, SPELL_HUNTER_HUNTERS_MARK_AURA, SPELL_HUNTER_HUNTERS_MARK_AURA_2 });
    }

    void HandleOnCast()
    {
        Unit* caster = GetCaster();
        if (!caster || !GetExplTargetUnit())
            return;

        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_HUNTER_SENTINEL, GetCastDifficulty());
        float radius = spellInfo->GetEffect(EFFECT_0).CalcRadius(caster);

        std::list<Unit*> targets;
        Trinity::AnyUnitInObjectRangeCheck check(caster, radius);
        Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(caster, targets, check);
        Cell::VisitAllObjects(caster, searcher, radius);

        for (Unit* target : targets)
            if (caster->IsValidAttackTarget(target))
                caster->CastSpell(target, SPELL_HUNTER_HUNTERS_MARK_AURA, true);

        caster->CastSpell(caster, SPELL_HUNTER_HUNTERS_MARK_AURA_2, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_hun_sentinel::HandleOnCast);
    }
};

class spell_hun_steady_shot : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_STEADY_SHOT_FOCUS, SPELL_HUNTER_AIMED_SHOT, SPELL_HUNTER_MARKSMANSHIP_HUNTER_AURA });
    }

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleOnHit() const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_HUNTER_STEADY_SHOT_FOCUS, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });

        if (GetCaster()->HasAura(SPELL_HUNTER_MARKSMANSHIP_HUNTER_AURA))
            GetCaster()->GetSpellHistory()->ModifyCooldown(SPELL_HUNTER_AIMED_SHOT, Milliseconds(-GetEffectInfo(EFFECT_1).CalcValue()));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_steady_shot::HandleOnHit);
    }
};

// 191241 - Sticky Bomb
// When the root wears off, detonates a proc on whoever it was rooting.
class spell_hun_sticky_bomb : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_STICKY_BOMB_PROC });
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(GetTarget(), SPELL_HUNTER_STICKY_BOMB_PROC, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hun_sticky_bomb::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 260367 - Streamline (attached to 257044 - Rapid Fire)
class spell_hun_streamline : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_STREAMLINE_TALENT, SPELL_HUNTER_STREAMLINE_BUFF });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_HUNTER_STREAMLINE_TALENT);
    }

    void HandleAfterCast() const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_HUNTER_STREAMLINE_BUFF, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_hun_streamline::HandleAfterCast);
    }
};

// 391559 - Surging Shots
class spell_hun_surging_shots : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_RAPID_FIRE });
    }

    void HandleProc(ProcEventInfo const& /*eventInfo*/) const
    {
        GetTarget()->GetSpellHistory()->ResetCooldown(SPELL_HUNTER_RAPID_FIRE, true);
    }

    void Register() override
    {
        OnProc += AuraProcFn(spell_hun_surging_shots::HandleProc);
    }
};

// 1515 - Tame Beast
class spell_hun_tame_beast : public SpellScript
{
    static constexpr uint32 CallPetSpellIds[MAX_ACTIVE_PETS] =
    {
        883,
        83242,
        83243,
        83244,
        83245,
    };

    SpellCastResult CheckCast()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (!caster)
            return SPELL_FAILED_DONT_REPORT;

        if (!GetExplTargetUnit())
            return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

        if (Creature* target = GetExplTargetUnit()->ToCreature())
        {
            if (target->GetLevelForTarget(caster) > caster->GetLevel())
                return SPELL_FAILED_HIGHLEVEL;

            // use SMSG_PET_TAME_FAILURE?
            if (!target->GetCreatureTemplate()->IsTameable(caster->CanTameExoticPets(), target->GetCreatureDifficulty()))
                return SPELL_FAILED_BAD_TARGETS;

            if (PetStable const* petStable = caster->GetPetStable())
            {
                if (petStable->CurrentPetIndex)
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                auto freeSlotItr = std::find_if(petStable->ActivePets.begin(), petStable->ActivePets.end(), [](Optional<PetStable::PetInfo> const& petInfo)
                {
                    return !petInfo.has_value();
                });

                if (freeSlotItr == petStable->ActivePets.end())
                {
                    caster->SendTameFailure(PetTameResult::TooMany);
                    return SPELL_FAILED_DONT_REPORT;
                }

                // Check for known Call Pet X spells
                std::size_t freeSlotIndex = std::distance(petStable->ActivePets.begin(), freeSlotItr);
                if (!caster->HasSpell(CallPetSpellIds[freeSlotIndex]))
                {
                    caster->SendTameFailure(PetTameResult::TooMany);
                    return SPELL_FAILED_DONT_REPORT;
                }
            }

            if (!caster->GetCharmedGUID().IsEmpty())
                return SPELL_FAILED_ALREADY_HAVE_CHARM;

            if (!target->GetOwnerGUID().IsEmpty())
            {
                caster->SendTameFailure(PetTameResult::CreatureAlreadyOwned);
                return SPELL_FAILED_DONT_REPORT;
            }
        }
        else
            return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_hun_tame_beast::CheckCast);
    }
};

// 187700 - Tar Trap
// 4436 - AreatriggerId
struct areatrigger_hun_tar_trap : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
            if (caster->IsValidAttackTarget(unit))
                caster->CastSpell(unit, SPELL_HUNTER_TAR_TRAP_SLOW, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
    }

    void OnUnitExit(Unit* unit) override
    {
        unit->RemoveAurasDueToSpell(SPELL_HUNTER_TAR_TRAP_SLOW, at->GetCasterGuid());
    }
};

// 187699 - Tar Trap
// 4435 - AreatriggerId
struct areatrigger_hun_tar_trap_activate : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnInitialize() override
    {
        if (Unit* caster = at->GetCaster())
            for (AreaTrigger* other : caster->GetAreaTriggers(SPELL_HUNTER_TAR_TRAP))
                other->SetDuration(0);
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (caster->IsValidAttackTarget(unit))
            {
                caster->CastSpell(at->GetPosition(), SPELL_HUNTER_TAR_TRAP_AREATRIGGER, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
                at->Remove();
            }
        }
    }
};

// 67151 - Item - Hunter T9 4P Bonus (Steady Shot)
class spell_hun_t9_4p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_T9_4P_GREATNESS });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetActor()->GetTypeId() == TYPEID_PLAYER && eventInfo.GetActor()->ToPlayer()->GetPet())
            return true;
        return false;
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();

        caster->CastSpell(caster->ToPlayer()->GetPet(), SPELL_HUNTER_T9_4P_GREATNESS, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_t9_4p_bonus::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_hun_t9_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 185789 - Wild Call
// Proc-driven: a critical Auto Shot has a chance to reset Dire Beast's and Dire Frenzy's
// cooldowns if either is currently on cooldown.
class spell_hun_wild_call : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_AUTO_SHOT && (eventInfo.GetHitMask() & PROC_HIT_CRITICAL);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        if (player->GetSpellHistory()->HasCooldown(SPELL_HUNTER_DIRE_BEAST_GENERIC))
            player->GetSpellHistory()->ResetCooldown(SPELL_HUNTER_DIRE_BEAST_GENERIC, true);

        if (player->GetSpellHistory()->HasCooldown(SPELL_HUNTER_DIRE_FRENZY))
            player->GetSpellHistory()->ResetCooldown(SPELL_HUNTER_DIRE_FRENZY, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_wild_call::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_hun_wild_call::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// Called by 136 - Mend Pet
// 201082 - Way of the Mok'Nathal
// Gates the talent aura's own DB2 proc data to Raptor Strike only.
// 199527 - True Aim
// Gates the talent aura's own DB2 proc data to Aimed Shot or Arcane Shot only.
class spell_hun_true_aim : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && (eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_AIMED_SHOT || eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_ARCANE_SHOT);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_true_aim::CheckProc);
    }
};

class spell_hun_way_of_the_moknathal : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_RAPTOR_STRIKE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_HUNTER_RAPTOR_STRIKE;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_hun_way_of_the_moknathal::CheckProc);
    }
};

class spell_hun_wilderness_medicine : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HUNTER_WILDERNESS_MEDICINE_TALENT, SPELL_HUNTER_WILDERNESS_MEDICINE_DISPEL });
    }

    bool Load() override
    {
        Unit const* caster = GetCaster();
        if (!caster)
            return false;

        AuraEffect const* wildernessMedicine = GetCaster()->GetAuraEffect(SPELL_HUNTER_WILDERNESS_MEDICINE_TALENT, EFFECT_1);
        if (!wildernessMedicine)
            return false;

        _dispelChance = wildernessMedicine->GetAmount();
        return true;
    }

    void OnPeriodic(AuraEffect const* aurEff) const
    {
        if (Unit* caster = GetCaster())
            if (roll_chance_i(_dispelChance))
                caster->CastSpell(GetTarget(), SPELL_HUNTER_WILDERNESS_MEDICINE_DISPEL, CastSpellExtraArgsInit{
                    .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                    .TriggeringAura = aurEff
                });
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_wilderness_medicine::OnPeriodic, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH);
    }

    int32 _dispelChance = 0;
};

void AddSC_hunter_spell_scripts()
{
    RegisterSpellScript(spell_hun_a_murder_of_crows);
    RegisterSpellScript(spell_hun_arcane_shot);
    RegisterSpellScript(spell_hun_aspect_cheetah);
    RegisterSpellScript(spell_hun_aspect_of_the_fox);
    RegisterSpellScript(spell_hun_aspect_of_the_turtle);
    RegisterSpellScript(spell_hun_barrage);
    RegisterSpellScript(spell_hun_beast_cleave);
    RegisterSpellScript(spell_hun_beast_cleave_proc);
    RegisterSpellScript(spell_hun_bombardment);
    RegisterSpellScript(spell_hun_bullseye);
    RegisterSpellScript(spell_hun_camouflage);
    RegisterSpellScript(spell_hun_carve);
    RegisterSpellScript(spell_hun_chimaera_shot);
    RegisterSpellScript(spell_hun_cobra_sting);
    RegisterSpellScript(spell_hun_concussive_shot);
    RegisterSpellScript(spell_hun_emergency_salve);
    RegisterSpellScript(spell_hun_exhilaration);
    RegisterSpellScript(spell_hun_explosive_shot);
    RegisterAreaTriggerAI(areatrigger_hun_high_explosive_trap);
    RegisterSpellScript(spell_hun_farstrider);
    RegisterSpellScript(spell_hun_feign_death);
    RegisterSpellScript(spell_hun_flanking_strike);
    RegisterSpellScript(spell_hun_flanking_strike_proc);
    RegisterSpellScript(spell_hun_flanking_strike_proc_up);
    RegisterSpellScript(spell_hun_harpoon);
    RegisterSpellScript(spell_hun_hunters_mark);
    RegisterSpellScript(spell_hun_hunting_party);
    RegisterSpellScript(spell_hun_intimidation);
    RegisterSpellScript(spell_hun_kill_command);
    RegisterSpellScript(spell_hun_kill_command_proc);
    RegisterSpellScript(spell_hun_killer_cobra);
    RegisterAreaTriggerAI(areatrigger_hun_implosive_trap);
    RegisterSpellScript(spell_hun_last_stand_pet);
    RegisterSpellScript(spell_hun_latent_poison_damage);
    RegisterSpellScript(spell_hun_latent_poison_trigger);
    RegisterSpellScript(spell_hun_latent_poison_injectors_damage);
    RegisterSpellScript(spell_hun_latent_poison_injectors_trigger);
    RegisterSpellScript(spell_hun_lock_and_load);
    RegisterSpellScript(spell_hun_manhunter);
    RegisterSpellScript(spell_hun_marked_shot);
    RegisterSpellScript(spell_hun_marking_targets);
    RegisterSpellScript(spell_hun_masters_call);
    RegisterSpellScript(spell_hun_misdirection);
    RegisterSpellScript(spell_hun_misdirection_proc);
    RegisterSpellScript(spell_hun_mongoose_bite);
    RegisterSpellScript(spell_hun_mortal_wounds);
    RegisterSpellScript(spell_hun_multi_shot);
    RegisterSpellScript(spell_hun_multi_shot_marking_targets);
    RegisterSpellScript(spell_hun_pet_cobra_spit);
    RegisterSpellScript(spell_hun_pet_heart_of_the_phoenix);
    RegisterSpellScript(spell_hun_pet_thunderstomp);
    RegisterSpellScript(spell_hun_posthaste);
    RegisterSpellScript(spell_hun_rangers_net);
    RegisterSpellScript(spell_hun_rapid_fire);
    RegisterSpellScript(spell_hun_rapid_fire_damage);
    RegisterSpellScript(spell_hun_raptor_strike);
    RegisterSpellScript(spell_hun_rejuvenating_wind);
    RegisterSpellScript(spell_hun_roar_of_sacrifice);
    RegisterSpellScript(spell_hun_scatter_shot);
    RegisterSpellScript(spell_hun_scouts_instincts);
    RegisterSpellScript(spell_hun_scrappy);
    RegisterSpellScript(spell_hun_sentinel);
    RegisterSpellScript(spell_hun_snake_hunter);
    RegisterSpellScript(spell_hun_steady_shot);
    RegisterSpellScript(spell_hun_sticky_bomb);
    RegisterSpellScript(spell_hun_streamline);
    RegisterSpellScript(spell_hun_surging_shots);
    RegisterSpellScript(spell_hun_tame_beast);
    RegisterAreaTriggerAI(areatrigger_hun_tar_trap);
    RegisterAreaTriggerAI(areatrigger_hun_tar_trap_activate);
    RegisterSpellScript(spell_hun_t9_4p_bonus);
    RegisterSpellScript(spell_hun_true_aim);
    RegisterSpellScript(spell_hun_wild_call);
    RegisterSpellScript(spell_hun_way_of_the_moknathal);
    RegisterSpellScript(spell_hun_wilderness_medicine);
}
