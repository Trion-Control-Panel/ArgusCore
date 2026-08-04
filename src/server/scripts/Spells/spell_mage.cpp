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
 * Scripts for spells with SPELLFAMILY_MAGE and SPELLFAMILY_GENERIC spells used by mage players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_mage_".
 */

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "DB2Stores.h"
#include "GridNotifiers.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

enum MageSpells
{
    SPELL_MAGE_ALTER_TIME_AURA                   = 110909,
    SPELL_MAGE_ALTER_TIME_VISUAL                 = 347402,
    SPELL_MAGE_ARCANE_ALTER_TIME_AURA            = 342246,
    SPELL_MAGE_ARCANE_BARRAGE                    = 44425,
    SPELL_MAGE_ARCANE_BARRAGE_ENERGIZE           = 321529,
    SPELL_MAGE_ARCANE_BARRAGE_R3                 = 321526,
    SPELL_MAGE_ARCANE_BARRAGE_TRIGGERED          = 241241,
    SPELL_MAGE_ENHANCED_PYROTECHNICS_AURA        = 157644,
    SPELL_MAGE_ARCANE_BLAST                      = 30451,
    SPELL_MAGE_ARCANE_CHARGE                     = 36032,
    SPELL_MAGE_ARCANE_MAGE                       = 137021,
    SPELL_MAGE_ARCANE_MISSILES                   = 5143,
    SPELL_MAGE_ARCANE_MISSILES_DAMAGE            = 7268,
    SPELL_MAGE_ARCANE_MISSILES_POWER             = 208030,
    SPELL_MAGE_ARCANE_MISSILES_CHARGES           = 79683,
    SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE        = 170571,
    SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO        = 79808,
    SPELL_MAGE_ARCANE_MISSILES_VISUAL_THREE      = 170572,
    SPELL_MAGE_PRESENCE_OF_MIND                  = 205025,
    SPELL_MAGE_BLAZING_BARRIER                   = 235313,
    SPELL_MAGE_BLAZING_BARRIER_TRIGGER           = 235314,
    SPELL_MAGE_BLAZING_SOUL                      = 235365,
    SPELL_MAGE_BLINK                             = 1953,
    SPELL_MAGE_BLIZZARD_DAMAGE                   = 190357,
    SPELL_MAGE_BLIZZARD_SLOW                     = 12486,
    SPELL_MAGE_BRAIN_FREEZE                      = 190447,
    SPELL_MAGE_BRAIN_FREEZE_AURA                 = 190446,
    SPELL_MAGE_CAUTERIZE_DOT                     = 87023,
    SPELL_MAGE_CAUTERIZED                        = 87024,
    SPELL_MAGE_CHILLED                           = 205708,
    SPELL_MAGE_COMBUSTION                        = 190319,
    SPELL_MAGE_COMET_STORM_DAMAGE                = 153596,
    SPELL_MAGE_COMET_STORM_VISUAL                = 228601,
    SPELL_MAGE_CONE_OF_COLD                      = 120,
    SPELL_MAGE_CONE_OF_COLD_SLOW                 = 212792,
    SPELL_MAGE_CONJURE_REFRESHMENT               = 116136,
    SPELL_MAGE_CONJURE_REFRESHMENT_TABLE         = 167145,
    SPELL_MAGE_DISPLACEMENT_BEACON               = 212799,
    SPELL_MAGE_DRAGONHAWK_FORM                   = 32818,
    SPELL_MAGE_DRAGONS_BREATH                    = 31661,
    SPELL_MAGE_ALEXSTRASZAS_FURY                 = 235870,
    SPELL_MAGE_EROSION_TIMER                     = 210154,
    SPELL_MAGE_EROSION_AURASTATE                 = 210134,
    SPELL_MAGE_FINGERS_OF_FROST                  = 44544,
    SPELL_MAGE_FIRE_BLAST                        = 108853,
    SPELL_MAGE_FIREBALL                          = 133,
    SPELL_MAGE_FIRESTARTER                       = 205026,
    SPELL_MAGE_FLAME_PATCH_AREATRIGGER           = 205470,
    SPELL_MAGE_FLAME_PATCH_DAMAGE                = 205472,
    SPELL_MAGE_FLAME_PATCH_TALENT                = 205037,
    SPELL_MAGE_FLAMESTRIKE                       = 2120,
    SPELL_MAGE_FLURRY_DAMAGE                     = 228596,
    SPELL_MAGE_FROST_BOMB_AURA                    = 112948,
    SPELL_MAGE_FROST_BOMB_TRIGGERED               = 113092,
    SPELL_MAGE_FROST_NOVA                        = 122,
    SPELL_MAGE_GIRAFFE_FORM                      = 32816,
    SPELL_MAGE_GLACIAL_INSULATION                = 235297,
    SPELL_MAGE_HEATING_UP                        = 48107,
    SPELL_MAGE_HOT_STREAK                        = 48108,
    SPELL_MAGE_HYPOTHERMIA                       = 41425,
    SPELL_MAGE_ICE_BARRIER                       = 11426,
    SPELL_MAGE_ICE_BLOCK                         = 45438,
    SPELL_MAGE_ICE_FLOES                         = 108839,
    SPELL_MAGE_ICE_LANCE                         = 30455,
    SPELL_MAGE_ICE_NOVA                          = 157997,
    SPELL_MAGE_IGNITE                            = 12654,
    SPELL_MAGE_JOUSTER                           = 214626,
    SPELL_MAGE_FINGERS_OF_FROST_VISUAL_UI        = 126084,
    SPELL_MAGE_INCANTERS_FLOW                    = 116267,
    SPELL_MAGE_PHOENIX_FLAMES                    = 194466,
    SPELL_MAGE_PYROBLAST                         = 11366,
    SPELL_MAGE_PYROBLAST_CLEARCASTING_DRIVER     = 44448,
    SPELL_MAGE_SCORCH                            = 2948,
    SPELL_MAGE_LIVING_BOMB_EXPLOSION             = 44461,
    SPELL_MAGE_LIVING_BOMB_PERIODIC              = 217694,
    SPELL_MAGE_MANA_SURGE                        = 37445,
    SPELL_MAGE_MASTER_OF_TIME                    = 342249,
    SPELL_MAGE_MIRROR_IMAGE_LEFT                 = 58834,
    SPELL_MAGE_MIRROR_IMAGE_RIGHT                = 58833,
    SPELL_MAGE_MIRROR_IMAGE_FRONT                = 58831,
    SPELL_MAGE_RADIANT_SPARK_PROC_BLOCKER        = 376105,
    SPELL_MAGE_RAY_OF_FROST                      = 205021,
    SPELL_MAGE_RAY_OF_FROST_BONUS                = 208141,
    SPELL_MAGE_RAY_OF_FROST_BUFF                 = 208166,
    SPELL_MAGE_REVERBERATE                       = 281482,
    SPELL_MAGE_RING_OF_FROST_DUMMY               = 91264,
    SPELL_MAGE_RING_OF_FROST_FREEZE              = 82691,
    SPELL_MAGE_RING_OF_FROST_SUMMON              = 113724,
    SPELL_MAGE_SERPENT_FORM                      = 32817,
    SPELL_MAGE_SHEEP_FORM                        = 32820,
    SPELL_MAGE_SHIMMER                           = 212653,
    SPELL_MAGE_SLOW                              = 31589,
    SPELL_MAGE_SQUIRREL_FORM                     = 32813,
    SPELL_MAGE_SUPERNOVA                         = 157980,
    SPELL_MAGE_TEMPEST_BARRIER_ABSORB            = 382290,
    SPELL_MAGE_UNSTABLE_MAGIC                    = 157976,
    SPELL_MAGE_UNSTABLE_MAGIC_DAMAGE_FIRE        = 157977,
    SPELL_MAGE_UNSTABLE_MAGIC_DAMAGE_FROST       = 157978,
    SPELL_MAGE_UNSTABLE_MAGIC_DAMAGE_ARCANE      = 157979,
    SPELL_MAGE_WORGEN_FORM                       = 32819,
    SPELL_PET_NETHERWINDS_FATIGUED               = 160455,
    SPELL_MAGE_ICE_LANCE_TRIGGER                 = 228598,
    SPELL_MAGE_THERMAL_VOID                      = 155149,
    SPELL_MAGE_ICY_VEINS                         = 12472,
    SPELL_MAGE_CHAIN_REACTION                    = 195419,
    SPELL_MAGE_FROSTBOLT                         = 116,
    SPELL_MAGE_FROSTBOLT_TRIGGER                 = 228597,
    SPELL_MAGE_FROZEN_ORB_DAMAGE                 = 84721,
    SPELL_MAGE_METEOR_DAMAGE                     = 153564,
    SPELL_MAGE_METEOR_TIMER                      = 177345,
    SPELL_MAGE_METEOR_VISUAL                     = 174556,
    SPELL_MAGE_METEOR_BURN                       = 155158,
    SPELL_MAGE_TOUCH_OF_THE_MAGI_EXPLODE         = 210833,
    SPELL_MAGE_WINTERS_CHILL                     = 228358
};

// 110909 - Alter Time Aura
// 342246 - Alter Time Aura
class spell_mage_alter_time_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_MAGE_ALTER_TIME_VISUAL,
            SPELL_MAGE_MASTER_OF_TIME,
            SPELL_MAGE_BLINK,
        });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* unit = GetTarget();
        _health = unit->GetHealth();
        _pos = unit->GetPosition();
    }

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* unit = GetTarget();
        if (unit->GetDistance(_pos) <= 100.0f && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            unit->SetHealth(_health);
            unit->NearTeleportTo(_pos);

            if (unit->HasAura(SPELL_MAGE_MASTER_OF_TIME))
            {
                SpellInfo const* blink = sSpellMgr->AssertSpellInfo(SPELL_MAGE_BLINK, DIFFICULTY_NONE);
                unit->GetSpellHistory()->ResetCharges(blink->ChargeCategoryId);
            }
            unit->CastSpell(unit, SPELL_MAGE_ALTER_TIME_VISUAL);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_mage_alter_time_aura::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_mage_alter_time_aura::AfterRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
    }

