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
#include "DB2Stores.h"
#include "GridNotifiers.h"
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
    SPELL_MONK_FISTS_OF_FURY_DAMAGE                     = 117418,
    SPELL_MONK_FISTS_OF_FURY_VISUAL                     = 123154,
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
            AddAndRefreshStagger(absorbAmount);
        }
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_stagger::AbsorbNormal, EFFECT_1);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_stagger::AbsorbMagic, EFFECT_2);
    }

private:
    void AddAndRefreshStagger(float amount)
    {
        Unit* target = GetTarget();
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
                GetTarget()->RemoveAura(auraStagger);
                AddNewStagger(target, spellId, newAmount);
            }
        }
        else
            AddNewStagger(target, GetStaggerSpellId(target, amount), amount);
    }

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

void AddSC_monk_spell_scripts()
{
    RegisterSpellScript(spell_monk_blackout_kick);
    RegisterSpellScript(spell_monk_burst_of_life);
    RegisterSpellScript(spell_monk_burst_of_life_heal);
    RegisterSpellScript(spell_monk_crackling_jade_lightning);
    RegisterSpellScript(spell_monk_crackling_jade_lightning_knockback_proc_aura);
    RegisterSpellScript(spell_monk_fists_of_fury);
    RegisterSpellScript(spell_monk_fists_of_fury_damage);
    RegisterSpellScript(spell_monk_fists_of_fury_visual_filter);
    RegisterSpellScript(spell_monk_fists_of_fury_visual);
    RegisterSpellScript(spell_monk_jade_walk);
    RegisterSpellScript(spell_monk_life_cocoon);
    RegisterSpellScript(spell_monk_mists_of_life);
    RegisterSpellScript(spell_monk_open_palm_strikes);
    RegisterSpellScript(spell_monk_power_strike_periodic);
    RegisterSpellScript(spell_monk_power_strike_proc);
    RegisterSpellScript(spell_monk_pressure_points);
    RegisterSpellScript(spell_monk_provoke);
    RegisterSpellScript(spell_monk_rising_sun_kick);
    RegisterSpellScript(spell_monk_roll);
    RegisterSpellScript(spell_monk_roll_aura);
    RegisterSpellScript(spell_monk_save_them_all);
    RegisterAreaTriggerAI(at_monk_song_of_chi_ji);
    RegisterSpellScript(spell_monk_stagger);
    RegisterSpellScript(spell_monk_stagger_damage_aura);
    RegisterSpellScript(spell_monk_stagger_debuff_aura);
    RegisterSpellScript(spell_monk_tigers_lust);
}
