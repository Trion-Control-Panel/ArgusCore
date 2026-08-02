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
 * Scripts for spells with SPELLFAMILY_ROGUE and SPELLFAMILY_GENERIC spells used by rogue players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_rog_".
 */

#include "ScriptMgr.h"
#include "Containers.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum RogueSpells
{
    SPELL_ROGUE_ADRENALINE_RUSH                     = 13750,
    SPELL_ROGUE_BETWEEN_THE_EYES                    = 199804,
    SPELL_ROGUE_BLADE_FLURRY                        = 13877,
    SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK           = 22482,
    SPELL_ROGUE_BROADSIDE                           = 193356,
    SPELL_ROGUE_BURIED_TREASURE                     = 199600,
    SPELL_ROGUE_CANNONBALL_BARRAGE                  = 185767,
    SPELL_ROGUE_CANNONBALL_BARRAGE_DAMAGE           = 185779,
    SPELL_ROGUE_CANNONBALL_BARRAGE_SLOW             = 185778,
    SPELL_ROGUE_CHEAT_DEATH_DUMMY                   = 31231,
    SPELL_ROGUE_CHEATED_DEATH                       = 45181,
    SPELL_ROGUE_CHEATING_DEATH                      = 45182,
    SPELL_ROGUE_CRIPPLING_POISON                    = 3408,
    SPELL_ROGUE_CRIPPLING_POISON_DEBUFF             = 3409,
    SPELL_ROGUE_DEADLY_POISON                       = 2823,
    SPELL_ROGUE_DEADLY_POISON_DEBUFF                = 2818,
    SPELL_ROGUE_DEADLY_POISON_INSTANT_DAMAGE        = 113780,
    SPELL_ROGUE_DEEPENING_SHADOWS                   = 185314,
    SPELL_ROGUE_DEEPER_STRATAGEM                    = 193531,
    SPELL_ROGUE_ALACRITY                            = 193539,
    SPELL_ROGUE_ALACRITY_BUFF                       = 193538,
    SPELL_ROGUE_FAN_OF_KNIVES                       = 51723,
    SPELL_ROGUE_GRAND_MELEE                         = 193358,
    SPELL_ROGUE_GRAPPLING_HOOK                      = 195457,
    SPELL_ROGUE_GRAPPLING_HOOK_TRIGGER              = 227180,
    SPELL_ROGUE_KILLING_SPREE                       = 51690,
    SPELL_ROGUE_KILLING_SPREE_TELEPORT              = 57840,
    SPELL_ROGUE_KILLING_SPREE_WEAPON_DMG            = 57841,
    SPELL_ROGUE_KILLING_SPREE_DMG_BUFF              = 61851,
    SPELL_ROGUE_KIDNEY_SHOT                         = 408,
    SPELL_ROGUE_MARKED_FOR_DEATH                    = 137619,
    SPELL_ROGUE_MAIN_GAUCHE                         = 86392,
    SPELL_ROGUE_MASTER_OF_SHADOWS                   = 196976,
    SPELL_ROGUE_NIGHTBLADE                          = 195452,
    SPELL_ROGUE_NIGHT_TERRORS                       = 206760, // real id, cross-referenced by Nightblade's (195452) own tooltip (277953 is a later-expansion remake id)
    SPELL_ROGUE_NUMBING_POISON                      = 5761,
    SPELL_ROGUE_NUMBING_POISON_DEBUFF               = 5760,
    SPELL_ROGUE_PREMEDITATION_PASSIVE               = 196979, // real id "Premeditation" Passive (343160 is a later-expansion remake id)
    SPELL_ROGUE_PREMEDITATION_AURA                  = 235777, // real id, tooltip-linked to 196979 (343173 is a later-expansion remake id)
    SPELL_ROGUE_PREMEDITATION_ENERGIZE              = 343170,
    SPELL_ROGUE_PREY_ON_THE_WEAK_TALENT             = 131511,
    SPELL_ROGUE_PREY_ON_THE_WEAK                    = 131511, // real id "Prey on the Weak" (255909 is a later-expansion remake id)
    SPELL_ROGUE_RELENTLESS_STRIKES                  = 58423,
    SPELL_ROGUE_RUTHLESS_PRECISION                  = 193357,
    SPELL_ROGUE_SANCTUARY                           = 98877,
    SPELL_ROGUE_SHADOWSTEP_LEAP                     = 36563,
    SPELL_ROGUE_SKULL_AND_CROSSBONES                = 199603,
    SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS             = 196951,
    SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS_SLOW        = 222775,
    SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS_STUN        = 196958,
    SPELL_ROGUE_SHADOW_DANCE                        = 185313,
    SPELL_ROGUE_SHADOW_DANCE_AURA                   = 185422,
    SPELL_ROGUE_SHADOW_FOCUS                        = 108209,
    SPELL_ROGUE_SHADOW_FOCUS_EFFECT                 = 112942,
    SPELL_ROGUE_SHADOWS_GRASP                       = 206760,
    SPELL_ROGUE_SHADOW_TECHNIQUES_ENERGIZE          = 196911,
    SPELL_ROGUE_SHURIKEN_STORM_DAMAGE               = 197835,
    SPELL_ROGUE_SHURIKEN_STORM_ENERGIZE             = 212743,
    SPELL_ROGUE_SLICE_AND_DICE                      = 5171,
    SPELL_ROGUE_SPRINT                              = 2983,
    SPELL_ROGUE_STEALTH                             = 1784,
    SPELL_ROGUE_STEALTH_STEALTH_AURA                = 158185,
    SPELL_ROGUE_STEALTH_SHAPESHIFT_AURA             = 158188,
    SPELL_ROGUE_TRUE_BEARING                        = 193359,
    SPELL_ROGUE_TURN_THE_TABLES_BUFF                = 198027,
    SPELL_ROGUE_VANISH                              = 1856,
    SPELL_ROGUE_VANISH_AURA                         = 11327,
    SPELL_ROGUE_TRICKS_OF_THE_TRADE                 = 57934,
    SPELL_ROGUE_TRICKS_OF_THE_TRADE_PROC            = 59628,
    SPELL_ROGUE_HONOR_AMONG_THIEVES_ENERGIZE        = 51699,
    SPELL_ROGUE_SABER_SLASH                         = 193315,
    SPELL_ROGUE_OPPORTUNITY                         = 195627,
    SPELL_ROGUE_VENOMOUS_WOUNDS                     = 79134,
    SPELL_ROGUE_WOUND_POISON                        = 8679,
    SPELL_ROGUE_WOUND_POISON_DEBUFF                 = 8680,
};

