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
 * Scripts for spells with SPELLFAMILY_MONK and SPELLFAMILY_GENERIC spells used by monk players.
 * Scriptnames of files in this file should be prefixed with "spell_monk_".
 */

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "CommonPredicates.h"
#include "DB2Stores.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PathGenerator.h"
#include "Player.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum MonkSpells
{
    SPELL_MONK_BURST_OF_LIFE_TALENT                     = 399226,
    SPELL_MONK_BURST_OF_LIFE_HEAL                       = 399230,
    SPELL_MONK_CALMING_COALESCENCE                      = 388220,
    SPELL_MONK_COMBAT_CONDITIONING                      = 128595,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHANNEL         = 117952,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHI_PROC        = 123333,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK       = 117962,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK_CD    = 117953,
    SPELL_MONK_ENVELOPING_MIST                          = 124682,
    SPELL_MONK_ENVELOPING_MIST_HEAL                     = 132120,
    SPELL_MONK_FISTS_OF_FURY_DAMAGE                     = 117418,
    SPELL_MONK_FISTS_OF_FURY_VISUAL                     = 123154,
    SPELL_MONK_FORTIFYING_BREW                          = 120954,
    SPELL_MONK_GIFT_OF_THE_OX_AURA                       = 124502,
    SPELL_MONK_GIFT_OF_THE_OX_AT_RIGHT                   = 124503,
    SPELL_MONK_GIFT_OF_THE_OX_AT_LEFT                    = 124506,
    SPELL_MONK_GIFT_OF_THE_OX_HEAL                       = 178173,
    SPELL_MONK_HEALING_SPHERE_COOLDOWN                   = 224863,
    SPELL_MONK_JADE_WALK                                = 450552,
    SPELL_MONK_MISTS_OF_LIFE                            = 388548,
    SPELL_MONK_MORTAL_WOUNDS                            = 115804,
    SPELL_MONK_POWER_STRIKE_PROC                        = 129914,
    SPELL_MONK_POWER_STRIKE_ENERGIZE                    = 121283,
    SPELL_MONK_PRESSURE_POINTS                          = 450432,
    SPELL_MONK_PROVOKE_SINGLE_TARGET                    = 116189,
    SPELL_MONK_PROVOKE_AOE                              = 118635,
    SPELL_MONK_NO_FEATHER_FALL                          = 79636,
    SPELL_MONK_OPEN_PALM_STRIKES_TALENT                 = 392970,
    SPELL_MONK_RENEWING_MIST                            = 119611,
    SPELL_MONK_RISING_SUN_KICK                          = 107428,
    SPELL_MONK_RISING_THUNDER                           = 210804,
    SPELL_MONK_ROLL_BACKWARD                            = 109131,
    SPELL_MONK_ROLL_FORWARD                             = 107427,
    SPELL_MONK_SAVE_THEM_ALL_HEAL_BONUS                 = 390105,
    SPELL_MONK_SONG_OF_CHI_JI_STUN                      = 198909,
    SPELL_MONK_SOOTHING_MIST                            = 115175,
    SPELL_MONK_SOOTHING_MIST_ENERGIZE                   = 116335,
    SPELL_MONK_SOOTHING_MIST_VISUAL                     = 125955,
    SPELL_MONK_SPIRIT_OF_THE_CRANE_AURA                 = 210802,
    SPELL_MONK_SPIRIT_OF_THE_CRANE_MANA                 = 210803,
    SPELL_MONK_STANCE_OF_THE_SPIRITED_CRANE             = 154436,
    SPELL_MONK_STAGGER_DAMAGE_AURA                      = 124255,
    SPELL_MONK_STAGGER_HEAVY                            = 124273,
    SPELL_MONK_STAGGER_LIGHT                            = 124275,
    SPELL_MONK_STAGGER_MODERATE                         = 124274,
    SPELL_MONK_SURGING_MIST_HEAL                        = 116995,
    SPELL_MONK_TEACHINGS_OF_THE_MONASTERY               = 116645,
    SPELL_MONK_THUNDER_FOCUS_TEA                        = 116680,
    SPELL_MONK_TEACHINGS_OF_THE_MONASTERY_AURA          = 202090,
    SPELL_MONK_TOUCH_OF_DEATH                           = 115080,
    SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE           = 124280,
};

// 100784 - Blackout Kick
// Windwalker/Mistweaver builder-consumer interaction with Teachings of the Monastery: Tiger
// Palm grants stacks of a buff (202090) that Blackout Kick consumes for bonus damage, and
// separately has a chance to reset Rising Sun Kick's cooldown/charges if the talent (116645)
// is known. Mistweavers with Spirit of the Crane (210802) refund mana based on stacks consumed.
// NOTE: DestinyCore's reference re-deals the hit's damage once per stack via a manually
// constructed SpellNonMeleeDamage (producing N separate combat-log entries) - that low-level
// damage-dealing pattern has no precedent anywhere in ArgusCore's engine and couldn't be
// verified to behave correctly here, so this instead multiplies the single hit's damage by
// (stacks + 1), which delivers the same total damage as one combat-log entry rather than N.
// The RSK-reset percentage (15%) is DestinyCore's own value, not independently verified
// against Legion 7.3.5 client data - flagged the same way Tactician's proc-chance was earlier.
class spell_monk_blackout_kick : public SpellScript
{
    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (caster->HasAura(SPELL_MONK_TEACHINGS_OF_THE_MONASTERY) && roll_chance_i(15))
        {
            caster->GetSpellHistory()->ResetCooldown(SPELL_MONK_RISING_SUN_KICK, true);
            if (SpellInfo const* risingSunKick = sSpellMgr->GetSpellInfo(SPELL_MONK_RISING_SUN_KICK, DIFFICULTY_NONE))
                caster->GetSpellHistory()->ResetCharges(risingSunKick->ChargeCategoryId);
        }