private:
    uint64 _health = 0;
    Position _pos;
};

// 127140 - Alter Time Active
// 342247 - Alter Time Active
class spell_mage_alter_time_active : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_MAGE_ALTER_TIME_AURA,
            SPELL_MAGE_ARCANE_ALTER_TIME_AURA,
        });
    }

    void RemoveAlterTimeAura(SpellEffIndex /*effIndex*/)
    {
        Unit* unit = GetCaster();
        unit->RemoveAura(SPELL_MAGE_ALTER_TIME_AURA, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        unit->RemoveAura(SPELL_MAGE_ARCANE_ALTER_TIME_AURA, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_mage_alter_time_active::RemoveAlterTimeAura, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 30451 - Arcane Blast
// Consumes a Presence of Mind charge on cast (2 charges granted, makes Arcane Blast instant and
// free of its Arcane Charge mana-cost scaling until both are consumed). The base Arcane
// Charge-stacking mechanic itself needs no script here - it's handled by the spell's own
// generic self-buff-stacking effect data.
class spell_mage_arcane_blast : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_PRESENCE_OF_MIND });
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Aura* presenceOfMind = caster->GetAura(SPELL_MAGE_PRESENCE_OF_MIND))
            presenceOfMind->ModCharges(-1);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_arcane_blast::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 235450 - Arcane Barrier
class spell_mage_arcane_barrier : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated) const
    {
        canBeRecalculated = false;
        if (Unit* caster = GetCaster())
            amount += int32(7.0f * caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask()));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_arcane_barrier::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 44425 - Arcane Barrage
class spell_mage_arcane_barrage : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_ARCANE_BARRAGE_R3, SPELL_MAGE_ARCANE_BARRAGE_ENERGIZE })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void ConsumeArcaneCharges()
    {
        Unit* caster = GetCaster();

        // Consume all arcane charges
        if (int32 arcaneCharges = -caster->ModifyPower(POWER_ARCANE_CHARGES, -caster->GetMaxPower(POWER_ARCANE_CHARGES), false))
            if (AuraEffect const* auraEffect = caster->GetAuraEffect(SPELL_MAGE_ARCANE_BARRAGE_R3, EFFECT_0, caster->GetGUID()))
                caster->CastSpell(caster, SPELL_MAGE_ARCANE_BARRAGE_ENERGIZE, { SPELLVALUE_BASE_POINT0, arcaneCharges * auraEffect->GetAmount() / 100 });
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/)
    {
        if (GetHitUnit()->GetGUID() != _primaryTarget)
            SetHitDamage(CalculatePct(GetHitDamage(), GetEffectInfo(EFFECT_1).CalcValue(GetCaster())));
    }

    void MarkPrimaryTarget(SpellEffIndex /*effIndex*/)
    {
        _primaryTarget = GetHitUnit()->GetGUID();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_arcane_barrage::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectLaunchTarget += SpellEffectFn(spell_mage_arcane_barrage::MarkPrimaryTarget, EFFECT_1, SPELL_EFFECT_DUMMY);
        AfterCast += SpellCastFn(spell_mage_arcane_barrage::ConsumeArcaneCharges);
    }

    ObjectGuid _primaryTarget;
};

// 235711 - Chrono Shift
class spell_mage_chrono_shift : public AuraScript
{
    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && (spellInfo->Id == SPELL_MAGE_ARCANE_BARRAGE || spellInfo->Id == SPELL_MAGE_ARCANE_BARRAGE_TRIGGERED);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_chrono_shift::CheckProc);
    }
};

// 195302 - Arcane Charge
class spell_mage_arcane_charge_clear : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_ARCANE_CHARGE });
    }

    void RemoveArcaneCharge(SpellEffIndex /*effIndex*/)
    {
        GetHitUnit()->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_CHARGE);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_arcane_charge_clear::RemoveArcaneCharge, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 1449 - Arcane Explosion
class spell_mage_arcane_explosion : public SpellScript
{
    // real Arcane Explosion (1449) only has 2 effects (EFFECT_0 SCHOOL_DAMAGE, EFFECT_1 ENERGIZE)
    // - Validate() was checking the wrong index for SCHOOL_DAMAGE, and Reverberate
    // (SPELL_MAGE_REVERBERATE, 281482) is confirmed absent from this build's Spell.db2 under any
    // id or name - a Dragonflight Arcane Mage talent, not Legion content. Removed the whole
    // HandleReverberate proc-chance branch; the real baseline "3+ targets + Arcane Mage talent"
    // energize gate (CheckRequiredAuraForBaselineEnergize) is genuine and kept, rebound to EFFECT_1.
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_ARCANE_MAGE })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_0 } })
            && spellInfo->GetEffect(EFFECT_0).IsEffect(SPELL_EFFECT_SCHOOL_DAMAGE);
    }

    void CheckRequiredAuraForBaselineEnergize(SpellEffIndex effIndex)
    {
        if (!GetUnitTargetCountForEffect(EFFECT_1) || !GetCaster()->HasAura(SPELL_MAGE_ARCANE_MAGE))
            PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_arcane_explosion::CheckRequiredAuraForBaselineEnergize, EFFECT_1, SPELL_EFFECT_ENERGIZE);
    }
};

// 5143 - Arcane Missiles
// Each missile volley fired during the channel (the periodic trigger effect) grants the caster
// Arcane Power's damage-per-missile buff and consumes one stack of the Arcane Missiles Charges
// aura, swapping the stack-count visual down to match.
class spell_mage_arcane_missiles : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            SPELL_MAGE_ARCANE_MISSILES_POWER, SPELL_MAGE_ARCANE_MISSILES_CHARGES,
            SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE, SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO,
            SPELL_MAGE_ARCANE_MISSILES_VISUAL_THREE
        });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_MAGE_ARCANE_MISSILES_POWER, true);

        if (Aura* charges = caster->GetAura(SPELL_MAGE_ARCANE_MISSILES_CHARGES))
        {
            switch (charges->GetStackAmount())
            {
                case 1:
                    caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE);
                    break;
                case 2:
                    caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO);
                    caster->CastSpell(caster, SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE, true);
                    break;
                case 3:
                    caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_THREE);
                    caster->CastSpell(caster, SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO, true);
                    break;
            }
            charges->ModStackAmount(-1);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_mage_arcane_missiles::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 7268 - Arcane Missiles (damage)
class spell_mage_arcane_missiles_damage : public SpellScript
{
    void FilterSelf(WorldObject*& target) const
    {
        if (target == GetCaster())
            target = nullptr;
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_mage_arcane_missiles_damage::FilterSelf, EFFECT_0, TARGET_UNIT_CHANNEL_TARGET);
    }
};

// 79684 - Clearcasting (Arcane Missiles proc)
// Prevents Arcane Missiles' own cast/damage from re-triggering Clearcasting, avoiding an
// infinite free-cast feedback loop.
class spell_mage_arcane_missiles_proc : public AuraScript
{
    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return !spellInfo || (spellInfo->Id != SPELL_MAGE_ARCANE_MISSILES && spellInfo->Id != SPELL_MAGE_ARCANE_MISSILES_DAMAGE);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_arcane_missiles_proc::CheckProc);
    }
};

// 79683 - Arcane Missiles Charges
// The stacking resource pool (max 3) that Arcane Missiles' channel consumes one-by-one; each
// stack gained here swaps the cosmetic charge-count visual up to match.
class spell_mage_arcane_missiles_charges : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE, SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO,
            SPELL_MAGE_ARCANE_MISSILES_VISUAL_THREE
        });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Aura* charges = caster->GetAura(SPELL_MAGE_ARCANE_MISSILES_CHARGES))
        {
            switch (charges->GetStackAmount())
            {
                case 1:
                    caster->CastSpell(caster, SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE, true);
                    break;
                case 2:
                    caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE);
                    caster->CastSpell(caster, SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO, true);
                    break;
                case 3:
                    caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO);
                    caster->CastSpell(caster, SPELL_MAGE_ARCANE_MISSILES_VISUAL_THREE, true);
                    break;
            }
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (!caster->HasAura(SPELL_MAGE_ARCANE_MISSILES_CHARGES))
        {
            caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_ONE);
            caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_TWO);
            caster->RemoveAurasDueToSpell(SPELL_MAGE_ARCANE_MISSILES_VISUAL_THREE);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_mage_arcane_missiles_charges::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_mage_arcane_missiles_charges::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 235313 - Blazing Barrier