static constexpr std::array<std::pair<uint32 /*poisonAura*/, uint32 /*triggeredPoisonSpell*/>, 4> PoisonAuraToDebuff
{{
    { SPELL_ROGUE_WOUND_POISON,      SPELL_ROGUE_WOUND_POISON_DEBUFF      },
    { SPELL_ROGUE_DEADLY_POISON,     SPELL_ROGUE_DEADLY_POISON_DEBUFF     },
    { SPELL_ROGUE_CRIPPLING_POISON,  SPELL_ROGUE_CRIPPLING_POISON_DEBUFF  },
    { SPELL_ROGUE_NUMBING_POISON,    SPELL_ROGUE_NUMBING_POISON_DEBUFF    }
}};

/* Returns true if the spell is a finishing move.
 * A finishing move is a spell that cost combo points */
Optional<int32> GetFinishingMoveCPCost(Spell const* spell)
{
    if (!spell)
        return { };

    return spell->GetPowerTypeCostAmount(POWER_COMBO_POINTS);
}

/* Return true if the spell is a finishing move.
 * A finishing move is a spell that cost combo points */
bool IsFinishingMove(Spell const* spell)
{
    return GetFinishingMoveCPCost(spell).has_value();
}

// 53 - Backstab
class spell_rog_backstab : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_3 } });
    }

    void HandleHitDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* hitUnit = GetHitUnit();
        if (!hitUnit)
            return;

        Unit* caster = GetCaster();
        if (hitUnit->isInBack(caster))
        {
            float currDamage = float(GetHitDamage());
            float newDamage = AddPct(currDamage, float(GetEffectInfo(EFFECT_3).CalcValue(caster)));
            SetHitDamage(newDamage);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_backstab::HandleHitDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 13877, 33735, (check 51211, 65956) - Blade Flurry
class spell_rog_blade_flurry : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        _procTarget = GetTarget()->SelectNearbyTarget(eventInfo.GetProcTarget());
        return _procTarget && eventInfo.GetDamageInfo();
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        if (DamageInfo* damageInfo = eventInfo.GetDamageInfo())
        {
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(damageInfo->GetDamage());
            GetTarget()->CastSpell(_procTarget, SPELL_ROGUE_BLADE_FLURRY_EXTRA_ATTACK, args);
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_rog_blade_flurry::CheckProc);
        if (m_scriptSpellId == SPELL_ROGUE_BLADE_FLURRY)
            OnEffectProc += AuraEffectProcFn(spell_rog_blade_flurry::HandleProc, EFFECT_0, SPELL_AURA_MOD_POWER_REGEN_PERCENT);
        else
            OnEffectProc += AuraEffectProcFn(spell_rog_blade_flurry::HandleProc, EFFECT_0, SPELL_AURA_MOD_MELEE_HASTE);
    }

    Unit* _procTarget = nullptr;
};

// 31230 - Cheat Death
class spell_rog_cheat_death : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_CHEAT_DEATH_DUMMY, SPELL_ROGUE_CHEATED_DEATH, SPELL_ROGUE_CHEATING_DEATH })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void HandleAbsorb(AuraEffect const* /*aurEff*/, DamageInfo const& /*dmgInfo*/, uint32& absorbAmount)
    {
        Unit* target = GetTarget();
        if (target->HasAura(SPELL_ROGUE_CHEATED_DEATH))
        {
            absorbAmount = 0;
            return;
        }

        PreventDefaultAction();

        target->CastSpell(target, SPELL_ROGUE_CHEAT_DEATH_DUMMY, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
        target->CastSpell(target, SPELL_ROGUE_CHEATED_DEATH, TRIGGERED_DONT_REPORT_CAST_ERROR);
        target->CastSpell(target, SPELL_ROGUE_CHEATING_DEATH, TRIGGERED_DONT_REPORT_CAST_ERROR);

        target->SetHealth(target->CountPctFromMaxHealth(GetEffectInfo(EFFECT_1).CalcValue(target)));
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_rog_cheat_death::HandleAbsorb, EFFECT_0);
    }
};

// 2818 - Deadly Poison
class spell_rog_deadly_poison : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_DEADLY_POISON_INSTANT_DAMAGE });
    }

    void HandleInstantDamage(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (target->HasAura(GetSpellInfo()->Id, caster->GetGUID()))
            caster->CastSpell(target, SPELL_ROGUE_DEADLY_POISON_INSTANT_DAMAGE, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR
            });
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_rog_deadly_poison::HandleInstantDamage, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 185314 - Deepening Shadows
class spell_rog_deepening_shadows : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_SHADOW_DANCE });
    }

    static bool CheckProc(AuraScript const&, AuraEffect const* /*aurEff*/, ProcEventInfo const& procEvent)
    {
        if (Spell const* procSpell = procEvent.GetProcSpell())
            return procSpell->GetPowerTypeCostAmount(POWER_COMBO_POINTS) > 0;

        return false;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& procInfo) const
    {
        Milliseconds amount = -Seconds(aurEff->GetAmount()) * *procInfo.GetProcSpell()->GetPowerTypeCostAmount(POWER_COMBO_POINTS);
        GetTarget()->GetSpellHistory()->ModifyChargeRecoveryTime(sSpellMgr->AssertSpellInfo(SPELL_ROGUE_SHADOW_DANCE, GetCastDifficulty())->ChargeCategoryId, amount / 10);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_rog_deepening_shadows::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_rog_deepening_shadows::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 185313 - Shadow Dance
// Subtlety's core stealth-enabling cooldown - was entirely missing its own cast effect
// (SPELL_ROGUE_SHADOW_DANCE was already referenced elsewhere, e.g. spell_rog_deepening_shadows
// above modifying its charge recovery time, but nothing actually applied the buff on cast).
class spell_rog_shadow_dance : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_SHADOW_DANCE_AURA, SPELL_ROGUE_MASTER_OF_SHADOWS });
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();

        if (caster->HasAura(SPELL_ROGUE_MASTER_OF_SHADOWS))
            caster->ModifyPower(POWER_ENERGY, 30);

        caster->CastSpell(caster, SPELL_ROGUE_SHADOW_DANCE_AURA, true);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_rog_shadow_dance::HandleHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 196912 - Shadow Techniques