        if (Aura* teachings = caster->GetAura(SPELL_MONK_TEACHINGS_OF_THE_MONASTERY_AURA))
        {
            int32 stacks = teachings->GetStackAmount();
            caster->RemoveAurasDueToSpell(SPELL_MONK_TEACHINGS_OF_THE_MONASTERY_AURA);

            SetHitDamage(GetHitDamage() * (stacks + 1));

            if (caster->HasAura(SPELL_MONK_SPIRIT_OF_THE_CRANE_AURA))
            {
                CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
                args.AddSpellBP0(int32(0.65f * float(stacks)));
                caster->CastSpell(caster, SPELL_MONK_SPIRIT_OF_THE_CRANE_MANA, args);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_blackout_kick::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 399226 - Burst of Life (attached to 116849 - Life Cocoon)
class spell_monk_burst_of_life : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_BURST_OF_LIFE_HEAL })
            && ValidateSpellEffect({ { SPELL_MONK_BURST_OF_LIFE_TALENT, EFFECT_0 } });
    }

    bool Load() override
    {
        Unit* caster = GetCaster();
        return caster && caster->HasAuraEffect(SPELL_MONK_BURST_OF_LIFE_TALENT, EFFECT_0);
    }

    void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/) const
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode != AURA_REMOVE_BY_EXPIRE && (removeMode != AURA_REMOVE_BY_ENEMY_SPELL || aurEff->GetAmount()))
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        AuraEffect const* burstOfLife = caster->GetAuraEffect(SPELL_MONK_BURST_OF_LIFE_TALENT, EFFECT_0);
        if (!burstOfLife)
            return;

        caster->CastSpell(GetTarget(), SPELL_MONK_BURST_OF_LIFE_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .SpellValueOverrides = { { SPELLVALUE_MAX_TARGETS, burstOfLife->GetAmount() } }
        });
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_monk_burst_of_life::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

// 399230 - Burst of Life
class spell_monk_burst_of_life_heal : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets) const
    {
        Trinity::SelectRandomInjuredTargets(targets, GetSpellValue()->MaxAffectedTargets, true, GetExplTargetUnit());
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_burst_of_life_heal::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 117952 - Crackling Jade Lightning
class spell_monk_crackling_jade_lightning : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_MONK_STANCE_OF_THE_SPIRITED_CRANE,
            SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHI_PROC
        });
    }

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_MONK_STANCE_OF_THE_SPIRITED_CRANE))
                caster->CastSpell(caster, SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHI_PROC, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_crackling_jade_lightning::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 117959 - Crackling Jade Lightning
class spell_monk_crackling_jade_lightning_knockback_proc_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK,
            SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK_CD
        });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (GetTarget()->HasAura(SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK_CD))
            return false;

        if (eventInfo.GetActor()->HasAura(SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHANNEL, GetTarget()->GetGUID()))
            return false;

        Spell* currentChanneledSpell = GetTarget()->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (!currentChanneledSpell || currentChanneledSpell->GetSpellInfo()->Id != SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHANNEL)
            return false;

        return true;
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        GetTarget()->CastSpell(eventInfo.GetActor(), SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK, TRIGGERED_FULL_MASK);
        GetTarget()->CastSpell(GetTarget(), SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK_CD, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_monk_crackling_jade_lightning_knockback_proc_aura::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_monk_crackling_jade_lightning_knockback_proc_aura::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 122278 - Dampen Harm
// Charge-based defensive: absorbs a percentage of any single hit large enough to exceed a
// health-percentage threshold, consuming one charge per triggered absorb.
// NOTE: DestinyCore's reference computes the absorb as
// "dmgInfo.GetDamage() * (CalcValue(EFFECT_0) / 100)" - since CalcValue returns int32, that
// inner division truncates to 0 for any percentage under 100 (integer division), making the
// absorb always zero. Used CalculatePct (float-based, matching the convention already used
// throughout this codebase) instead of porting that division verbatim.
class spell_monk_dampen_harm : public AuraScript
{
    int32 _healthPct = 0;

    bool Load() override
    {
        _healthPct = GetEffectInfo(EFFECT_0).CalcValue(GetCaster());
        return GetUnitOwner()->ToPlayer() != nullptr;
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* target = GetTarget();
        uint32 threshold = target->CountPctFromMaxHealth(_healthPct);
        if (dmgInfo.GetDamage() < threshold)
            return;

        absorbAmount = uint32(CalculatePct(dmgInfo.GetDamage(), _healthPct));
        aurEff->GetBase()->DropCharge();
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_dampen_harm::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_dampen_harm::Absorb, EFFECT_0);
    }
};

// 115288 - Energizing Brew
// Only usable in combat.
class spell_monk_energizing_brew : public SpellScript
{
    SpellCastResult CheckFight()
    {
        if (!GetCaster()->IsInCombat())
            return SPELL_FAILED_CASTER_AURASTATE;
        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_monk_energizing_brew::CheckFight);
    }
};

// 124682 - Enveloping Mist
// Casts the actual heal (132120) after the cast completes. Already relied upon by the
// existing spell_monk_mists_of_life class in this file, which casts 124682 directly and
// expects it to heal - without this script, that talent's Enveloping Mist half was inert.
class spell_monk_enveloping_mist : public SpellScript
{
    void HandleAfterCast()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_MONK_ENVELOPING_MIST_HEAL, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_monk_enveloping_mist::HandleAfterCast);
    }
};