class spell_mage_blazing_barrier : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_BLAZING_BARRIER_TRIGGER });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = false;
        if (Unit* caster = GetCaster())
            amount = int32(caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask()) * 7.0f);
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetDamageInfo()->GetVictim();
        Unit* target = eventInfo.GetDamageInfo()->GetAttacker();

        if (caster && target)
            caster->CastSpell(target, SPELL_MAGE_BLAZING_BARRIER_TRIGGER, true);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_blazing_barrier::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectProc += AuraEffectProcFn(spell_mage_blazing_barrier::HandleProc, EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 235365 - Blazing Soul (Fire artifact trait)
// Melee/spell hits landed within range refund a portion of the damage dealt as bonus absorb onto
// Blazing Barrier's shield, capped at the same maximum the shield itself uses.
class spell_mage_blazing_soul : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_BLAZING_BARRIER, SPELL_MAGE_BLAZING_SOUL });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* victim = eventInfo.GetActionTarget();
        if (!caster || !victim || !eventInfo.GetDamageInfo())
            return;

        int32 dist = aurEff->GetBase()->GetEffect(EFFECT_1) ? aurEff->GetBase()->GetEffect(EFFECT_1)->GetAmount() : 0;
        if (caster->GetDistance(victim) > dist)
            return;

        if (AuraEffect* barrier = caster->GetAuraEffect(SPELL_MAGE_BLAZING_BARRIER, EFFECT_0))
        {
            int32 bonus = int32(eventInfo.GetDamageInfo()->GetDamage()) * aurEff->GetAmount() / 100;
            int32 maxAmount = int32(barrier->GetBaseAmount() + caster->SpellBaseHealingBonusDone(barrier->GetSpellInfo()->GetSchoolMask()) * 7.0f);
            barrier->ChangeAmount(std::min(barrier->GetAmount() + bonus, maxAmount));
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_blazing_soul::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 190356 - Blizzard
// 4658 - AreaTrigger Create Properties
struct areatrigger_mage_blizzard : AreaTriggerAI
{
    areatrigger_mage_blizzard(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger), _tickTimer(TICK_PERIOD) { }

    static constexpr Milliseconds TICK_PERIOD = Milliseconds(1000);

    void OnUpdate(uint32 diff) override
    {
        _tickTimer -= Milliseconds(diff);

        while (_tickTimer <= 0s)
        {
            if (Unit* caster = at->GetCaster())
                caster->CastSpell(at->GetPosition(), SPELL_MAGE_BLIZZARD_DAMAGE);

            _tickTimer += TICK_PERIOD;
        }
    }

private:
    Milliseconds _tickTimer;
};

// 190357 - Blizzard (Damage)
class spell_mage_blizzard_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_BLIZZARD_SLOW });
    }

    void HandleSlow(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_MAGE_BLIZZARD_SLOW, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_blizzard_damage::HandleSlow, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 198063 - Burning Determination
class spell_mage_burning_determination : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (SpellInfo const* spellInfo = eventInfo.GetSpellInfo())
            if (spellInfo->GetAllEffectsMechanicMask() & ((1 << MECHANIC_INTERRUPT) | (1 << MECHANIC_SILENCE)))
                return true;

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_burning_determination::CheckProc);
    }
};

// 190447 - Brain Freeze
// Frostbolt has a chance to grant Brain Freeze, making the caster's next Ice Lance guaranteed
// to crit; consumption (and the free/instant cast itself) is handled on the Ice Lance side.
// Modeled directly on spell_mage_fingers_of_frost below, which procs off the same Frostbolt
// SpellFamily classmask bit.
class spell_mage_brain_freeze : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_BRAIN_FREEZE_AURA });
    }

    bool CheckProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->IsAffected(SPELLFAMILY_MAGE, flag128(0, 0x2000000, 0, 0))
            && roll_chance_i(aurEff->GetAmount());
    }

    void Trigger(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_MAGE_BRAIN_FREEZE_AURA, aurEff);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_mage_brain_freeze::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectProc += AuraEffectProcFn(spell_mage_brain_freeze::Trigger, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 86949 - Cauterize
class spell_mage_cauterize : public SpellScript
{
    void SuppressSpeedBuff(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_mage_cauterize::SuppressSpeedBuff, EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

class spell_mage_cauterize_AuraScript : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } }) && ValidateSpellInfo
        ({
            SPELL_MAGE_CAUTERIZE_DOT,
            SPELL_MAGE_CAUTERIZED,
            spellInfo->GetEffect(EFFECT_2).TriggerSpell
        });
    }

    void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
    {
        AuraEffect const* effect1 = GetEffect(EFFECT_1);
        if (!effect1 ||
            !GetTargetApplication()->HasEffect(EFFECT_1) ||
            dmgInfo.GetDamage() < GetTarget()->GetHealth() ||
            dmgInfo.GetDamage() > GetTarget()->GetMaxHealth() * 2 ||
            GetTarget()->HasAura(SPELL_MAGE_CAUTERIZED))
        {
            PreventDefaultAction();
            return;
        }

        GetTarget()->SetHealth(GetTarget()->CountPctFromMaxHealth(effect1->GetAmount()));
        GetTarget()->CastSpell(GetTarget(), GetEffectInfo(EFFECT_2).TriggerSpell, TRIGGERED_FULL_MASK);
        GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_CAUTERIZE_DOT, TRIGGERED_FULL_MASK);
        GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_CAUTERIZED, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_cauterize_AuraScript::HandleAbsorb, EFFECT_0);
    }
};

// 195419 - Chain Reaction
// Legion's Chain Reaction is a self-stacking passive: Frostbolt casts build stacks of this
// aura, whose own (DB2-defined) effects apply the Ice Lance damage bonus per stack. The script's
// only job is restricting which spells are allowed to add a stack.
// NOTE: ArgusCore previously used spell 278310 for "Chain Reaction", cast explicitly from
// spell_mage_ice_lance on a frozen-target hit gated behind a separate "dummy" marker aura
// (278309) - that ID and structure match modern retail's redesigned version of this talent, not
// Legion 7.3.5's. Corrected to the Legion-era spell id (195419, confirmed against an
// independent Legion 7.3.5 reference) and the self-sufficient aura structure it actually uses
// there; the explicit
// cast-from-Ice-Lance code and the now-unused dummy marker were removed accordingly.
class spell_mage_chain_reaction : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() &&
            (eventInfo.GetSpellInfo()->Id == SPELL_MAGE_FROSTBOLT || eventInfo.GetSpellInfo()->Id == SPELL_MAGE_FROSTBOLT_TRIGGER);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_chain_reaction::CheckProc);
    }
};

// 195345 - Frozen Veins (Frost artifact trait): Frostbolt casts reduce Icy Veins' cooldown.
// Confirmed via two independent reference sources. `SpellHistory::ModifyCooldown` needs a
// `Milliseconds` argument (documented gotcha in this project's own CLAUDE.md) - the
// reference's raw `aurEff->GetAmount()` int would fail to compile without the wrap.
class spell_mage_frozen_veins : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_ICY_VEINS });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() &&
            (eventInfo.GetSpellInfo()->Id == SPELL_MAGE_FROSTBOLT || eventInfo.GetSpellInfo()->Id == SPELL_MAGE_FROSTBOLT_TRIGGER);
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->GetSpellHistory()->HasCooldown(SPELL_MAGE_ICY_VEINS))
            caster->GetSpellHistory()->ModifyCooldown(SPELL_MAGE_ICY_VEINS, Milliseconds(aurEff->GetAmount()));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_frozen_veins::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_mage_frozen_veins::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 235219 - Cold Snap
class spell_mage_cold_snap : public SpellScript
{
    static uint32 constexpr SpellsToReset[] =
    {
        SPELL_MAGE_CONE_OF_COLD,
        SPELL_MAGE_ICE_BARRIER,
        SPELL_MAGE_ICE_BLOCK,
    };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(SpellsToReset) && ValidateSpellInfo({ SPELL_MAGE_FROST_NOVA });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        for (uint32 spellId : SpellsToReset)
            GetCaster()->GetSpellHistory()->ResetCooldown(spellId, true);

        GetCaster()->GetSpellHistory()->RestoreCharge(sSpellMgr->AssertSpellInfo(SPELL_MAGE_FROST_NOVA, GetCastDifficulty())->ChargeCategoryId);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_mage_cold_snap::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class CometStormEvent : public BasicEvent
{
public:
    CometStormEvent(Unit* caster, ObjectGuid originalCastId, Position const& dest) : _caster(caster), _originalCastId(originalCastId), _dest(dest), _count(0) { }

    bool Execute(uint64 time, uint32 /*diff*/) override
    {
        Position destPosition = {_dest.GetPositionX() + frand(-3.0f, 3.0f), _dest.GetPositionY() + frand(-3.0f, 3.0f), _dest.GetPositionZ()};
        _caster->CastSpell(destPosition, SPELL_MAGE_COMET_STORM_VISUAL,
            CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetOriginalCastId(_originalCastId));
        ++_count;

        if (_count >= 7)
            return true;

        _caster->m_Events.AddEvent(this, Milliseconds(time) + randtime(100ms, 275ms));
        return false;
    }

private:
    Unit* _caster;
    ObjectGuid _originalCastId;
    Position _dest;
    uint8 _count;
};

// 190319 - Combustion
// Doubles the caster's current spell critical strike rating for the aura's duration.
// NOTE: Legion's Combustion is also documented as refreshing its remaining duration by 1 sec
// per critical strike landed while active; that refresh behavior is intentionally NOT
// implemented here pending verification of the exact Legion 7.3.5 mechanic/values.
class spell_mage_combustion : public AuraScript
{
    void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = false;

        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        amount += int32(caster->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_SPELL));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_combustion::CalcAmount, EFFECT_1, SPELL_AURA_MOD_RATING);
    }
};