// Baseline Rogue passive (all specs, long-standing since Cataclysm): melee auto-attacks have a
// chance to grant a combo point directly, independent of any finisher/builder. Was entirely
// missing - one of the more fundamental combo-point-generation mechanisms across all 3 specs.
// The proc chance is read from this aura's own DB2 data rather than hardcoded.
class spell_rog_shadow_techniques : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_SHADOW_TECHNIQUES_ENERGIZE });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        DamageInfo const* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || (damageInfo->GetAttackType() != BASE_ATTACK && damageInfo->GetAttackType() != OFF_ATTACK))
            return;

        if (roll_chance_i(aurEff->GetAmount()))
            eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_ROGUE_SHADOW_TECHNIQUES_ENERGIZE, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_rog_shadow_techniques::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 32645 - Envenom
class spell_rog_envenom : public SpellScript
{
    void CalculateDamage(SpellEffectInfo const& /*spellEffectInfo*/, Unit* /*victim*/, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        pctMod *= GetSpell()->GetPowerTypeCostAmount(POWER_COMBO_POINTS).value_or(0);
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_rog_envenom::CalculateDamage);
    }
};

// 196819 - Eviscerate
class spell_rog_eviscerate : public SpellScript
{
    void CalculateDamage(SpellEffectInfo const& /*spellEffectInfo*/, Unit* /*victim*/, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        pctMod *= GetSpell()->GetPowerTypeCostAmount(POWER_COMBO_POINTS).value_or(0);
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_rog_eviscerate::CalculateDamage);
    }
};

// 193358 - Grand Melee
class spell_rog_grand_melee : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_SLICE_AND_DICE });
    }

    bool HandleCheckProc(ProcEventInfo& eventInfo)
    {
        Spell const* procSpell = eventInfo.GetProcSpell();
        return procSpell && procSpell->HasPowerTypeCost(POWER_COMBO_POINTS);
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& procInfo)
    {
        Spell const* procSpell = procInfo.GetProcSpell();
        int32 amount = aurEff->GetAmount() * *procSpell->GetPowerTypeCostAmount(POWER_COMBO_POINTS) * 1000;

        if (Unit* target = GetTarget())
        {
            if (Aura* aura = target->GetAura(SPELL_ROGUE_SLICE_AND_DICE))
                aura->SetDuration(aura->GetDuration() + amount);
            else
            {
                CastSpellExtraArgs args;
                args.TriggerFlags = TRIGGERED_FULL_MASK;
                args.AddSpellMod(SPELLVALUE_DURATION, amount);
                target->CastSpell(target, SPELL_ROGUE_SLICE_AND_DICE, args);
            }
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_rog_grand_melee::HandleCheckProc);
        OnEffectProc += AuraEffectProcFn(spell_rog_grand_melee::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 198031 - Honor Among Thieves
/// 7.1.5
class spell_rog_honor_among_thieves : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_HONOR_AMONG_THIEVES_ENERGIZE });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_ROGUE_HONOR_AMONG_THIEVES_ENERGIZE, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_rog_honor_among_thieves::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 51690 - Killing Spree
class spell_rog_killing_spree_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_ROGUE_KILLING_SPREE_TELEPORT,
            SPELL_ROGUE_KILLING_SPREE_WEAPON_DMG,
            SPELL_ROGUE_KILLING_SPREE_DMG_BUFF
        });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_ROGUE_KILLING_SPREE_DMG_BUFF, true);
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        while (!_targets.empty())
        {
            ObjectGuid guid = Trinity::Containers::SelectRandomContainerElement(_targets);
            if (Unit* target = ObjectAccessor::GetUnit(*GetTarget(), guid))
            {
                GetTarget()->CastSpell(target, SPELL_ROGUE_KILLING_SPREE_TELEPORT, true);
                GetTarget()->CastSpell(target, SPELL_ROGUE_KILLING_SPREE_WEAPON_DMG, true);
                break;
            }
            else
                _targets.remove(guid);
        }
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_ROGUE_KILLING_SPREE_DMG_BUFF);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_rog_killing_spree_aura::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_killing_spree_aura::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_rog_killing_spree_aura::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }

public:
    void AddTarget(Unit* target)
    {
        _targets.push_back(target->GetGUID());
    }

private:
    GuidList _targets;
};

class spell_rog_killing_spree : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty() || GetCaster()->GetVehicleBase())
            FinishCast(SPELL_FAILED_OUT_OF_RANGE);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Aura* aura = GetCaster()->GetAura(SPELL_ROGUE_KILLING_SPREE))
            if (spell_rog_killing_spree_aura* script = aura->GetScript<spell_rog_killing_spree_aura>())
                script->AddTarget(GetHitUnit());
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_rog_killing_spree::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_rog_killing_spree::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// 408 - Kidney Shot
// Stun duration scales 1 sec per combo point spent (1-5, or 6 with Deeper Stratagem), rather than
// the fixed default duration - real, long-standing base mechanic missing entirely (only its
// downstream interactions, e.g. Prey on the Weak, existed in this file already).
class spell_rog_kidney_shot : public SpellScript
{
    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        int32 comboPoints = GetSpell()->GetPowerTypeCostAmount(POWER_COMBO_POINTS).value_or(0);
        if (Aura* aura = target->GetAura(SPELL_ROGUE_KIDNEY_SHOT, caster->GetGUID()))
            aura->SetDuration(comboPoints * IN_MILLISECONDS);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_rog_kidney_shot::HandleAfterHit);
    }
};