// 113656 - Fists of Fury
// Channeled aura; ticks fast for the visual pulse, but only fires the actual damage sub-spell
// every 6th tick.
class spell_monk_fists_of_fury : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_FISTS_OF_FURY_DAMAGE });
    }

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (aurEff->GetTickNumber() % 6 == 0)
            caster->CastSpell(GetTarget(), SPELL_MONK_FISTS_OF_FURY_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_fists_of_fury::HandlePeriodic, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 117418 - Fists of Fury (damage)
// Manual damage formula (AttackPower * 5.25, then the normal damage-bonus-done/taken pipeline)
// since this coefficient isn't expressible via a plain weapon-percent-damage effect. The
// coefficient itself is DestinyCore's own value, not independently verified against Legion
// 7.3.5 client data.
class spell_monk_fists_of_fury_damage : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        int32 damage = int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 5.25f);
        damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE, GetEffectInfo(EFFECT_0));
        damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE);

        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_fists_of_fury_damage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 117418 - Fists of Fury (target filter)
// Same spell id as the damage script above - TrinityCore-style script binding supports
// multiple independent script classes on one spell id. Prevents re-targeting a unit already
// hit by the visual sweep aura (123154) this tick.
class spell_monk_fists_of_fury_visual_filter : public SpellScript
{
    void RemoveInvalidTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_MONK_FISTS_OF_FURY_VISUAL, caster->GetGUID()));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_fists_of_fury_visual_filter::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CONE_ENEMY_24);
    }
};

// 123154 - Fists of Fury (visual sweep)
// This aura has no duration in client data and would never end without one - a defensive
// workaround from DestinyCore's own reference, not a guessed value; the actual channel length
// is governed by the main Fists of Fury cast (113656) regardless.
class spell_monk_fists_of_fury_visual : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        SetMaxDuration(1000);
        SetDuration(1000);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_monk_fists_of_fury_visual::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 115203 - Fortifying Brew
// Thin wrapper: applies the actual defensive buff (120954, SPELL_MONK_FORTIFYING_BREW) on hit.
class spell_monk_fortifying_brew : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (caster)
            caster->CastSpell(caster, SPELL_MONK_FORTIFYING_BREW, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_fortifying_brew::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 450553 - Jade Walk
class spell_monk_jade_walk : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_JADE_WALK });
    }

    void HandlePeriodicTick(AuraEffect const* aurEff)
    {
        Unit* target = GetTarget();
        if (!target->IsInCombat())
            target->CastSpell(target, SPELL_MONK_JADE_WALK, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                .TriggeringAura = aurEff
            });
        else
            target->RemoveAurasDueToSpell(SPELL_MONK_JADE_WALK);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_jade_walk::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 116849 - Life Cocoon
class spell_monk_life_cocoon : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_CALMING_COALESCENCE });
    }

    void CalculateAbsorb(SpellEffIndex /*effIndex*/)
    {
        int32 absorb = GetCaster()->CountPctFromMaxHealth(GetEffectValue());
        if (Player* player = GetCaster()->ToPlayer())
            AddPct(absorb, player->GetRatingBonusValue(CR_VERSATILITY_HEALING_DONE));

        if (AuraEffect* calmingCoalescence = GetCaster()->GetAuraEffect(SPELL_MONK_CALMING_COALESCENCE, EFFECT_0, GetCaster()->GetGUID()))
        {
            AddPct(absorb, calmingCoalescence->GetAmount());
            calmingCoalescence->GetBase()->Remove();
        }

        GetSpell()->SetSpellValue({ SPELLVALUE_BASE_POINT0, absorb });
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_monk_life_cocoon::CalculateAbsorb, EFFECT_2, SPELL_EFFECT_DUMMY);
    }
};

// 388548 - Mists of Life (attached to 116849 - Life Cocoon)
class spell_monk_mists_of_life : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_MISTS_OF_LIFE, SPELL_MONK_RENEWING_MIST, SPELL_MONK_ENVELOPING_MIST });
    }

    bool Load() override
    {
        return GetCaster()->HasAuraEffect(SPELL_MONK_MISTS_OF_LIFE, EFFECT_0);
    }

    void HandleEffectApply(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        CastSpellExtraArgs args;
        args.SetTriggerFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_POWER_COST | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_IGNORE_CAST_TIME | TRIGGERED_DONT_REPORT_CAST_ERROR);
        args.SetTriggeringSpell(GetSpell());

        caster->CastSpell(target, SPELL_MONK_RENEWING_MIST, args);
        caster->CastSpell(target, SPELL_MONK_ENVELOPING_MIST, args);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_mists_of_life::HandleEffectApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 392972 - Open Palm Strikes
class spell_monk_open_palm_strikes : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellEffect({ { SPELL_MONK_OPEN_PALM_STRIKES_TALENT, EFFECT_1} });
    }

    bool CheckProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*procInfo*/)
    {
        AuraEffect const* talent = GetTarget()->GetAuraEffect(SPELL_MONK_OPEN_PALM_STRIKES_TALENT, EFFECT_1);
        return talent && roll_chance_i(talent->GetAmount());
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_monk_open_palm_strikes::CheckProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 121817 - Power Strike
class spell_monk_power_strike_periodic : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_POWER_STRIKE_PROC });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_MONK_POWER_STRIKE_PROC, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_power_strike_periodic::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 129914 - Power Strike Proc
class spell_monk_power_strike_proc : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_POWER_STRIKE_ENERGIZE });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_MONK_POWER_STRIKE_ENERGIZE, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_power_strike_proc::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 115078 - Paralysis
class spell_monk_pressure_points : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_MONK_PRESSURE_POINTS })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } })
            && spellInfo->GetEffect(EFFECT_2).IsEffect(SPELL_EFFECT_DISPEL);
    }

    bool Load() override
    {
        return !GetCaster()->HasAura(SPELL_MONK_PRESSURE_POINTS);
    }

    static void PreventDispel(SpellScript const&, WorldObject*& target)
    {
        target = nullptr;
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_monk_pressure_points::PreventDispel, EFFECT_2, TARGET_UNIT_TARGET_ENEMY);
    }
};