// 153595 - Comet Storm (launch)
class spell_mage_comet_storm : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_COMET_STORM_VISUAL });
    }

    void EffectHit(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->m_Events.AddEventAtOffset(new CometStormEvent(GetCaster(), GetSpell()->m_castId, *GetHitDest()), randtime(100ms, 275ms));
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_mage_comet_storm::EffectHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 228601 - Comet Storm (damage)
class spell_mage_comet_storm_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_COMET_STORM_DAMAGE });
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(*GetHitDest(), SPELL_MAGE_COMET_STORM_DAMAGE,
            CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetOriginalCastId(GetSpell()->m_originalCastId));
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_mage_comet_storm_damage::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 120 - Cone of Cold
class spell_mage_cone_of_cold : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_CONE_OF_COLD_SLOW });
    }

    void HandleSlow(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_MAGE_CONE_OF_COLD_SLOW, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_cone_of_cold::HandleSlow, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 190336 - Conjure Refreshment
class spell_mage_conjure_refreshment : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_MAGE_CONJURE_REFRESHMENT,
            SPELL_MAGE_CONJURE_REFRESHMENT_TABLE
        });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* caster = GetCaster()->ToPlayer())
        {
            Group* group = caster->GetGroup();
            if (group)
                caster->CastSpell(caster, SPELL_MAGE_CONJURE_REFRESHMENT_TABLE, true);
            else
                caster->CastSpell(caster, SPELL_MAGE_CONJURE_REFRESHMENT, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_conjure_refreshment::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 195676 - Displacement
// A one-button toggle: if a Displacement Beacon is already on the ground, teleport to it and
// remove it (short-circuiting the cast) instead of the normal cast flow placing a new one - the
// beacon's own placement is handled entirely by this spell's own DB2 effect data, no script
// needed for that half.
class spell_mage_displacement : public SpellScript
{
    void HandleBeforeCast() const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (AreaTrigger* beacon = caster->GetAreaTrigger(SPELL_MAGE_DISPLACEMENT_BEACON))
        {
            caster->NearTeleportTo(beacon->GetPosition());
            caster->RemoveAura(SPELL_MAGE_DISPLACEMENT_BEACON);
        }
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_mage_displacement::HandleBeforeCast);
    }
};

// 383395 - Feel the Burn
// 112965 - Fingers of Frost
class spell_mage_fingers_of_frost : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_FINGERS_OF_FROST });
    }

    bool CheckFrostboltProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->IsAffected(SPELLFAMILY_MAGE, flag128(0, 0x2000000, 0, 0))
            && roll_chance_i(aurEff->GetAmount());
    }

    bool CheckFrozenOrbProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->IsAffected(SPELLFAMILY_MAGE, flag128(0, 0, 0x80, 0))
            && roll_chance_i(aurEff->GetAmount());
    }

    void Trigger(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(GetTarget(), SPELL_MAGE_FINGERS_OF_FROST, aurEff);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_mage_fingers_of_frost::CheckFrostboltProc, EFFECT_0, SPELL_AURA_DUMMY);
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_mage_fingers_of_frost::CheckFrozenOrbProc, EFFECT_1, SPELL_AURA_DUMMY);
        AfterEffectProc += AuraEffectProcFn(spell_mage_fingers_of_frost::Trigger, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectProc += AuraEffectProcFn(spell_mage_fingers_of_frost::Trigger, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 133 - Fireball
// 11366 - Pyroblast
class spell_mage_firestarter : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_FIRESTARTER });
    }

    void CalcCritChance(Unit const* victim, float& critChance)
    {
        if (AuraEffect const* aurEff = GetCaster()->GetAuraEffect(SPELL_MAGE_FIRESTARTER, EFFECT_0))
            if (victim->GetHealthPct() >= aurEff->GetAmount())
                critChance = 100.0f;
    }

    void Register() override
    {
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_mage_firestarter::CalcCritChance);
    }
};

// 321712 - Pyroblast
class spell_mage_firestarter_dots : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_FIRESTARTER });
    }

    void CalcCritChance(AuraEffect const* /*aurEff*/, Unit const* victim, float& critChance)
    {
        if (AuraEffect const* aurEff = GetCaster()->GetAuraEffect(SPELL_MAGE_FIRESTARTER, EFFECT_0))
            if (victim->GetHealthPct() >= aurEff->GetAmount())
                critChance = 100.0f;
    }

    void Register() override
    {
        DoEffectCalcCritChance += AuraEffectCalcCritChanceFn(spell_mage_firestarter_dots::CalcCritChance, EFFECT_ALL, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 108853 - Fire Blast
class spell_mage_fire_blast : public SpellScript
{
    void CalcCritChance(Unit const* /*victim*/, float& critChance)
    {
        critChance = 100.0f;
    }

    void Register() override
    {
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_mage_fire_blast::CalcCritChance);
    }
};

// 205023 - Conflagration
class spell_mage_conflagration : public AuraScript
{
    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id == SPELL_MAGE_FIREBALL;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_conflagration::CheckProc);
    }
};

// 157642 - Enhanced Pyrotechnics
// Fireball crits remove the Enhanced Pyrotechnics buff instead of consuming a normal proc chance
// roll, preventing the buff from being wasted on a hit that didn't need its crit-chance bonus.
class spell_mage_enhanced_pyrotechnics : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_ENHANCED_PYROTECHNICS_AURA });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (!caster || !eventInfo.GetSpellInfo() || eventInfo.GetSpellInfo()->Id != SPELL_MAGE_FIREBALL)
            return false;

        if (eventInfo.GetHitMask() & PROC_HIT_CRITICAL)
        {
            if (caster->HasAura(SPELL_MAGE_ENHANCED_PYROTECHNICS_AURA))
                caster->RemoveAurasDueToSpell(SPELL_MAGE_ENHANCED_PYROTECHNICS_AURA);
            return false;
        }

        return true;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_enhanced_pyrotechnics::CheckProc);
    }
};

// 205029 - Flame On
class spell_mage_flame_on : public AuraScript
{
   bool Validate(SpellInfo const* spellInfo) override
   {
       return ValidateSpellInfo({ SPELL_MAGE_FIRE_BLAST })
           && sSpellCategoryStore.HasRecord(sSpellMgr->AssertSpellInfo(SPELL_MAGE_FIRE_BLAST, DIFFICULTY_NONE)->ChargeCategoryId)
           && ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } });
   }

   void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
   {
       canBeRecalculated = false;
       amount = -GetPctOf(GetEffectInfo(EFFECT_2).CalcValue() * IN_MILLISECONDS, sSpellCategoryStore.AssertEntry(sSpellMgr->AssertSpellInfo(SPELL_MAGE_FIRE_BLAST, DIFFICULTY_NONE)->ChargeCategoryId)->ChargeRecoveryTime);
   }

   void Register() override
   {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_flame_on::CalculateAmount, EFFECT_1, SPELL_AURA_CHARGE_RECOVERY_MULTIPLIER);
   }
};

// 205037 - Flame Patch (attached to 2120 - Flamestrike)
class spell_mage_flame_patch : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_FLAME_PATCH_TALENT, SPELL_MAGE_FLAME_PATCH_AREATRIGGER });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_MAGE_FLAME_PATCH_TALENT);
    }

    void HandleFlamePatch() const
    {
        GetCaster()->CastSpell(GetExplTargetDest()->GetPosition(), SPELL_MAGE_FLAME_PATCH_AREATRIGGER, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_mage_flame_patch::HandleFlamePatch);
    }
};

// 205470 - Flame Patch
// Id - 6122
struct at_mage_flame_patch : public AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnCreate(Spell const* /*creatingSpell*/) override
    {
        _scheduler.Schedule(1s, [this](TaskContext task)
        {
            if (Unit* caster = at->GetCaster())
                caster->CastSpell(at->GetPosition(), SPELL_MAGE_FLAME_PATCH_DAMAGE, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
            task.Repeat(1s);
        });
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 44614 - Flurry
class spell_mage_flurry : public SpellScript
{
    class FlurryEvent : public BasicEvent
    {
    public:
        FlurryEvent(Unit* caster, ObjectGuid const& target, ObjectGuid const& originalCastId, int32 count)
            : _caster(caster), _target(target), _originalCastId(originalCastId), _count(count) { }

        bool Execute(uint64 time, uint32 /*diff*/) override
        {
            Unit* target = ObjectAccessor::GetUnit(*_caster, _target);

            if (!target)
                return true;

            _caster->CastSpell(target, SPELL_MAGE_FLURRY_DAMAGE, CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetOriginalCastId(_originalCastId));

            if (!--_count)
                return true;

            _caster->m_Events.AddEvent(this, Milliseconds(time) + randtime(300ms, 400ms));
            return false;
        }

    private:
        Unit* _caster;
        ObjectGuid _target;
        ObjectGuid _originalCastId;
        int32 _count;
    };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_FLURRY_DAMAGE });
    }

    void EffectHit(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->m_Events.AddEventAtOffset(new FlurryEvent(GetCaster(), GetHitUnit()->GetGUID(), GetSpell()->m_castId, GetEffectValue() - 1), randtime(300ms, 400ms));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_flurry::EffectHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 228354 - Flurry (damage)
class spell_mage_flurry_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_WINTERS_CHILL });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_MAGE_WINTERS_CHILL, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_flurry_damage::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 116 - Frostbolt
class spell_mage_frostbolt : public SpellScript
{
    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_CHILLED });
    }

    void HandleChilled()
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, SPELL_MAGE_CHILLED, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_mage_frostbolt::HandleChilled);
    }
};