// 199804 - Between the Eyes
// Duration scales with combo points spent - same shape as spell_rog_kidney_shot above.
class spell_rog_between_the_eyes : public SpellScript
{
    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        int32 comboPoints = GetSpell()->GetPowerTypeCostAmount(POWER_COMBO_POINTS).value_or(0);
        if (Aura* aura = target->GetAura(SPELL_ROGUE_BETWEEN_THE_EYES, caster->GetGUID()))
            aura->SetDuration(comboPoints * IN_MILLISECONDS);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_rog_between_the_eyes::HandleAfterHit);
    }
};

// 195457 - Grappling Hook
class spell_rog_grappling_hook : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_GRAPPLING_HOOK_TRIGGER });
    }

    void HandleOnCast()
    {
        Unit* caster = GetCaster();
        WorldLocation const* dest = GetExplTargetDest();
        if (!caster || !dest)
            return;

        caster->CastSpell(*dest, SPELL_ROGUE_GRAPPLING_HOOK_TRIGGER, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_rog_grappling_hook::HandleOnCast);
    }
};

// 58423 - Relentless Strikes
// Finishing moves generate 5 Energy per combo point spent - generic across all finishers via
// Spell::GetPowerTypeCostAmount, the same "any combo-point-costing spell" idiom already used
// in this file for spell_rog_deepening_shadows above.
class spell_rog_relentless_strikes : public AuraScript
{
    static bool CheckProc(AuraScript const&, AuraEffect const* /*aurEff*/, ProcEventInfo const& procEvent)
    {
        if (Spell const* procSpell = procEvent.GetProcSpell())
            return procSpell->GetPowerTypeCostAmount(POWER_COMBO_POINTS) > 0;

        return false;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& procInfo) const
    {
        int32 comboPoints = *procInfo.GetProcSpell()->GetPowerTypeCostAmount(POWER_COMBO_POINTS);
        GetTarget()->ModifyPower(POWER_ENERGY, comboPoints * aurEff->GetAmount());
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_rog_relentless_strikes::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_rog_relentless_strikes::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 193539 - Alacrity
// Finishing moves have a 20% chance per combo point spent to grant a stacking 1% haste buff
// (per community guides - "up to 25 times"), generic across all finishers via
// Spell::GetPowerTypeCostAmount, the same idiom as spell_rog_relentless_strikes above.
// Implemented as its own standalone proc rather than duplicated inside every finisher script
// (the reference source inlines this same roll separately inside its Nightblade and
// Enveloping Shadows classes - this avoids that duplication).
class spell_rog_alacrity : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_ALACRITY_BUFF });
    }

    static bool CheckProc(AuraScript const&, AuraEffect const* /*aurEff*/, ProcEventInfo const& procEvent)
    {
        if (Spell const* procSpell = procEvent.GetProcSpell())
            return procSpell->GetPowerTypeCostAmount(POWER_COMBO_POINTS) > 0;

        return false;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& procInfo) const
    {
        int32 comboPoints = *procInfo.GetProcSpell()->GetPowerTypeCostAmount(POWER_COMBO_POINTS);
        if (roll_chance_i(20 * comboPoints))
            GetTarget()->CastSpell(GetTarget(), SPELL_ROGUE_ALACRITY_BUFF, true);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_rog_alacrity::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_rog_alacrity::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 185767 - Cannonball Barrage
// Slows the initial target on cast; the actual bombardment damage ticks periodically from a
// DynamicObject the cast itself spawns (SPELL_EFFECT_PERSISTENT_AREA_AURA via this spell's own
// DB2 data), matching the modern DB2-driven idiom already used elsewhere in this codebase
// rather than a manually-tracked AreaTrigger.
class spell_rog_cannonball_barrage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_CANNONBALL_BARRAGE_SLOW });
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_ROGUE_CANNONBALL_BARRAGE_SLOW, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_cannonball_barrage::HandleHit, EFFECT_2, SPELL_EFFECT_DUMMY);
    }
};

// 185767 - Cannonball Barrage (periodic bombardment damage)
class spell_rog_cannonball_barrage_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_CANNONBALL_BARRAGE, SPELL_ROGUE_CANNONBALL_BARRAGE_DAMAGE });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        DynamicObject* barrage = caster->GetDynObject(SPELL_ROGUE_CANNONBALL_BARRAGE);
        if (!barrage)
            return;

        caster->CastSpell(barrage->GetPosition(), SPELL_ROGUE_CANNONBALL_BARRAGE_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_cannonball_barrage_aura::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 385627 - Kingsbane
class spell_rog_kingsbane : public AuraScript
{
    bool CheckProc(AuraEffect const* /*aurEff*/, ProcEventInfo& procInfo)
    {
        return procInfo.GetActionTarget()->HasAura(GetId(), GetCasterGUID());
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_rog_kingsbane::CheckProc, EFFECT_4, SPELL_AURA_PROC_TRIGGER_SPELL);;
    }
};