// 115546 - Provoke
class spell_monk_provoke : public SpellScript
{
    static uint32 const BlackOxStatusEntry = 61146;

    bool Validate(SpellInfo const* spellInfo) override
    {
        if (!(spellInfo->GetExplicitTargetMask() & TARGET_FLAG_UNIT_MASK)) // ensure GetExplTargetUnit() will return something meaningful during CheckCast
            return false;
        return ValidateSpellInfo(
        {
            SPELL_MONK_PROVOKE_SINGLE_TARGET,
            SPELL_MONK_PROVOKE_AOE
        });
    }

    SpellCastResult CheckExplicitTarget()
    {
        if (GetExplTargetUnit()->GetEntry() != BlackOxStatusEntry)
        {
            SpellInfo const* singleTarget = sSpellMgr->AssertSpellInfo(SPELL_MONK_PROVOKE_SINGLE_TARGET, GetCastDifficulty());
            SpellCastResult singleTargetExplicitResult = singleTarget->CheckExplicitTarget(GetCaster(), GetExplTargetUnit());
            if (singleTargetExplicitResult != SPELL_CAST_OK)
                return singleTargetExplicitResult;
        }
        else if (GetExplTargetUnit()->GetOwnerGUID() != GetCaster()->GetGUID())
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        if (GetHitUnit()->GetEntry() != BlackOxStatusEntry)
            GetCaster()->CastSpell(GetHitUnit(), SPELL_MONK_PROVOKE_SINGLE_TARGET, true);
        else
            GetCaster()->CastSpell(GetHitUnit(), SPELL_MONK_PROVOKE_AOE, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_monk_provoke::CheckExplicitTarget);
        OnEffectHitTarget += SpellEffectFn(spell_monk_provoke::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 115151 - Renewing Mist
// Thin wrapper: applies the actual periodic HoT (119611, SPELL_MONK_RENEWING_MIST) on hit.
class spell_monk_renewing_mist : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_MONK_RENEWING_MIST, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_renewing_mist::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 119611 - Renewing Mist (periodic)
// Legion's signature Renewing Mist redesign: when the current target reaches full health, the
// HoT jumps to the most injured ally within 25 yards instead of just falling off, carrying its
// remaining duration with it.
class spell_monk_renewing_mist_periodic : public AuraScript
{
    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        if (target->GetHealthPct() < 100.0f)
            return;

        Aura* thisAura = target->GetAura(SPELL_MONK_RENEWING_MIST);
        if (!thisAura)
            return;

        int32 remainingDuration = thisAura->GetDuration();

        std::list<Unit*> targets;
        Trinity::AnyFriendlyUnitInObjectRangeCheck check(target, target, 25.0f);
        Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(target, targets, check);
        Cell::VisitAllObjects(target, searcher, 25.0f);

        if (targets.empty())
            return;

        targets.sort(Trinity::Predicates::HealthPctOrderPred());

        Unit* newTarget = targets.front();
        caster->CastSpell(newTarget, SPELL_MONK_RENEWING_MIST, true);

        if (Aura* newAura = newTarget->GetAura(SPELL_MONK_RENEWING_MIST))
        {
            newAura->SetDuration(remainingDuration);
            target->RemoveAura(SPELL_MONK_RENEWING_MIST);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_renewing_mist_periodic::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
    }
};

// 107428 - Rising Sun Kick
// NOTE: previously gated its entire HandleOnHit behind Load() requiring Combat Conditioning,
// which was correct while Mortal Wounds application was the only thing this class did. Adding
// the Rising Thunder (Mistweaver) interaction below meant that gate needed to move: Rising
// Thunder and Combat Conditioning are unrelated talents (different specs), so gating the whole
// script on Combat Conditioning would have silently prevented Rising Thunder's reset from ever
// running. Removed Load() and moved the Combat Conditioning check inline instead, matching the
// internal-HasAura-check pattern already used by sibling classes in this file
// (spell_monk_pressure_points, etc.) - each condition is now independently checked.
class spell_monk_rising_sun_kick : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_COMBAT_CONDITIONING, SPELL_MONK_MORTAL_WOUNDS, SPELL_MONK_RISING_THUNDER, SPELL_MONK_THUNDER_FOCUS_TEA });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (caster->HasAura(SPELL_MONK_COMBAT_CONDITIONING))
        {
            caster->CastSpell(target, SPELL_MONK_MORTAL_WOUNDS, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                .TriggeringSpell = GetSpell()
            });
        }

        if (caster->HasAura(SPELL_MONK_RISING_THUNDER))
            caster->GetSpellHistory()->ResetCooldown(SPELL_MONK_THUNDER_FOCUS_TEA, true);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_monk_rising_sun_kick::HandleOnHit, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