// 84721 - Frozen Orb (damage)
// Fingers of Frost's own proc gate already independently detects Frozen Orb hits via
// SPELLFAMILY flags (see spell_mage_fingers_of_frost::CheckFrozenOrbProc), so this only needs to
// apply the Chilled slow - no explicit Fingers of Frost cast needed here.
class spell_mage_frozen_orb : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_CHILLED });
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        if (Unit* caster = GetCaster())
            if (Unit* target = GetHitUnit())
                caster->CastSpell(target, SPELL_MAGE_CHILLED, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_frozen_orb::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 153561 - Meteor
class spell_mage_meteor : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_METEOR_TIMER });
    }

    void HandleAfterCast() const
    {
        Unit* caster = GetCaster();
        WorldLocation const* dest = GetExplTargetDest();
        if (!caster || !dest)
            return;

        caster->CastSpell(*dest, SPELL_MAGE_METEOR_TIMER, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_mage_meteor::HandleAfterCast);
    }
};

// 153564 - Meteor (damage)
class spell_mage_meteor_damage : public SpellScript
{
    void CountTargets(std::list<WorldObject*> const& targets)
    {
        _targets = int32(targets.size());
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        if (_targets > 0)
            SetHitDamage(GetHitDamage() / _targets);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_meteor_damage::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_mage_meteor_damage::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }

private:
    int32 _targets = 1;
};

// 177345 - Meteor (timer)
// AreaTriggerId - 3467
// Drives Meteor's delayed impact: shows the falling-meteor visual immediately on create, then
// casts the actual damage once the timer's own duration expires.
struct at_mage_meteor_timer : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnCreate(Spell const* /*creatingSpell*/) override
    {
        if (Unit* caster = at->GetCaster())
            caster->CastSpell(at->GetPosition(), SPELL_MAGE_METEOR_VISUAL, true);
    }

    void OnRemove() override
    {
        if (Unit* caster = at->GetCaster())
            caster->CastSpell(at->GetPosition(), SPELL_MAGE_METEOR_DAMAGE, true);
    }
};

// 155158 - Meteor (burn)
// AreaTriggerId - 1712
// The lingering burning crater Meteor leaves behind on impact.
struct at_mage_meteor_burn : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster || !unit)
            return;

        if (caster->IsValidAttackTarget(unit))
            caster->CastSpell(unit, SPELL_MAGE_METEOR_BURN, true);
    }

    void OnUnitExit(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster || !unit)
            return;

        if (Aura* burn = unit->GetAura(SPELL_MAGE_METEOR_BURN, caster->GetGUID()))
            burn->SetDuration(0);
    }
};

// 386737 - Hyper Impact
class spell_mage_hyper_impact : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_SUPERNOVA });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActionTarget(), SPELL_MAGE_SUPERNOVA, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_hyper_impact::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 45438 - Ice Block: applies Hypothermia on activation (long-standing anti-spam debuff,
// relevant since Cold Snap above can reset Ice Block's cooldown), and converts into Ice
// Barrier on removal if the Glacial Insulation talent is active. Confirmed via two independent
// reference sources (identical implementations). Neither reference implements any explicit
// cast-blocking check for Hypothermia itself - relies on the debuff's own DB2 data, matching
// this project's established pattern of data-driven mechanics needing no script at all.
class spell_mage_ice_block : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_HYPOTHERMIA, SPELL_MAGE_GLACIAL_INSULATION, SPELL_MAGE_ICE_BARRIER });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_HYPOTHERMIA, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget()->HasAura(SPELL_MAGE_GLACIAL_INSULATION))
            GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_ICE_BARRIER, true);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_mage_ice_block::OnApply, EFFECT_2, SPELL_AURA_SCHOOL_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_mage_ice_block::OnRemove, EFFECT_2, SPELL_AURA_SCHOOL_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 11426 - Ice Barrier
class spell_mage_ice_barrier : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_MAGE_CHILLED
        });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated) const
    {
        canBeRecalculated = false;
        amount = CalculatePct(GetUnitOwner()->GetMaxHealth(), GetEffectInfo(EFFECT_1).CalcValue());
        if (Player const* player = GetUnitOwner()->ToPlayer())
            AddPct(amount, player->GetRatingBonusValue(CR_VERSATILITY_DAMAGE_DONE) + player->GetTotalAuraModifier(SPELL_AURA_MOD_VERSATILITY));
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = eventInfo.GetDamageInfo()->GetVictim();
        Unit* target = eventInfo.GetDamageInfo()->GetAttacker();

        if (caster && target)
            caster->CastSpell(target, SPELL_MAGE_CHILLED, true);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_ice_barrier::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectProc += AuraEffectProcFn(spell_mage_ice_barrier::HandleProc, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 157997 - Ice Nova
// Doubles damage against the unit that was the explicit cast target. (Supernova/157980
// has its own dedicated spell_mage_supernova script below, reading its bonus from the
// spell's own effect data instead of a hardcoded multiplier - this class used to also be
// registered under "spell_mage_supernova", which crashed ScriptMgr::AddScript with a
// duplicate-registration abort against that other script; removed the duplicate rather
// than the more idiomatic one.)
class spell_mage_nova_talent : public SpellScript
{
    void HandleOnCast()
    {
        if (Unit* target = GetExplTargetUnit())
            _explicitTargetGUID = target->GetGUID();
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (target->GetGUID() == _explicitTargetGUID && !target->IsFriendlyTo(caster))
            SetHitDamage(GetHitDamage() * 2);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_mage_nova_talent::HandleOnCast);
        OnEffectHitTarget += SpellEffectFn(spell_mage_nova_talent::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }

    ObjectGuid _explicitTargetGUID;
};

// Ice Lance - 30455
class spell_mage_ice_lance : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_MAGE_ICE_LANCE_TRIGGER,
            SPELL_MAGE_THERMAL_VOID,
            SPELL_MAGE_ICY_VEINS,
            SPELL_MAGE_FINGERS_OF_FROST,
            SPELL_MAGE_BRAIN_FREEZE_AURA,
            SPELL_MAGE_FROST_BOMB_AURA,
            SPELL_MAGE_FROST_BOMB_TRIGGERED
        });
    }

    // Brain Freeze guarantees a crit; checked independently of the target-frozen state used by
    // Thermal Void/Chain Reaction below, since Brain Freeze is meant to work on unfrozen targets.
    void CalcCritChance(Unit const* /*victim*/, float& critChance)
    {
        if (GetCaster()->HasAura(SPELL_MAGE_BRAIN_FREEZE_AURA))
            critChance = 100.0f;
    }

    // Consumed once per cast (not per target) after the whole spell resolves, so every target
    // hit by this cast still sees Brain Freeze active for the crit-chance check above.
    void ConsumeBrainFreeze()
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_MAGE_BRAIN_FREEZE_AURA))
                caster->RemoveAurasDueToSpell(SPELL_MAGE_BRAIN_FREEZE_AURA);
    }

    void IndexTarget(SpellEffIndex /*effIndex*/)
    {
        _orderedTargets.push_back(GetHitUnit()->GetGUID());
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        std::ptrdiff_t index = std::distance(_orderedTargets.begin(), std::find(_orderedTargets.begin(), _orderedTargets.end(), target->GetGUID()));

        if (index == 0 // only primary target triggers these benefits
            && target->HasAuraState(AURA_STATE_FROZEN, GetSpellInfo(), caster))
        {
            // Thermal Void
            if (Aura const* thermalVoid = caster->GetAura(SPELL_MAGE_THERMAL_VOID))
                if (!thermalVoid->GetSpellInfo()->GetEffects().empty())
                    if (Aura* icyVeins = caster->GetAura(SPELL_MAGE_ICY_VEINS))
                        icyVeins->SetDuration(icyVeins->GetDuration() + thermalVoid->GetSpellInfo()->GetEffect(EFFECT_0).CalcValue(caster) * IN_MILLISECONDS);

            // Chain Reaction is handled entirely by its own self-sufficient generic proc/stack
            // data (see spell_mage_chain_reaction below) - Ice Lance no longer casts it directly.
        }

        // Frost Bomb: detonates on the primary target when either the target is frozen or the
        // caster has Fingers of Frost up - a broader condition than Thermal Void above (which
        // requires frozen specifically), so checked separately. Confirmed via two independent
        // reference sources (identical implementations).
        if (index == 0 && target->HasAura(SPELL_MAGE_FROST_BOMB_AURA)
            && (target->HasAuraState(AURA_STATE_FROZEN, GetSpellInfo(), caster) || caster->HasAura(SPELL_MAGE_FINGERS_OF_FROST)))
        {
            caster->CastSpell(target, SPELL_MAGE_FROST_BOMB_TRIGGERED, true);
        }

        // put target index for chain value multiplier into EFFECT_1 base points, otherwise triggered spell doesn't know which damage multiplier to apply
        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        args.AddSpellMod(SPELLVALUE_BASE_POINT1, index);
        caster->CastSpell(target, SPELL_MAGE_ICE_LANCE_TRIGGER, args);
    }

    void Register() override
    {
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_mage_ice_lance::CalcCritChance);
        OnEffectLaunchTarget += SpellEffectFn(spell_mage_ice_lance::IndexTarget, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        OnEffectHitTarget += SpellEffectFn(spell_mage_ice_lance::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        AfterCast += SpellCastFn(spell_mage_ice_lance::ConsumeBrainFreeze);
    }

    std::vector<ObjectGuid> _orderedTargets;
};

// 113092 - Frost Bomb (damage): explicit target takes more damage than the splash targets.
// The exact 1.84275/1.15128 spell-power ratios are ported verbatim from the reference -
// unlike most hardcoded-formula cases this project is cautious about, these match to 5
// decimal places across two independently-written reference cores, which is corroboration
// for genuine data rather than a shared guess.
class spell_mage_frost_bomb_damage : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Unit* explicitTarget = GetExplTargetUnit();
        if (!caster || !target || !explicitTarget)
            return;

        float multiplier = target == explicitTarget ? 1.84275f : 1.15128f;
        SetHitDamage(int32(caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL) * multiplier));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_frost_bomb_damage::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 195448 - Chilled to the Core