// 76806 - Mastery: Main Gauche
class spell_rog_mastery_main_gauche : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_MAIN_GAUCHE });
    }

    bool HandleCheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetDamageInfo() && eventInfo.GetDamageInfo()->GetVictim();
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& procInfo)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(procInfo.GetDamageInfo()->GetVictim(), SPELL_ROGUE_MAIN_GAUCHE, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_rog_mastery_main_gauche::HandleCheckProc);
        OnEffectProc += AuraEffectProcFn(spell_rog_mastery_main_gauche::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 277953 - Night Terrors (attached to 197835 - Shuriken Storm)
class spell_rog_night_terrors : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_ROGUE_NIGHT_TERRORS, SPELL_ROGUE_SHADOWS_GRASP });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_ROGUE_NIGHT_TERRORS);
    }

    void HandleEnergize(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_ROGUE_SHADOWS_GRASP, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_night_terrors::HandleEnergize, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_rog_pickpocket : public SpellScript
{
    SpellCastResult CheckCast()
    {
        if (!GetExplTargetUnit() || !GetCaster()->IsValidAttackTarget(GetExplTargetUnit(), GetSpellInfo()))
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_rog_pickpocket::CheckCast);
    }
};

// 185565 - Poisoned Knife
class spell_rog_poisoned_knife : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_ROGUE_WOUND_POISON,
            SPELL_ROGUE_WOUND_POISON_DEBUFF,
            SPELL_ROGUE_DEADLY_POISON,
            SPELL_ROGUE_DEADLY_POISON_DEBUFF,
            SPELL_ROGUE_CRIPPLING_POISON,
            SPELL_ROGUE_CRIPPLING_POISON_DEBUFF,
            SPELL_ROGUE_NUMBING_POISON,
            SPELL_ROGUE_NUMBING_POISON_DEBUFF
        });
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        for (auto const& [poisonAura, debuffSpellId] : PoisonAuraToDebuff)
            if (caster->HasAura(poisonAura))
                caster->CastSpell(GetHitUnit(), debuffSpellId, CastSpellExtraArgsInit{
                    .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                    .TriggeringSpell = GetSpell()
                });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_poisoned_knife::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Called by 1784 - Stealth
class spell_rog_premeditation : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_PREMEDITATION_PASSIVE, SPELL_ROGUE_PREMEDITATION_AURA });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_ROGUE_PREMEDITATION_PASSIVE);
    }

    void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_ROGUE_PREMEDITATION_AURA, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_rog_premeditation::HandleEffectApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 343173 - Premeditation (proc)
class spell_rog_premeditation_proc : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_PREMEDITATION_ENERGIZE });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*procInfo*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_ROGUE_PREMEDITATION_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_rog_premeditation_proc::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 131511 - Prey on the Weak
// Called by Cheap Shot - 1833 and Kidney Shot - 408
class spell_rog_prey_on_the_weak : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_PREY_ON_THE_WEAK_TALENT, SPELL_ROGUE_PREY_ON_THE_WEAK });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_ROGUE_PREY_ON_THE_WEAK_TALENT))
                caster->CastSpell(GetTarget(), SPELL_ROGUE_PREY_ON_THE_WEAK, true);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_rog_prey_on_the_weak::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