// 109132 - Roll
class spell_monk_roll : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_ROLL_BACKWARD, SPELL_MONK_ROLL_FORWARD, SPELL_MONK_NO_FEATHER_FALL });
    }

    SpellCastResult CheckCast()
    {
        if (GetCaster()->HasUnitState(UNIT_STATE_ROOT))
            return SPELL_FAILED_ROOTED;
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), GetCaster()->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD) ? SPELL_MONK_ROLL_BACKWARD : SPELL_MONK_ROLL_FORWARD,
            TRIGGERED_IGNORE_CAST_IN_PROGRESS);
        GetCaster()->CastSpell(GetCaster(), SPELL_MONK_NO_FEATHER_FALL, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_monk_roll::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_monk_roll::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 107427 - Roll
// 109131 - Roll (backward)
class spell_monk_roll_aura : public AuraScript
{
    void CalcMovementAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount += 100;
    }

    void CalcImmunityAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount -= 100;
    }

    void ChangeRunBackSpeed(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->SetSpeed(MOVE_RUN_BACK, GetTarget()->GetSpeed(MOVE_RUN));
    }

    void RestoreRunBackSpeed(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->UpdateSpeed(MOVE_RUN_BACK);
    }

    void Register() override
    {
        // Values need manual correction
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_roll_aura::CalcMovementAmount, EFFECT_0, SPELL_AURA_MOD_SPEED_NO_CONTROL);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_roll_aura::CalcMovementAmount, EFFECT_2, SPELL_AURA_MOD_MINIMUM_SPEED);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_roll_aura::CalcImmunityAmount, EFFECT_5, SPELL_AURA_MECHANIC_IMMUNITY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_roll_aura::CalcImmunityAmount, EFFECT_6, SPELL_AURA_MECHANIC_IMMUNITY);

        // This is a special aura that sets backward run speed equal to forward speed
        AfterEffectApply += AuraEffectApplyFn(spell_monk_roll_aura::ChangeRunBackSpeed, EFFECT_4, SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectApplyFn(spell_monk_roll_aura::RestoreRunBackSpeed, EFFECT_4, SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED, AURA_EFFECT_HANDLE_REAL);
    }
};

// 389579 - Save Them All
class spell_monk_save_them_all : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_MONK_SAVE_THEM_ALL_HEAL_BONUS })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } });
    }

    bool CheckProc(ProcEventInfo const& eventInfo) const
    {
        return eventInfo.GetActionTarget()->HealthBelowPct(GetEffectInfo(EFFECT_2).CalcValue(eventInfo.GetActor()));
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/) const
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_MONK_SAVE_THEM_ALL_HEAL_BONUS, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_monk_save_them_all::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_monk_save_them_all::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 198898 - Song of Chi-Ji
struct at_monk_song_of_chi_ji : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnInitialize() override
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(at->GetSpellId(), DIFFICULTY_NONE);
        if (!spellInfo)
            return;

        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        Position destPos = at->GetFirstCollisionPosition(spellInfo->GetMaxRange(false, caster), 0.0f);
        PathGenerator path(at);

        path.CalculatePath(destPos.GetPositionX(), destPos.GetPositionY(), destPos.GetPositionZ(), false);

        at->InitSplines(path.GetPath());
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
            if (caster->IsValidAttackTarget(unit))
                caster->CastSpell(unit, SPELL_MONK_SONG_OF_CHI_JI_STUN, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
    }
};

// Gift of the Ox healing sphere - spawned by 124503/124506
// Heals the caster on pickup (after a short delay to avoid instant self-pickup) and clears the
// spawn cooldown marker when the sphere expires/is removed.
struct at_monk_gift_of_the_ox_sphere : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    uint32 pickupDelay = 1000;

    void OnUpdate(uint32 diff) override
    {
        pickupDelay = (pickupDelay >= diff) ? (pickupDelay - diff) : 0;
    }

    void OnUnitEnter(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster || unit != caster || pickupDelay)
            return;

        caster->CastSpell(caster, SPELL_MONK_GIFT_OF_THE_OX_HEAL, true);
        at->Remove();
    }

    void OnRemove() override
    {
        if (Unit* caster = at->GetCaster())
            if (caster->HasAura(SPELL_MONK_HEALING_SPHERE_COOLDOWN))
                caster->RemoveAura(SPELL_MONK_HEALING_SPHERE_COOLDOWN);
    }
};

// 124502 - Gift of the Ox
// Chance to spawn a healing sphere when taking damage, scaling with damage taken relative to
// max health and increasing as the Monk's own health drops.
// NOTE: DestinyCore's reference implements this as a global PlayerScript::OnTakeDamage hook -
// ArgusCore's PlayerScript has no OnTakeDamage hook at all (a missing engine capability, the
// same category of gap as Execute's missing OnTakePower earlier this session). Rather than
// needing a new engine hook, implemented this as a self-contained proc-driven AuraScript on
// the talent aura itself instead, matching the idiom used throughout this file/session
// (Defensive Stance, Second Wind, etc.) - this assumes 124502's own DB2 proc data is scoped to
// "on taking damage" (the entire point of this talent), which wasn't independently verified
// but is a reasonable, low-risk assumption given the ability's name and purpose.
class spell_monk_gift_of_the_ox_aura : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo const* dmgInfo = eventInfo.GetDamageInfo();
        Unit* victim = GetTarget();
        if (!dmgInfo || !dmgInfo->GetDamage() || !victim || !victim->GetMaxHealth())
            return false;

        float dmgRatio = float(dmgInfo->GetDamage()) / float(victim->GetMaxHealth());
        float healthScaling = 3.f - 2.f * (victim->GetHealthPct() / 100.f);
        return roll_chance_f(0.75f * dmgRatio * healthScaling * 100.f);
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        Unit* victim = GetTarget();
        if (!victim || victim->HasAura(SPELL_MONK_HEALING_SPHERE_COOLDOWN))
            return;

        static constexpr uint32 SpellsToCast[] = { SPELL_MONK_GIFT_OF_THE_OX_AT_RIGHT, SPELL_MONK_GIFT_OF_THE_OX_AT_LEFT };
        victim->CastSpell(victim, SPELL_MONK_HEALING_SPHERE_COOLDOWN, true);
        victim->CastSpell(victim, SpellsToCast[urand(0, 1)], true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_monk_gift_of_the_ox_aura::CheckProc);
        OnProc += AuraProcFn(spell_monk_gift_of_the_ox_aura::HandleProc);
    }
};