class spell_mage_chilled_to_the_core : public AuraScript
{
    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id == SPELL_MAGE_ICY_VEINS;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_chilled_to_the_core::CheckProc);
    }
};

// 228598 - Ice Lance
class spell_mage_ice_lance_damage : public SpellScript
{
    void ApplyDamageMultiplier(SpellEffIndex /*effIndex*/)
    {
        SpellValue const* spellValue = GetSpellValue();
        if (spellValue->CustomBasePointsMask & (1 << EFFECT_1))
        {
            int32 originalDamage = GetHitDamage();
            float targetIndex = float(spellValue->EffectBasePoints[EFFECT_1]);
            float multiplier = std::pow(GetEffectInfo().CalcDamageMultiplier(GetCaster(), GetSpell()), targetIndex);
            SetHitDamage(int32(originalDamage * multiplier));
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_ice_lance_damage::ApplyDamageMultiplier, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 214626 - Jouster (Frost artifact trait)
class spell_mage_jouster : public AuraScript
{
    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id == SPELL_MAGE_ICE_LANCE;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_jouster::CheckProc);
    }
};

// 195391 - Jouster (damage-taken reduction buff)
class spell_mage_jouster_buff : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_JOUSTER });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/) const
    {
        if (Unit* caster = GetCaster())
            if (AuraEffect const* jousterRank = caster->GetAuraEffect(SPELL_MAGE_JOUSTER, EFFECT_0))
                amount = jousterRank->GetAmount();
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_jouster_buff::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    }
};

// 155148 - Kindling
class spell_mage_kindling : public AuraScript
{
    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && (spellInfo->Id == SPELL_MAGE_FIREBALL || spellInfo->Id == SPELL_MAGE_FIRE_BLAST || spellInfo->Id == SPELL_MAGE_PYROBLAST);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& /*eventInfo*/) const
    {
        if (Unit* caster = GetCaster())
            caster->GetSpellHistory()->ModifyCooldown(SPELL_MAGE_COMBUSTION, Milliseconds(-1000));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_kindling::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_mage_kindling::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 198928 - Cinderstorm
class spell_mage_cinderstorm : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_IGNITE }) && ValidateSpellEffect({ { spellInfo->Id, EFFECT_0 } });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->HasAura(SPELL_MAGE_IGNITE))
            return;

        SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), GetEffectInfo(EFFECT_0).CalcValue(caster)));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_cinderstorm::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 12846 - Ignite
class spell_mage_ignite : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_IGNITE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        SpellInfo const* igniteDot = sSpellMgr->AssertSpellInfo(SPELL_MAGE_IGNITE, GetCastDifficulty());
        int32 pct = aurEff->GetAmount();

        ASSERT(igniteDot->GetMaxTicks() > 0);
        int32 amount = int32(CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), pct) / igniteDot->GetMaxTicks());

        CastSpellExtraArgs args(aurEff);
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, amount);
        GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_MAGE_IGNITE, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_ignite::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_mage_ignite::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 44448 - Pyroblast! (Hot Streak driver)
// Two consecutive crits from a qualifying Fire spell grant Hot Streak (instant, free Pyroblast/
// Flamestrike); an intervening non-crit resets progress back to no stacks.
class spell_mage_pyroblast_clearcasting_driver : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_MAGE_SCORCH,
            SPELL_MAGE_FIREBALL,
            SPELL_MAGE_FIRE_BLAST,
            SPELL_MAGE_FLAMESTRIKE,
            SPELL_MAGE_PYROBLAST,
            SPELL_MAGE_PHOENIX_FLAMES,
            SPELL_MAGE_DRAGONS_BREATH,
            SPELL_MAGE_HEATING_UP,
            SPELL_MAGE_HOT_STREAK
        });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        switch (spellInfo->Id)
        {
            case SPELL_MAGE_SCORCH:
            case SPELL_MAGE_FIREBALL:
            case SPELL_MAGE_FIRE_BLAST:
            case SPELL_MAGE_FLAMESTRIKE:
            case SPELL_MAGE_PYROBLAST:
            case SPELL_MAGE_PHOENIX_FLAMES:
                return true;
            case SPELL_MAGE_DRAGONS_BREATH:
                // Only procs from Dragon's Breath while talented into Alexstrasza's Fury
                return GetCaster()->HasAura(SPELL_MAGE_ALEXSTRASZAS_FURY);
            default:
                return false;
        }
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // A non-critical qualifying hit resets Heating Up progress.
        if (eventInfo.GetHitMask() & PROC_HIT_NORMAL)
        {
            caster->RemoveAurasDueToSpell(SPELL_MAGE_HEATING_UP);
            return;
        }

        if (!caster->HasAura(SPELL_MAGE_HEATING_UP) && !caster->HasAura(SPELL_MAGE_HOT_STREAK))
            caster->CastSpell(caster, SPELL_MAGE_HEATING_UP, true);
        else if (caster->HasAura(SPELL_MAGE_HEATING_UP) && !caster->HasAura(SPELL_MAGE_HOT_STREAK))
        {
            caster->RemoveAurasDueToSpell(SPELL_MAGE_HEATING_UP);
            caster->CastSpell(caster, SPELL_MAGE_HOT_STREAK, true);
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_pyroblast_clearcasting_driver::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_mage_pyroblast_clearcasting_driver::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 11366 - Pyroblast
// Consumes Hot Streak on cast; without this the aura would never be removed and Hot Streak
// would grant unlimited free instant Pyroblasts instead of just the one it earned.
class spell_mage_pyroblast : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_HOT_STREAK });
    }

    void ConsumeHotStreak(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_MAGE_HOT_STREAK))
                caster->RemoveAurasDueToSpell(SPELL_MAGE_HOT_STREAK);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_mage_pyroblast::ConsumeHotStreak, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 2120 - Flamestrike
// Same Hot Streak consumption as Pyroblast above.
class spell_mage_flamestrike : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_HOT_STREAK });
    }

    void ConsumeHotStreak(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_MAGE_HOT_STREAK))
                caster->RemoveAurasDueToSpell(SPELL_MAGE_HOT_STREAK);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_mage_flamestrike::ConsumeHotStreak, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 37447 - Improved Mana Gems
// 61062 - Improved Mana Gems
class spell_mage_imp_mana_gems : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_MANA_SURGE });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActor()->CastSpell(nullptr, SPELL_MAGE_MANA_SURGE, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_imp_mana_gems::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 205039 - Erosion
class spell_mage_erosion : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_EROSION_TIMER });
    }

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo) const
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target)
            return;

        if (Aura* erosionTimer = target->GetAura(SPELL_MAGE_EROSION_TIMER, caster->GetGUID()))
            erosionTimer->RefreshDuration(false);
        else
            caster->AddAura(SPELL_MAGE_EROSION_TIMER, target);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_erosion::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 210154 - Erosion (timer)
// Each application drives one stack of the Erosion Aurastate debuff (210134); when a timer
// instance expires, it removes one stack rather than the whole debuff.
class spell_mage_erosion_timer : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_EROSION_AURASTATE });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        if (Aura* aurastate = target->GetAura(SPELL_MAGE_EROSION_AURASTATE, caster->GetGUID()))
            aurastate->ModStackAmount(-1);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_mage_erosion_timer::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 236058 - Frenetic Speed
class spell_mage_frenetic_speed : public AuraScript
{
    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id == SPELL_MAGE_SCORCH;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_mage_frenetic_speed::CheckProc);
    }
};

// 108839 - Ice Floes
// Casting a non-instant spell while Ice Floes is up consumes a stack, letting that one cast
// happen while moving. Scorch is excluded since it's already instant-equivalent for this purpose.
class spell_mage_ice_floes : public AuraScript
{
    void HandleAfterProc(ProcEventInfo& eventInfo) const
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo || !spellInfo->CalcCastTime() || spellInfo->Id == SPELL_MAGE_SCORCH)
            return;

        if (Unit* caster = GetCaster())
            if (Aura* iceFloes = caster->GetAura(SPELL_MAGE_ICE_FLOES))
                iceFloes->ModStackAmount(-1);
    }

    void Register() override
    {
        AfterProc += AuraProcFn(spell_mage_ice_floes::HandleAfterProc);
    }
};

// 1463 - Incanter's Flow
class spell_mage_incanters_flow : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_INCANTERS_FLOW });
    }

    void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
    {
        // Incanter's flow should not cycle out of combat
        if (!GetTarget()->IsInCombat())
            return;

        if (Aura* aura = GetTarget()->GetAura(SPELL_MAGE_INCANTERS_FLOW))
        {
            uint32 stacks = aura->GetStackAmount();

            // Force always to values between 1 and 5
            if ((modifier == -1 && stacks == 1) || (modifier == 1 && stacks == 5))
            {
                modifier *= -1;
                return;
            }

            aura->ModStackAmount(modifier);
        }
        else
            GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_INCANTERS_FLOW, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_incanters_flow::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    int8 modifier = 1;
};