// 79096 - Restless Blades
class spell_rog_restless_blades : public AuraScript
{
    static uint32 constexpr Spells[] = { SPELL_ROGUE_ADRENALINE_RUSH, SPELL_ROGUE_BETWEEN_THE_EYES, SPELL_ROGUE_SPRINT,
        SPELL_ROGUE_GRAPPLING_HOOK, SPELL_ROGUE_VANISH, SPELL_ROGUE_KILLING_SPREE, SPELL_ROGUE_MARKED_FOR_DEATH };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(Spells);
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& procInfo)
    {
        if (Optional<int32> spentCP = GetFinishingMoveCPCost(procInfo.GetProcSpell()))
        {
            int32 cdExtra = -(float(aurEff->GetAmount() * *spentCP) * 0.1f);

            SpellHistory* history = GetTarget()->GetSpellHistory();
            for (uint32 spellId : Spells)
                history->ModifyCooldown(spellId, Seconds(cdExtra), true);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_rog_restless_blades::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 315508 - Roll the Bones
class spell_rog_roll_the_bones : public SpellScript
{
    static uint32 constexpr Spells[] = { SPELL_ROGUE_SKULL_AND_CROSSBONES, SPELL_ROGUE_GRAND_MELEE, SPELL_ROGUE_RUTHLESS_PRECISION,
        SPELL_ROGUE_TRUE_BEARING, SPELL_ROGUE_BURIED_TREASURE, SPELL_ROGUE_BROADSIDE };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(Spells);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        int32 currentDuration = 0;
        for (uint32 spellId : Spells)
        {
            if (Aura* aura = GetCaster()->GetAura(spellId))
            {
                currentDuration = aura->GetDuration();
                GetCaster()->RemoveAura(aura);
            }
        }

        std::vector<uint32> possibleBuffs(std::begin(Spells), std::end(Spells));
        Trinity::Containers::RandomShuffle(possibleBuffs);

        // https://www.icy-veins.com/wow/outlaw-rogue-pve-dps-rotation-cooldowns-abilities
        // 1 Roll the Bones buff  : 100.0 % chance;
        // 2 Roll the Bones buffs : 19 % chance;
        // 5 Roll the Bones buffs : 1 % chance.
        int32 chance = irand(1, 100);
        int32 numBuffs = 1;
        if (chance <= 1)
            numBuffs = 5;
        else if (chance <= 20)
            numBuffs = 2;

        for (int32 i = 0; i < numBuffs; ++i)
        {
            uint32 spellId = possibleBuffs[i];
            CastSpellExtraArgs args;
            args.TriggerFlags = TRIGGERED_FULL_MASK;
            args.AddSpellMod(SPELLVALUE_DURATION, GetSpellInfo()->GetDuration() + currentDuration);
            GetCaster()->CastSpell(GetCaster(), spellId, args);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_roll_the_bones::HandleDummy, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 1943 - Rupture
class spell_rog_rupture : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_VENOMOUS_WOUNDS });
    }

    void OnEffectRemoved(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            return;

        Aura* aura = GetAura();
        Unit* caster = aura->GetCaster();
        if (!caster)
            return;

        Aura* auraVenomousWounds = caster->GetAura(SPELL_ROGUE_VENOMOUS_WOUNDS);
        if (!auraVenomousWounds)
            return;

        // Venomous Wounds: if unit dies while being affected by rupture, regain energy based on remaining duration
        Optional<SpellPowerCost> cost = GetSpellInfo()->CalcPowerCost(POWER_ENERGY, false, caster, GetSpellInfo()->GetSchoolMask(), nullptr);
        if (!cost)
            return;

        float pct = float(aura->GetDuration()) / float(aura->GetMaxDuration());
        int32 extraAmount = float(cost->Amount) * pct;
        caster->ModifyPower(POWER_ENERGY, extraAmount);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_rog_rupture::OnEffectRemoved, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 14161 - Ruthlessness
class spell_rog_ruthlessness : public AuraScript
{
    void HandleProc(AuraEffect* aurEff, ProcEventInfo& procInfo)
    {
        Unit* target = GetTarget();

        if (Optional<int32> cost = GetFinishingMoveCPCost(procInfo.GetProcSpell()))
            if (roll_chance_i(aurEff->GetSpellEffectInfo().PointsPerResource * (*cost)))
                target->ModifyPower(POWER_COMBO_POINTS, 1);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_rog_ruthlessness::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 185438 - Shadowstrike
class spell_rog_shadowstrike : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_PREMEDITATION_AURA, SPELL_ROGUE_SLICE_AND_DICE, SPELL_ROGUE_PREMEDITATION_PASSIVE,
                SPELL_ROGUE_SHADOWSTEP_LEAP, SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS_SLOW, SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS_STUN })
            && ValidateSpellEffect({ { SPELL_ROGUE_PREMEDITATION_PASSIVE, EFFECT_0 } });
    }

    SpellCastResult HandleCheckCast()
    {
        // Because the premeditation aura is removed when we're out of stealth,
        // when we reach HandleEnergize the aura won't be there, even if it was when player launched the spell
        _hasPremeditationAura = GetCaster()->HasAura(SPELL_ROGUE_PREMEDITATION_AURA);
        return SPELL_FAILED_SUCCESS;
    }

    // Shadowstrike's whole identity - teleport behind the target before the strike lands.
    // Was completely missing: this ability did not reposition the caster at all previously.
    void HandleLeapBehind()
    {
        if (Unit* target = GetExplTargetUnit())
            GetCaster()->CastSpell(target, SPELL_ROGUE_SHADOWSTEP_LEAP, true);
    }

    void HandleEnergize(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (_hasPremeditationAura)
        {
            if (caster->HasAura(SPELL_ROGUE_SLICE_AND_DICE))
                if (Aura* premeditationPassive = caster->GetAura(SPELL_ROGUE_PREMEDITATION_PASSIVE))
                    if (AuraEffect const* auraEff = premeditationPassive->GetEffect(EFFECT_1))
                        SetHitDamage(GetHitDamage() + auraEff->GetAmount());

            // Grant 10 seconds of slice and dice
            int32 duration = sSpellMgr->AssertSpellInfo(SPELL_ROGUE_PREMEDITATION_PASSIVE, DIFFICULTY_NONE)->GetEffect(EFFECT_0).CalcValue(GetCaster());

            CastSpellExtraArgs args;
            args.TriggerFlags = TRIGGERED_FULL_MASK;
            args.AddSpellMod(SPELLVALUE_DURATION, duration * IN_MILLISECONDS);
            caster->CastSpell(caster, SPELL_ROGUE_SLICE_AND_DICE, args);
        }
    }

    // Strike from the Shadows (level 75 talent, patch 7.0.3-8.0.1): applies a snare to players,
    // a stun to NPCs (a common Blizzard PvP-balance split for on-hit CC talents).
    // NOTE: hooked to EFFECT_0/SPELL_EFFECT_WEAPON_PERCENT_DAMAGE, inferred from this file's own
    // already-confirmed EFFECT_1 == SPELL_EFFECT_ENERGIZE above (the reference source used
    // EFFECT_1 for this piece too, which would collide with the energize effect here - ArgusCore's
    // actual effect layout for this spell differs from the reference's). If this hook doesn't
    // fire, the effect index is the first thing to check.
    void HandleHitTarget(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (caster->HasAura(SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS))
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
                caster->CastSpell(target, SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS_SLOW, true);
            else
                caster->CastSpell(target, SPELL_ROGUE_STRIKE_FROM_THE_SHADOWS_STUN, true);
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_rog_shadowstrike::HandleCheckCast);
        BeforeCast += SpellCastFn(spell_rog_shadowstrike::HandleLeapBehind);
        OnEffectHitTarget += SpellEffectFn(spell_rog_shadowstrike::HandleEnergize, EFFECT_1, SPELL_EFFECT_ENERGIZE);
        OnEffectHitTarget += SpellEffectFn(spell_rog_shadowstrike::HandleHitTarget, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }

private:
    bool _hasPremeditationAura = false;
};

// Called by 1784 - Stealth and 185313 - Shadow Dance
class spell_rog_shadow_focus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_SHADOW_FOCUS, SPELL_ROGUE_SHADOW_FOCUS_EFFECT });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_ROGUE_SHADOW_FOCUS);
    }

    void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_ROGUE_SHADOW_FOCUS_EFFECT, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_ROGUE_SHADOW_FOCUS_EFFECT);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_rog_shadow_focus::HandleEffectApply, EFFECT_1, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_rog_shadow_focus::HandleEffectRemove, EFFECT_1, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 197835 - Shuriken Storm