// Utility for stagger scripts
Aura* FindExistingStaggerEffect(Unit* unit)
{
    if (Aura* auraLight = unit->GetAura(SPELL_MONK_STAGGER_LIGHT))
        return auraLight;

    if (Aura* auraModerate = unit->GetAura(SPELL_MONK_STAGGER_MODERATE))
        return auraModerate;

    if (Aura* auraHeavy = unit->GetAura(SPELL_MONK_STAGGER_HEAVY))
        return auraHeavy;

    return nullptr;
}

static constexpr SpellEffIndex AuraStaggerEffectTick = EFFECT_0;
static constexpr SpellEffIndex AuraStaggerEffectTotal = EFFECT_1;

uint32 GetStaggerSpellId(Unit* unit, float amount)
{
    const float StaggerHeavy = 0.6f;
    const float StaggerModerate = 0.3f;

    float staggerPct = amount / float(unit->GetMaxHealth());
    return (staggerPct >= StaggerHeavy) ? SPELL_MONK_STAGGER_HEAVY :
        (staggerPct >= StaggerModerate) ? SPELL_MONK_STAGGER_MODERATE :
        SPELL_MONK_STAGGER_LIGHT;
}

void AddNewStagger(Unit* unit, uint32 staggerSpellId, float staggerAmount)
{
    // We only set the total stagger amount. The amount per tick will be set by the stagger spell script
    unit->CastSpell(unit, staggerSpellId, CastSpellExtraArgs(SPELLVALUE_BASE_POINT1, staggerAmount).SetTriggerFlags(TRIGGERED_FULL_MASK));
}

// Shared by spell_monk_stagger (redirecting the Monk's own taken damage) and spell_monk_guard
// (redirecting a protected ally's taken damage into the Monk's own Stagger pool).
void AddAndRefreshStagger(Unit* target, float amount)
{
    if (Aura* auraStagger = FindExistingStaggerEffect(target))
    {
        AuraEffect* effStaggerRemaining = auraStagger->GetEffect(AuraStaggerEffectTotal);
        if (!effStaggerRemaining)
            return;

        float newAmount = effStaggerRemaining->GetAmount() + amount;
        uint32 spellId = GetStaggerSpellId(target, newAmount);
        if (spellId == effStaggerRemaining->GetSpellInfo()->Id)
        {
            auraStagger->RefreshDuration();
            effStaggerRemaining->ChangeAmount(newAmount, false, true /* reapply */);
        }
        else
        {
            // amount changed the stagger type so we need to change the stagger amount (e.g. from medium to light)
            target->RemoveAura(auraStagger);
            AddNewStagger(target, spellId, newAmount);
        }
    }
    else
        AddNewStagger(target, GetStaggerSpellId(target, amount), amount);
}

// 115175 - Soothing Mist
// Mistweaver's healing channel: applies a visual on the target while channeling, has a 25%
// chance per tick to generate a Chi, and cleans up the visual when the channel ends.
// NOTE: DestinyCore's reference also crosses over into the Jade Serpent Statue mechanic here
// (redirecting the channel through a summoned totem, creature entry 60849) - not ported, since
// that requires a creature_template row this repo's SQL has no record of (the same kind of
// unverified NPC data dependency that blocked Ravager earlier this session). The core
// player-facing healing channel and Chi generation are unaffected by omitting it.
class spell_monk_soothing_mist : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(target, SPELL_MONK_SOOTHING_MIST_VISUAL, true);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (roll_chance_i(25))
            caster->CastSpell(caster, SPELL_MONK_SOOTHING_MIST_ENERGIZE, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
            target->RemoveAura(SPELL_MONK_SOOTHING_MIST_VISUAL);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_monk_soothing_mist::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_soothing_mist::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_monk_soothing_mist::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 193884 - Soothing Mist (redirect)
// Lets other instant Mistweaver spells cast while channeling Soothing Mist without breaking
// the channel, by re-casting Soothing Mist on the same target when this driver aura procs.
class spell_monk_soothing_mist_aura : public AuraScript
{
    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (!caster || !eventInfo.GetProcTarget())
            return;

        caster->CastSpell(eventInfo.GetActionTarget(), SPELL_MONK_SOOTHING_MIST, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_soothing_mist_aura::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 119582 - Purifying Brew
// Halves whatever Stagger DoT is currently active (Light/Moderate/Heavy), using the existing
// FindExistingStaggerEffect helper shared with the other Stagger scripts in this file.
class spell_monk_purifying_brew : public SpellScript
{
    void HandleOnHit()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        if (Aura* staggerAura = FindExistingStaggerEffect(caster))
            if (AuraEffect* total = staggerAura->GetEffect(AuraStaggerEffectTotal))
                total->ChangeAmount(total->GetAmount() / 2);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_monk_purifying_brew::HandleOnHit);
    }
};

// 202162 - Guard
// NOTE: DestinyCore's reference implements this as a flat self-absorb shield
// (AttackPower * 18) - that matches Guard's pre-Legion (Mists of Pandaria) mechanic, not
// 7.3.5's. Guard was redesigned into a PvP honor talent in patch 7.1.5 (well before 7.3.5):
// it no longer provides a self-absorb at all, instead redirecting 30% of a protected nearby
// ally's incoming damage into the Monk's own Stagger pool. DestinyCore's own bound spell id
// (202162) matches this later PvP-talent version's real id, not the old MoP ability's id
// (115295) - meaning DestinyCore's code and its own binding actually disagree with each
// other. Found the correct implementation for this exact id in LegionCore instead (explicitly
// labeled "Guard (PvP talent) - 202162"), which redirects damage into Stagger via the same
// technique spell_monk_stagger already uses for the Monk's own damage - reused that shared
// AddAndRefreshStagger helper (extracted from spell_monk_stagger for this purpose) rather
// than duplicating the logic.
class spell_monk_guard : public AuraScript
{
    int32 _pctAbsorb = 0;

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        amount = -1;
        canBeRecalculated = false;
        _pctAbsorb = GetEffectInfo(EFFECT_1).CalcValue();
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* caster = GetCaster();
        if (!caster)
        {
            absorbAmount = 0;
            return;
        }

        absorbAmount = uint32(CalculatePct(dmgInfo.GetDamage(), _pctAbsorb));
        AddAndRefreshStagger(caster, float(absorbAmount));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_guard::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_guard::Absorb, EFFECT_0);
    }
};