// 55342 - Mirror Image
// Summons the three cosmetic clone images positioned around the caster.
class spell_mage_mirror_image_summon : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_MIRROR_IMAGE_LEFT, SPELL_MAGE_MIRROR_IMAGE_FRONT, SPELL_MAGE_MIRROR_IMAGE_RIGHT });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        if (Unit* caster = GetCaster())
        {
            caster->CastSpell(caster, SPELL_MAGE_MIRROR_IMAGE_LEFT, true);
            caster->CastSpell(caster, SPELL_MAGE_MIRROR_IMAGE_FRONT, true);
            caster->CastSpell(caster, SPELL_MAGE_MIRROR_IMAGE_RIGHT, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_mirror_image_summon::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// 44457 - Living Bomb
class spell_mage_living_bomb : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_LIVING_BOMB_PERIODIC });
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        GetCaster()->CastSpell(GetHitUnit(), SPELL_MAGE_LIVING_BOMB_PERIODIC, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT2, 1));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_living_bomb::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 44461 - Living Bomb
class spell_mage_living_bomb_explosion : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return spellInfo->NeedsExplicitUnitTarget() && ValidateSpellInfo({ SPELL_MAGE_LIVING_BOMB_PERIODIC });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove(GetExplTargetWorldObject());
    }

    void HandleSpread(SpellEffIndex /*effIndex*/)
    {
        if (GetSpellValue()->EffectBasePoints[EFFECT_0] > 0)
            GetCaster()->CastSpell(GetHitUnit(), SPELL_MAGE_LIVING_BOMB_PERIODIC, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT2, 0));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_living_bomb_explosion::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_mage_living_bomb_explosion::HandleSpread, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 217694 - Living Bomb
class spell_mage_living_bomb_periodic : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_LIVING_BOMB_EXPLOSION });
    }

    void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget(), SPELL_MAGE_LIVING_BOMB_EXPLOSION, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, aurEff->GetAmount()));
    }

    void Register() override
    {
        // real Living Bomb Periodic (217694) only has 2 effects - the DUMMY is at EFFECT_1, not EFFECT_2
        AfterEffectRemove += AuraEffectRemoveFn(spell_mage_living_bomb_periodic::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

enum SilvermoonPolymorph
{
    NPC_AUROSALIA       = 18744
};

/// @todo move out of here and rename - not a mage spell
// 32826 - Polymorph (Visual)
class spell_mage_polymorph_visual : public SpellScript
{
    static const uint32 PolymorhForms[6];

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(PolymorhForms);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetCaster()->FindNearestCreature(NPC_AUROSALIA, 30.0f))
            if (target->GetTypeId() == TYPEID_UNIT)
                target->CastSpell(target, PolymorhForms[urand(0, 5)], true);
    }

    void Register() override
    {
        // add dummy effect spell handler to Polymorph visual
        OnEffectHitTarget += SpellEffectFn(spell_mage_polymorph_visual::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

uint32 const spell_mage_polymorph_visual::PolymorhForms[6] =
{
    SPELL_MAGE_SQUIRREL_FORM,
    SPELL_MAGE_GIRAFFE_FORM,
    SPELL_MAGE_SERPENT_FORM,
    SPELL_MAGE_DRAGONHAWK_FORM,
    SPELL_MAGE_WORGEN_FORM,
    SPELL_MAGE_SHEEP_FORM
};

// 235450 - Prismatic Barrier
class spell_mage_prismatic_barrier : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = false;
        if (Unit* caster = GetCaster())
            amount += int32(caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask()) * 7.0f);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_prismatic_barrier::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 376103 - Radiant Spark
class spell_mage_radiant_spark : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_RADIANT_SPARK_PROC_BLOCKER });
    }

    bool CheckProc(AuraEffect const* /*aurEff*/, ProcEventInfo& procInfo)
    {
        return !procInfo.GetProcTarget()->HasAura(SPELL_MAGE_RADIANT_SPARK_PROC_BLOCKER, GetCasterGUID());
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& procInfo)
    {
        Aura* vulnerability = procInfo.GetProcTarget()->GetAura(aurEff->GetSpellEffectInfo().TriggerSpell, GetCasterGUID());
        if (vulnerability && vulnerability->GetStackAmount() == vulnerability->CalcMaxStackAmount())
        {
            PreventDefaultAction();
            vulnerability->Remove();
            GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_RADIANT_SPARK_PROC_BLOCKER, true);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_radiant_spark::HandleProc, EFFECT_2, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 205021 - Ray of Frost
class spell_mage_ray_of_frost_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_MAGE_RAY_OF_FROST_BONUS, SPELL_MAGE_RAY_OF_FROST_BUFF });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            if (!caster->HasAura(SPELL_MAGE_RAY_OF_FROST_BUFF))
                caster->CastSpell(caster, SPELL_MAGE_RAY_OF_FROST_BUFF, true);
    }

    void HandleEffectPeriodic(AuraEffect const* aurEff)
    {
        if (Unit* caster = GetCaster())
        {
            if (aurEff->GetTickNumber() > 1) // First tick should deal base damage
               caster->CastSpell(caster, SPELL_MAGE_RAY_OF_FROST_BONUS, true);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_mage_ray_of_frost_aura::HandleApply, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_ray_of_frost_aura::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 208166 - Ray of Frost (Buff)
// Suppresses Ray of Frost's own cooldown for the duration of the channel (so the channel itself
// isn't blocked by its post-cast cooldown), then restores the normal cooldown and force-ends the
// channel once the buff falls off - covers both a natural channel finish and an early interrupt.
class spell_mage_ray_of_frost_buff : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_RAY_OF_FROST });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->GetSpellHistory()->ResetCooldown(SPELL_MAGE_RAY_OF_FROST, true);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (SpellInfo const* rayOfFrostInfo = sSpellMgr->GetSpellInfo(SPELL_MAGE_RAY_OF_FROST, DIFFICULTY_NONE))
            GetTarget()->GetSpellHistory()->StartCooldown(rayOfFrostInfo, 0);

        if (GetTarget()->GetChannelSpellId() == SPELL_MAGE_RAY_OF_FROST)
            GetTarget()->FinishSpell(CURRENT_CHANNELED_SPELL);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_mage_ray_of_frost_buff::HandleApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_mage_ray_of_frost_buff::HandleRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

// 157976 - Unstable Magic (legendary ring proc)
// Shared script bound under three separate ScriptNames to Arcane Blast (30451), Fireball (133),
// and the Frostbolt damage trigger (228597) - reads the equipped ring's passive Unstable Magic
// aura for the caster's spec-specific proc chance/damage percent, then splashes bonus damage of
// the matching school onto the same target.
class spell_mage_unstable_magic : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        Aura* unstableMagic = player->GetAura(SPELL_MAGE_UNSTABLE_MAGIC);
        if (!unstableMagic)
            return;

        int32 chance = 0;
        switch (player->GetPrimarySpecialization())
        {
            case ChrSpecialization::MageArcane:
                chance = unstableMagic->GetEffect(EFFECT_0) ? unstableMagic->GetEffect(EFFECT_0)->GetAmount() : 0;
                break;
            case ChrSpecialization::MageFrost:
                chance = unstableMagic->GetEffect(EFFECT_1) ? unstableMagic->GetEffect(EFFECT_1)->GetAmount() : 0;
                break;
            case ChrSpecialization::MageFire:
                chance = unstableMagic->GetEffect(EFFECT_2) ? unstableMagic->GetEffect(EFFECT_2)->GetAmount() : 0;
                break;
            default:
                break;
        }

        if (!chance || !roll_chance_i(chance))
            return;

        Unit* target = GetHitUnit();
        if (!target)
            return;

        int32 pct = unstableMagic->GetEffect(EFFECT_3) ? unstableMagic->GetEffect(EFFECT_3)->GetAmount() : 0;
        int32 basePoints = CalculatePct(GetHitDamage(), pct);

        switch (GetSpellInfo()->Id)
        {
            case SPELL_MAGE_ARCANE_BLAST:
                player->CastSpell(target, SPELL_MAGE_UNSTABLE_MAGIC_DAMAGE_ARCANE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, basePoints));
                break;
            case SPELL_MAGE_FROSTBOLT_TRIGGER:
                player->CastSpell(target, SPELL_MAGE_UNSTABLE_MAGIC_DAMAGE_FROST, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, basePoints));
                break;
            case SPELL_MAGE_FIREBALL:
                player->CastSpell(target, SPELL_MAGE_UNSTABLE_MAGIC_DAMAGE_FIRE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, basePoints));
                break;
            default:
                break;
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_unstable_magic::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 136511 - Ring of Frost
class spell_mage_ring_of_frost : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_RING_OF_FROST_SUMMON, SPELL_MAGE_RING_OF_FROST_FREEZE })
            && ValidateSpellEffect({ { SPELL_MAGE_RING_OF_FROST_SUMMON, EFFECT_0 } });
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        if (TempSummon* ringOfFrost = GetRingOfFrostMinion())
            GetTarget()->CastSpell(ringOfFrost->GetPosition(), SPELL_MAGE_RING_OF_FROST_FREEZE, true);
    }

    void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        std::list<TempSummon*> minions;
        GetTarget()->GetAllMinionsByEntry(minions, sSpellMgr->AssertSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON, GetCastDifficulty())->GetEffect(EFFECT_0).MiscValue);

        // Get the last summoned RoF, save it and despawn older ones
        for (TempSummon* summon : minions)
        {
            if (TempSummon* ringOfFrost = GetRingOfFrostMinion())
            {
                if (summon->GetTimer() > ringOfFrost->GetTimer())
                {
                    ringOfFrost->DespawnOrUnsummon();
                    _ringOfFrostGUID = summon->GetGUID();
                }
                else
                    summon->DespawnOrUnsummon();
            }
            else
                _ringOfFrostGUID = summon->GetGUID();
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_ring_of_frost::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        OnEffectApply += AuraEffectApplyFn(spell_mage_ring_of_frost::Apply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }

private:
    TempSummon* GetRingOfFrostMinion() const
    {
        if (Creature* creature = ObjectAccessor::GetCreature(*GetOwner(), _ringOfFrostGUID))
            return creature->ToTempSummon();
        return nullptr;
    }

    ObjectGuid _ringOfFrostGUID;
};