class spell_rog_shuriken_storm : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_ROGUE_SHURIKEN_STORM_ENERGIZE });
    }

    void HandleEnergize(SpellEffIndex effIndex) const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_ROGUE_SHURIKEN_STORM_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell(),
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, static_cast<int32>(GetUnitTargetCountForEffect(effIndex))  } }
        });
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_rog_shuriken_storm::HandleEnergize, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 51723 - Fan of Knives
// Assassination's AoE builder - generates 1 combo point per cast (not per target hit).
// The reference source also clears old Killing Spree debuffs by hardcoded id on this cast,
// which looks like stale drift from a much older Killing Spree shape - dropped rather than
// ported.
class spell_rog_fan_of_knives : public SpellScript
{
    void HandleOnCast()
    {
        GetCaster()->ModifyPower(POWER_COMBO_POINTS, 1);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_rog_fan_of_knives::HandleOnCast);
    }
};

// 277925 - Shuriken Tornado
class spell_rog_shuriken_tornado : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_SHURIKEN_STORM_DAMAGE });
    }

    void HandlePeriodicEffect(AuraEffect const* aurEff) const
    {
        GetTarget()->CastSpell(nullptr, SPELL_ROGUE_SHURIKEN_STORM_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_POWER_COST | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_shuriken_tornado::HandlePeriodicEffect, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 193315 - Saber Slash (Outlaw's basic builder)
// Was previously mislabeled in this file as "Sinister Strike" and carried a Burning Crusade
// Tier 5 2-piece set bonus check (37169) that has no place in Legion at all - same dead-content
// pattern found and removed from Envenom/Eviscerate above. Confirmed via three independent
// reference lineages that 193315 is actually Saber Slash: a chance to strike again for free,
// granting the Opportunity buff (which lets the next Pistol Shot be cast free) and an extra
// combo point, with the chance boosted by the Skull and Crossbones buff (one of Roll the Bones'
// possible rolls, already implemented elsewhere in this file).
class spell_rog_saber_slash : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_SABER_SLASH, SPELL_ROGUE_OPPORTUNITY, SPELL_ROGUE_SKULL_AND_CROSSBONES })
            && ValidateSpellEffect({ { SPELL_ROGUE_SABER_SLASH, EFFECT_4 } });
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        SpellInfo const* saberSlash = sSpellMgr->AssertSpellInfo(SPELL_ROGUE_SABER_SLASH, GetCastDifficulty());
        int32 chance = saberSlash->GetEffect(EFFECT_4).CalcValue(caster);

        if (AuraEffect const* skullAndCrossbones = caster->GetAuraEffect(SPELL_ROGUE_SKULL_AND_CROSSBONES, EFFECT_0))
            chance += skullAndCrossbones->GetAmount();

        if (!roll_chance_i(chance))
            return;

        caster->CastSpell(caster, SPELL_ROGUE_OPPORTUNITY, true);

        SpellNonMeleeDamage log(caster, target, GetSpellInfo(), GetSpell()->m_SpellVisual, GetSpellInfo()->SchoolMask);
        log.damage = GetHitDamage();
        caster->DealSpellDamage(&log, false);
        caster->SendSpellNonMeleeDamageLog(&log);

        caster->ModifyPower(POWER_COMBO_POINTS, 1);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_saber_slash::HandleHit, EFFECT_2, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// 193537 - Weaponmaster
// Chance on a weapon hit to duplicate the damage as a free extra instance. Chance read from
// this aura's own DB2 data. Uses the same SpellNonMeleeDamage/DealSpellDamage idiom already
// established in this file for spell_rog_saber_slash's extra-hit mechanic above.
class spell_rog_weaponmaster : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetDamageInfo() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        if (!roll_chance_i(aurEff->GetAmount()))
            return;

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target)
            return;

        SpellNonMeleeDamage log(caster, target, GetSpellInfo(), {}, GetSpellInfo()->SchoolMask);
        log.damage = eventInfo.GetDamageInfo()->GetDamage();
        log.cleanDamage = log.damage;
        caster->DealSpellDamage(&log, false);
        caster->SendSpellNonMeleeDamageLog(&log);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_rog_weaponmaster::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_rog_weaponmaster::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 1784 - Stealth
class spell_rog_stealth : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_ROGUE_SANCTUARY,
            SPELL_ROGUE_STEALTH_STEALTH_AURA,
            SPELL_ROGUE_STEALTH_SHAPESHIFT_AURA
        });
    }

    void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();

        target->CastSpell(target, SPELL_ROGUE_SANCTUARY, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
        target->CastSpell(target, SPELL_ROGUE_STEALTH_STEALTH_AURA, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
        target->CastSpell(target, SPELL_ROGUE_STEALTH_SHAPESHIFT_AURA, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();

        target->RemoveAurasDueToSpell(SPELL_ROGUE_STEALTH_STEALTH_AURA);
        target->RemoveAurasDueToSpell(SPELL_ROGUE_STEALTH_SHAPESHIFT_AURA);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_rog_stealth::HandleEffectApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_rog_stealth::HandleEffectRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 57934 - Tricks of the Trade
class spell_rog_tricks_of_the_trade_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_TRICKS_OF_THE_TRADE_PROC });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEFAULT || !GetTarget()->HasAura(SPELL_ROGUE_TRICKS_OF_THE_TRADE_PROC))
            GetTarget()->GetThreatManager().UnregisterRedirectThreat(SPELL_ROGUE_TRICKS_OF_THE_TRADE);
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* rogue = GetTarget();
        if (ObjectAccessor::GetUnit(*rogue, _redirectTarget))
            rogue->CastSpell(rogue, SPELL_ROGUE_TRICKS_OF_THE_TRADE_PROC, aurEff);
        Remove(AURA_REMOVE_BY_DEFAULT);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_rog_tricks_of_the_trade_aura::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectProc += AuraEffectProcFn(spell_rog_tricks_of_the_trade_aura::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }

    ObjectGuid _redirectTarget;
public:
    void SetRedirectTarget(ObjectGuid guid) { _redirectTarget = guid; }
};