// 115069 - Stagger
class spell_monk_stagger : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_STAGGER_LIGHT, SPELL_MONK_STAGGER_MODERATE, SPELL_MONK_STAGGER_HEAVY });
    }

    void AbsorbNormal(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
    {
        Absorb(dmgInfo, 1.0f);
    }

    void AbsorbMagic(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
    {
        AuraEffect const* effect = GetEffect(EFFECT_4);
        if (!effect)
            return;

        Absorb(dmgInfo, float(effect->GetAmount()) / 100.0f);
    }

    void Absorb(DamageInfo& dmgInfo, float multiplier)
    {
        // Prevent default action (which would remove the aura)
        PreventDefaultAction();

        // make sure damage doesn't come from stagger damage spell SPELL_MONK_STAGGER_DAMAGE_AURA
        if (SpellInfo const* dmgSpellInfo = dmgInfo.GetSpellInfo())
            if (dmgSpellInfo->Id == SPELL_MONK_STAGGER_DAMAGE_AURA)
                return;

        AuraEffect const* effect = GetEffect(AuraStaggerEffectTick);
        if (!effect)
            return;

        // Absorb X percentage of the damage
        float absorbAmount = float(dmgInfo.GetDamage()) * multiplier;
        if (absorbAmount > 0)
        {
            dmgInfo.AbsorbDamage(absorbAmount);

            // Cast stagger and make it tick on each tick
            AddAndRefreshStagger(GetTarget(), absorbAmount);
        }
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_stagger::AbsorbNormal, EFFECT_1);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_stagger::AbsorbMagic, EFFECT_2);
    }
};

// 124255 - Stagger - SPELL_MONK_STAGGER_DAMAGE_AURA
class spell_monk_stagger_damage_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_STAGGER_LIGHT, SPELL_MONK_STAGGER_MODERATE, SPELL_MONK_STAGGER_HEAVY });
    }

    void OnPeriodicDamage(AuraEffect const* aurEff)
    {
        // Update our light/medium/heavy stagger with the correct stagger amount left
        if (Aura* auraStagger = FindExistingStaggerEffect(GetTarget()))
        {
            if (AuraEffect* auraEff = auraStagger->GetEffect(AuraStaggerEffectTotal))
            {
                float total = float(auraEff->GetAmount());
                float tickDamage = float(aurEff->GetAmount());
                auraEff->ChangeAmount(total - tickDamage);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_stagger_damage_aura::OnPeriodicDamage, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 124273, 124274, 124275 - Light/Moderate/Heavy Stagger - SPELL_MONK_STAGGER_LIGHT / SPELL_MONK_STAGGER_MODERATE / SPELL_MONK_STAGGER_HEAVY
class spell_monk_stagger_debuff_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_STAGGER_DAMAGE_AURA })
            && ValidateSpellEffect({ { SPELL_MONK_STAGGER_DAMAGE_AURA, EFFECT_0 } });
    }

    bool Load() override
    {
        _period = float(sSpellMgr->AssertSpellInfo(SPELL_MONK_STAGGER_DAMAGE_AURA, GetCastDifficulty())->GetEffect(EFFECT_0).ApplyAuraPeriod);
        return true;
    }

    void OnReapply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        // Calculate damage per tick
        float total = float(aurEff->GetAmount());
        float perTick = total * _period / float(GetDuration()); // should be same as GetMaxDuration() TODO: verify

        // Set amount on effect for tooltip
        AuraEffect* effInfo = GetAura()->GetEffect(AuraStaggerEffectTick);
        if (effInfo)
            effInfo->ChangeAmount(perTick);

        // Set amount on damage aura (or cast it if needed)
        CastOrChangeTickDamage(perTick);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        if (mode != AURA_EFFECT_HANDLE_REAL)
            return;

        // Remove damage aura
        GetTarget()->RemoveAura(SPELL_MONK_STAGGER_DAMAGE_AURA);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectRemoveFn(spell_monk_stagger_debuff_aura::OnReapply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        AfterEffectRemove += AuraEffectRemoveFn(spell_monk_stagger_debuff_aura::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }

private:
    float _period = 0.0f;

    void CastOrChangeTickDamage(float tickDamage)
    {
        Unit* unit = GetTarget();
        Aura* auraDamage = unit->GetAura(SPELL_MONK_STAGGER_DAMAGE_AURA);
        if (!auraDamage)
        {
            unit->CastSpell(unit, SPELL_MONK_STAGGER_DAMAGE_AURA, true);
            auraDamage = unit->GetAura(SPELL_MONK_STAGGER_DAMAGE_AURA);
        }

        if (auraDamage)
            if (AuraEffect* eff = auraDamage->GetEffect(AuraStaggerEffectTick))
                eff->ChangeAmount(tickDamage);
    }
};