// 82691 - Ring of Frost (freeze efect)
class spell_mage_ring_of_frost_freeze : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_RING_OF_FROST_SUMMON, SPELL_MAGE_RING_OF_FROST_FREEZE })
            && ValidateSpellEffect({ { SPELL_MAGE_RING_OF_FROST_SUMMON, EFFECT_0 } });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        WorldLocation const* dest = GetExplTargetDest();
        float outRadius = sSpellMgr->AssertSpellInfo(SPELL_MAGE_RING_OF_FROST_SUMMON, GetCastDifficulty())->GetEffect(EFFECT_0).CalcRadius(nullptr, SpellTargetIndex::TargetB);
        float inRadius = 6.5f;

        targets.remove_if([dest, outRadius, inRadius](WorldObject* target)
        {
            Unit* unit = target->ToUnit();
            if (!unit)
                return true;
            return unit->HasAura(SPELL_MAGE_RING_OF_FROST_DUMMY) || unit->HasAura(SPELL_MAGE_RING_OF_FROST_FREEZE) || unit->GetExactDist(dest) > outRadius || unit->GetExactDist(dest) < inRadius;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_ring_of_frost_freeze::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

class spell_mage_ring_of_frost_freeze_AuraScript : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_RING_OF_FROST_DUMMY });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            if (GetCaster())
                GetCaster()->CastSpell(GetTarget(), SPELL_MAGE_RING_OF_FROST_DUMMY, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_mage_ring_of_frost_freeze_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

// 157980 - Supernova
class spell_mage_supernova : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        if (GetExplTargetUnit() == GetHitUnit())
        {
            uint32 damage = GetHitDamage();
            AddPct(damage, GetEffectInfo(EFFECT_0).CalcValue());
            SetHitDamage(damage);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_supernova::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 382289 - Tempest Barrier
class spell_mage_tempest_barrier : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_TEMPEST_BARRIER_ABSORB });
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/)
    {
        PreventDefaultAction();
        Unit* target = GetTarget();
        int32 amount = CalculatePct(target->GetMaxHealth(), aurEff->GetAmount());
        target->CastSpell(target, SPELL_MAGE_TEMPEST_BARRIER_ABSORB, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, amount } }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_tempest_barrier::HandleEffectProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 210824 - Touch of the Magi (Aura)
class spell_mage_touch_of_the_magi_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_TOUCH_OF_THE_MAGI_EXPLODE });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (damageInfo)
        {
            if (damageInfo->GetAttacker() == GetCaster() && damageInfo->GetVictim() == GetTarget())
            {
                uint32 extra = CalculatePct(damageInfo->GetDamage(), 25);
                if (extra > 0)
                    aurEff->ChangeAmount(aurEff->GetAmount() + extra);
            }
        }
    }

    void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        int32 amount = aurEff->GetAmount();
        if (!amount || GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget(), SPELL_MAGE_TOUCH_OF_THE_MAGI_EXPLODE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, amount));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_touch_of_the_magi_aura::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_mage_touch_of_the_magi_aura::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 33395 Water Elemental's Freeze
class spell_mage_water_elemental_freeze : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MAGE_FINGERS_OF_FROST, SPELL_MAGE_FINGERS_OF_FROST_VISUAL_UI });
    }

    void HandleImprovedFreeze()
    {
        Unit* owner = GetCaster()->GetOwner();
        if (!owner)
            return;

        if (owner->HasAura(SPELL_MAGE_FINGERS_OF_FROST))
            owner->CastSpell(owner, SPELL_MAGE_FINGERS_OF_FROST_VISUAL_UI, true);

        owner->CastSpell(owner, SPELL_MAGE_FINGERS_OF_FROST, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_mage_water_elemental_freeze::HandleImprovedFreeze);
    }
};

void AddSC_mage_spell_scripts()
{
    RegisterSpellScript(spell_mage_alter_time_aura);
    RegisterSpellScript(spell_mage_alter_time_active);
    RegisterSpellScript(spell_mage_arcane_barrage);
    RegisterSpellScript(spell_mage_chrono_shift);
    RegisterSpellScript(spell_mage_arcane_barrier);
    RegisterSpellScript(spell_mage_arcane_blast);
    RegisterSpellScript(spell_mage_arcane_missiles);
    RegisterSpellScript(spell_mage_arcane_missiles_damage);
    RegisterSpellScript(spell_mage_arcane_missiles_proc);
    RegisterSpellScript(spell_mage_arcane_missiles_charges);
    RegisterSpellScript(spell_mage_arcane_charge_clear);
    RegisterSpellScript(spell_mage_arcane_explosion);
    RegisterSpellScript(spell_mage_blazing_barrier);
    RegisterSpellScript(spell_mage_blazing_soul);
    RegisterAreaTriggerAI(areatrigger_mage_blizzard);
    RegisterSpellScript(spell_mage_blizzard_damage);
    RegisterSpellScript(spell_mage_brain_freeze);
    RegisterSpellScript(spell_mage_burning_determination);
    RegisterSpellAndAuraScriptPair(spell_mage_cauterize, spell_mage_cauterize_AuraScript);
    RegisterSpellScript(spell_mage_chain_reaction);
    RegisterSpellScript(spell_mage_cold_snap);
    RegisterSpellScript(spell_mage_frozen_veins);
    RegisterSpellScript(spell_mage_combustion);
    RegisterSpellScript(spell_mage_comet_storm);
    RegisterSpellScript(spell_mage_comet_storm_damage);
    RegisterSpellScript(spell_mage_cone_of_cold);
    RegisterSpellScript(spell_mage_conjure_refreshment);
    RegisterSpellScript(spell_mage_displacement);
    RegisterSpellScript(spell_mage_fingers_of_frost);
    RegisterSpellScript(spell_mage_firestarter);
    RegisterSpellScript(spell_mage_firestarter_dots);
    RegisterSpellScript(spell_mage_fire_blast);
    RegisterSpellScript(spell_mage_conflagration);
    RegisterSpellScript(spell_mage_enhanced_pyrotechnics);
    RegisterSpellScript(spell_mage_flame_on);
    RegisterSpellScript(spell_mage_flame_patch);
    RegisterAreaTriggerAI(at_mage_flame_patch);
    RegisterSpellScript(spell_mage_flamestrike);
    RegisterSpellScript(spell_mage_flurry);
    RegisterSpellScript(spell_mage_flurry_damage);
    RegisterSpellScript(spell_mage_frostbolt);
    RegisterSpellScript(spell_mage_frozen_orb);
    RegisterSpellScript(spell_mage_hyper_impact);
    RegisterSpellScript(spell_mage_meteor);
    RegisterSpellScript(spell_mage_meteor_damage);
    RegisterAreaTriggerAI(at_mage_meteor_timer);
    RegisterAreaTriggerAI(at_mage_meteor_burn);
    RegisterSpellScript(spell_mage_ice_barrier);
    RegisterSpellScript(spell_mage_ice_block);
    RegisterSpellScriptWithArgs(spell_mage_nova_talent, "spell_mage_ice_nova");
    RegisterSpellScript(spell_mage_chilled_to_the_core);
    RegisterSpellScript(spell_mage_ice_lance);
    RegisterSpellScript(spell_mage_frost_bomb_damage);
    RegisterSpellScript(spell_mage_ice_lance_damage);
    RegisterSpellScript(spell_mage_jouster);
    RegisterSpellScript(spell_mage_jouster_buff);
    RegisterSpellScript(spell_mage_kindling);
    RegisterSpellScript(spell_mage_cinderstorm);
    RegisterSpellScript(spell_mage_ignite);
    RegisterSpellScript(spell_mage_erosion);
    RegisterSpellScript(spell_mage_erosion_timer);
    RegisterSpellScript(spell_mage_frenetic_speed);
    RegisterSpellScript(spell_mage_ice_floes);
    RegisterSpellScript(spell_mage_imp_mana_gems);
    RegisterSpellScript(spell_mage_incanters_flow);
    RegisterSpellScript(spell_mage_living_bomb);
    RegisterSpellScript(spell_mage_living_bomb_explosion);
    RegisterSpellScript(spell_mage_living_bomb_periodic);
    RegisterSpellScript(spell_mage_mirror_image_summon);
    RegisterSpellScript(spell_mage_polymorph_visual);
    RegisterSpellScript(spell_mage_prismatic_barrier);
    RegisterSpellScript(spell_mage_pyroblast);
    RegisterSpellScript(spell_mage_pyroblast_clearcasting_driver);
    RegisterSpellScript(spell_mage_radiant_spark);
    RegisterSpellScript(spell_mage_ray_of_frost_aura);
    RegisterSpellScript(spell_mage_ray_of_frost_buff);
    RegisterSpellScriptWithArgs(spell_mage_unstable_magic, "spell_mage_unstable_magic_arcane_blast");
    RegisterSpellScriptWithArgs(spell_mage_unstable_magic, "spell_mage_unstable_magic_fireball");
    RegisterSpellScriptWithArgs(spell_mage_unstable_magic, "spell_mage_unstable_magic_frostbolt");
    RegisterSpellScript(spell_mage_ring_of_frost);
    RegisterSpellAndAuraScriptPair(spell_mage_ring_of_frost_freeze, spell_mage_ring_of_frost_freeze_AuraScript);
    RegisterSpellScript(spell_mage_supernova);
    RegisterSpellScript(spell_mage_tempest_barrier);
    RegisterSpellScript(spell_mage_touch_of_the_magi_aura);
    RegisterSpellScript(spell_mage_water_elemental_freeze);
}