// 57934 - Tricks of the Trade
class spell_rog_tricks_of_the_trade : public SpellScript
{
    void DoAfterHit()
    {
        if (Aura* aura = GetHitAura())
            if (spell_rog_tricks_of_the_trade_aura* script = aura->GetScript<spell_rog_tricks_of_the_trade_aura>())
            {
                if (Unit* explTarget = GetExplTargetUnit())
                    script->SetRedirectTarget(explTarget->GetGUID());
                else
                    script->SetRedirectTarget(ObjectGuid::Empty);
            }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_rog_tricks_of_the_trade::DoAfterHit);
    }
};

// 59628 - Tricks of the Trade (Proc)
class spell_rog_tricks_of_the_trade_proc : public AuraScript
{
    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->GetThreatManager().UnregisterRedirectThreat(SPELL_ROGUE_TRICKS_OF_THE_TRADE);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_rog_tricks_of_the_trade_proc::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 198020 - Turn the Tables (PvP Talent)
class spell_rog_turn_the_tables : public AuraScript
{
    bool CheckForStun(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcSpell() && eventInfo.GetProcSpell()->GetSpellInfo()->HasAura(SPELL_AURA_MOD_STUN);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_rog_turn_the_tables::CheckForStun, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 198023 - Turn the Tables (periodic)
class spell_rog_turn_the_tables_periodic_check : public AuraScript
{
    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_TURN_THE_TABLES_BUFF });
    }

    void CheckForStun(AuraEffect const* aurEff)
    {
        Unit* target = GetTarget();
        if (!target->HasAuraType(SPELL_AURA_MOD_STUN))
        {
            target->CastSpell(target, SPELL_ROGUE_TURN_THE_TABLES_BUFF, aurEff);
            PreventDefaultAction();
            Remove();
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_turn_the_tables_periodic_check::CheckForStun, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 1856 - Vanish - SPELL_ROGUE_VANISH
class spell_rog_vanish : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_VANISH_AURA, SPELL_ROGUE_STEALTH_SHAPESHIFT_AURA });
    }

    void OnLaunchTarget(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Unit* target = GetHitUnit();

        target->RemoveAurasByType(SPELL_AURA_MOD_STALKED);
        if (target->GetTypeId() != TYPEID_PLAYER)
            return;

        if (target->HasAura(SPELL_ROGUE_VANISH_AURA))
            return;

        target->CastSpell(target, SPELL_ROGUE_VANISH_AURA, TRIGGERED_FULL_MASK);
        target->CastSpell(target, SPELL_ROGUE_STEALTH_SHAPESHIFT_AURA, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_rog_vanish::OnLaunchTarget, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

// 11327 - Vanish
class spell_rog_vanish_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROGUE_STEALTH });
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_ROGUE_STEALTH, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_rog_vanish_aura::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 79134 - Venomous Wounds
class spell_rog_venomous_wounds : public AuraScript
{
    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        int32 extraEnergy = aurEff->GetAmount();
        GetTarget()->ModifyPower(POWER_ENERGY, extraEnergy);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_rog_venomous_wounds::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

void AddSC_rogue_spell_scripts()
{
    RegisterSpellScript(spell_rog_backstab);
    RegisterSpellScript(spell_rog_blade_flurry);
    RegisterSpellScript(spell_rog_alacrity);
    RegisterSpellAndAuraScriptPair(spell_rog_cannonball_barrage, spell_rog_cannonball_barrage_aura);
    RegisterSpellScript(spell_rog_cheat_death);
    RegisterSpellScript(spell_rog_deadly_poison);
    RegisterSpellScript(spell_rog_deepening_shadows);
    RegisterSpellScript(spell_rog_envenom);
    RegisterSpellScript(spell_rog_eviscerate);
    RegisterSpellScript(spell_rog_grand_melee);
    RegisterSpellScript(spell_rog_grappling_hook);
    RegisterSpellScript(spell_rog_relentless_strikes);
    RegisterSpellScript(spell_rog_honor_among_thieves);
    RegisterSpellAndAuraScriptPair(spell_rog_killing_spree, spell_rog_killing_spree_aura);
    RegisterSpellScript(spell_rog_kidney_shot);
    RegisterSpellScript(spell_rog_between_the_eyes);
    RegisterSpellScript(spell_rog_kingsbane);
    RegisterSpellScript(spell_rog_mastery_main_gauche);
    RegisterSpellScript(spell_rog_night_terrors);
    RegisterSpellScript(spell_rog_pickpocket);
    RegisterSpellScript(spell_rog_poisoned_knife);
    RegisterSpellScript(spell_rog_premeditation);
    RegisterSpellScript(spell_rog_premeditation_proc);
    RegisterSpellScript(spell_rog_prey_on_the_weak);
    RegisterSpellScript(spell_rog_restless_blades);
    RegisterSpellScript(spell_rog_roll_the_bones);
    RegisterSpellScript(spell_rog_rupture);
    RegisterSpellScript(spell_rog_ruthlessness);
    RegisterSpellScript(spell_rog_shadow_dance);
    RegisterSpellScript(spell_rog_shadow_techniques);
    RegisterSpellScript(spell_rog_fan_of_knives);
    RegisterSpellScript(spell_rog_shadowstrike);
    RegisterSpellScript(spell_rog_shadow_focus);
    RegisterSpellScript(spell_rog_shuriken_storm);
    RegisterSpellScript(spell_rog_shuriken_tornado);
    RegisterSpellScript(spell_rog_saber_slash);
    RegisterSpellScript(spell_rog_stealth);
    RegisterSpellScript(spell_rog_weaponmaster);
    RegisterSpellAndAuraScriptPair(spell_rog_tricks_of_the_trade, spell_rog_tricks_of_the_trade_aura);
    RegisterSpellScript(spell_rog_tricks_of_the_trade_proc);
    RegisterSpellScript(spell_rog_turn_the_tables);
    RegisterSpellScript(spell_rog_turn_the_tables_periodic_check);
    RegisterSpellScript(spell_rog_vanish);
    RegisterSpellScript(spell_rog_vanish_aura);
    RegisterSpellScript(spell_rog_venomous_wounds);
}