// 116841 - Tiger's Lust
class spell_monk_tigers_lust : public SpellScript
{
    void HandleRemoveImpairingAuras(SpellEffIndex /*effIndex*/)
    {
        GetHitUnit()->RemoveMovementImpairingAuras(true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_tigers_lust::HandleRemoveImpairingAuras, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 115080 - Touch of Death
// Deals damage equal to a percentage of the caster's (not the target's) max health - halved
// against player targets, matching Legion's well-known Touch of Death design (execute-style
// instant kill vs. NPCs, balanced down for PvP). Self-referential: each tick recomputes the
// damage and re-casts this same spell id on the original target with the new value as a
// custom base point, rather than driving a separate damage sub-spell.
// NOTE: DestinyCore's own CalculateAmount has a Mastery: Combo Strikes integration commented
// out (with an author TODO note, "need to merge, already did" — never finished). Not ported,
// since that would require the broader Combo Strikes system this session deliberately deferred
// (see the Mastery: Combo Strikes entry in the backlog) - matches the file's own disabled state.
class spell_monk_touch_of_death : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_TOUCH_OF_DEATH });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        Unit* caster = GetCaster();
        Unit* owner = GetUnitOwner();
        if (!caster || !owner)
            return;

        canBeRecalculated = false;

        int32 pct = GetEffectInfo(EFFECT_1).CalcValue(caster);
        amount = int32(caster->CountPctFromMaxHealth(owner->GetTypeId() == TYPEID_PLAYER ? (pct / 2) : pct));
    }

    void OnTick(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        Unit* owner = GetUnitOwner();
        if (!caster || !owner || !caster->IsAlive())
            return;

        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.AddSpellBP0(aurEff->GetAmount());
        caster->CastSpell(owner, SPELL_MONK_TOUCH_OF_DEATH, args);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_touch_of_death::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_touch_of_death::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 122470 - Touch of Karma
// Absorbs damage up to the caster's max health, redirecting 1/16th of total absorbed damage
// back to the attacker as periodic damage - tracked cumulatively across the aura's duration
// (each new hit recalculates the redirect from the running total, not just the latest hit).
class spell_monk_touch_of_karma : public AuraScript
{
    uint32 _totalAbsorbAmount = 0;

    bool Load() override
    {
        _totalAbsorbAmount = 0;
        return true;
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
            amount = int32(caster->GetMaxHealth());
    }

    void OnAbsorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
    {
        Unit* caster = dmgInfo.GetVictim();
        Unit* attacker = dmgInfo.GetAttacker();
        if (!caster || !attacker)
            return;

        _totalAbsorbAmount += dmgInfo.GetDamage();

        if (attacker->HasAura(aurEff->GetSpellInfo()->Id, caster->GetGUID()))
        {
            CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
            args.AddSpellBP0(int32(_totalAbsorbAmount / 16));
            caster->CastSpell(attacker, SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE, args);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_touch_of_karma::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_touch_of_karma::OnAbsorb, EFFECT_1);
    }
};

void AddSC_monk_spell_scripts()
{
    RegisterSpellScript(spell_monk_blackout_kick);
    RegisterSpellScript(spell_monk_burst_of_life);
    RegisterSpellScript(spell_monk_burst_of_life_heal);
    RegisterSpellScript(spell_monk_crackling_jade_lightning);
    RegisterSpellScript(spell_monk_crackling_jade_lightning_knockback_proc_aura);
    RegisterSpellScript(spell_monk_dampen_harm);
    RegisterSpellScript(spell_monk_energizing_brew);
    RegisterSpellScript(spell_monk_enveloping_mist);
    RegisterSpellScript(spell_monk_fists_of_fury);
    RegisterSpellScript(spell_monk_fists_of_fury_damage);
    RegisterSpellScript(spell_monk_fists_of_fury_visual_filter);
    RegisterSpellScript(spell_monk_fists_of_fury_visual);
    RegisterSpellScript(spell_monk_fortifying_brew);
    RegisterSpellScript(spell_monk_gift_of_the_ox_aura);
    RegisterSpellScript(spell_monk_jade_walk);
    RegisterSpellScript(spell_monk_life_cocoon);
    RegisterSpellScript(spell_monk_mists_of_life);
    RegisterSpellScript(spell_monk_open_palm_strikes);
    RegisterSpellScript(spell_monk_power_strike_periodic);
    RegisterSpellScript(spell_monk_power_strike_proc);
    RegisterSpellScript(spell_monk_pressure_points);
    RegisterSpellScript(spell_monk_provoke);
    RegisterSpellScript(spell_monk_purifying_brew);
    RegisterSpellScript(spell_monk_guard);
    RegisterSpellScript(spell_monk_renewing_mist);
    RegisterSpellScript(spell_monk_renewing_mist_periodic);
    RegisterSpellScript(spell_monk_rising_sun_kick);
    RegisterSpellScript(spell_monk_soothing_mist);
    RegisterSpellScript(spell_monk_soothing_mist_aura);
    RegisterSpellScript(spell_monk_roll);
    RegisterSpellScript(spell_monk_roll_aura);
    RegisterSpellScript(spell_monk_save_them_all);
    RegisterAreaTriggerAI(at_monk_song_of_chi_ji);
    RegisterAreaTriggerAI(at_monk_gift_of_the_ox_sphere);
    RegisterSpellScript(spell_monk_stagger);
    RegisterSpellScript(spell_monk_stagger_damage_aura);
    RegisterSpellScript(spell_monk_stagger_debuff_aura);
    RegisterSpellScript(spell_monk_tigers_lust);
    RegisterSpellScript(spell_monk_touch_of_death);
    RegisterSpellScript(spell_monk_touch_of_karma);
}
