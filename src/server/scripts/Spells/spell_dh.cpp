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
 * Scripts for spells with SPELLFAMILY_DEMONHUNTER and SPELLFAMILY_GENERIC spells used by demon hunter players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dh_".
 */

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "Containers.h"
#include "DB2Stores.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PathGenerator.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "Unit.h"

enum DemonHunterSpells
{
    AREATRIGGER_DH_SHATTERED_SOULS_HAVOC           = 8352,
    AREATRIGGER_DH_SHATTERED_SOULS_HAVOC_DEMON     = 11231,
    AREATRIGGER_DH_SHATTERED_SOULS_VENGEANCE       = 11266,
    AREATRIGGER_DH_SHATTERED_SOULS_VENGEANCE_DEMON = 10693,
    AREATRIGGER_DH_SOUL_FRAGMENT_HAVOC             = 12929,
    AREATRIGGER_DH_SOUL_FRAGMENT_VENGEANCE         = 10665,

    SPELL_DH_ABYSSAL_STRIKE                        = 207550,
    SPELL_DH_ANNIHILATION                          = 201427,
    SPELL_DH_ANNIHILATION_MH                       = 227518,
    SPELL_DH_ANNIHILATION_OH                       = 201428,
    SPELL_DH_AWAKEN_THE_DEMON_WITHIN_CD            = 207128,
    SPELL_DH_BLOODLET_DOT                          = 207690,
    SPELL_DH_BLUR                                  = 212800,
    SPELL_DH_BLUR_TRIGGER                          = 198589,
    SPELL_DH_BURNING_ALIVE                         = 207739,
    SPELL_DH_BURNING_ALIVE_TARGET_SELECTOR         = 207760,
    SPELL_DH_CHAOS_NOVA                            = 179057,
    SPELL_DH_CHAOS_STRIKE                          = 162794,
    SPELL_DH_CHAOS_STRIKE_ENERGIZE                 = 193840,
    SPELL_DH_CHAOS_STRIKE_MH                       = 222031,
    SPELL_DH_CHAOS_STRIKE_OH                       = 199547,
    SPELL_DH_CHARRED_WARBLADES_HEAL                = 213011,
    SPELL_DH_CONSUME_SOUL_HAVOC                    = 228542,
    SPELL_DH_CONSUME_SOUL_HAVOC_DEMON              = 228556,
    SPELL_DH_CONSUME_SOUL_HAVOC_SHATTERED          = 228540,
    SPELL_DH_CONSUME_SOUL_HEAL                     = 203794,
    SPELL_DH_CONSUME_SOUL_VENGEANCE                = 208014,
    SPELL_DH_CONSUME_SOUL_VENGEANCE_DEMON          = 210050,
    SPELL_DH_CONSUME_SOUL_VENGEANCE_SHATTERED      = 210047,
    SPELL_DH_DARKNESS_ABSORB                       = 209426,
    SPELL_DH_DEMON_BLADES                          = 203555,
    SPELL_DH_DEMON_BLADES_DMG                      = 203796,
    SPELL_DH_DEMON_SPIKES                          = 203819,
    SPELL_DH_DEMON_SPIKES_TRIGGER                  = 203720,
    SPELL_DH_DEMONIC                               = 213410,
    SPELL_DH_DEMONIC_APPETITE                      = 206478,
    SPELL_DH_DEMONIC_ORIGINS                       = 235893,
    SPELL_DH_DEMONIC_ORIGINS_BUFF                  = 235894,
    SPELL_DH_DEMONIC_TRAMPLE_DMG                   = 208645,
    SPELL_DH_DEMONIC_TRAMPLE_STUN                  = 213491,
    SPELL_DH_DEMONS_BITE                           = 162243,
    SPELL_DH_DEMON_REBORN                          = 193897,
    SPELL_DH_EYE_BEAM                              = 198013,
    SPELL_DH_EYE_BEAM_DAMAGE                       = 198030,
    SPELL_DH_EYE_OF_LEOTHERAS_DMG                  = 206650,
    SPELL_DH_FEAST_OF_SOULS                        = 207697,
    SPELL_DH_FEAST_OF_SOULS_PERIODIC_HEAL          = 207693,
    SPELL_DH_FEED_THE_DEMON                        = 218612,
    SPELL_DH_FEL_BARRAGE                           = 211053,
    SPELL_DH_FEL_BARRAGE_DMG                       = 211052,
    SPELL_DH_FEL_BARRAGE_PROC                      = 222703,
    SPELL_DH_FEL_DEVASTATION                       = 212084,
    SPELL_DH_FEL_DEVASTATION_DMG                   = 212105,
    SPELL_DH_FEL_DEVASTATION_HEAL                  = 212106,
    SPELL_DH_FEL_ERUPTION                          = 211881,
    SPELL_DH_FEL_ERUPTION_DAMAGE                   = 225102,
    SPELL_DH_FEL_RUSH                              = 195072,
    SPELL_DH_FEL_RUSH_DMG                          = 192611,
    SPELL_DH_FEL_RUSH_GROUND                       = 197922,
    SPELL_DH_FEL_RUSH_WATER_AIR                    = 197923,
    SPELL_DH_FELBLADE                              = 232893,
    SPELL_DH_FELBLADE_CHARGE                       = 213241,
    SPELL_DH_FELBLADE_COOLDOWN_RESET_PROC_HAVOC    = 236167,
    SPELL_DH_FELBLADE_COOLDOWN_RESET_PROC_VENGEANCE= 203557,
    SPELL_DH_FELBLADE_COOLDOWN_RESET_PROC_VISUAL   = 204497,
    SPELL_DH_FELBLADE_DAMAGE                       = 213243,
    SPELL_DH_FRACTURE                              = 209795,
    SPELL_DH_FIERY_BRAND                           = 204021,
    SPELL_DH_FIERY_BRAND_DEBUFF_RANK_1             = 207744,
    SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2             = 207771,
    SPELL_DH_FIRST_BLOOD                           = 206416,
    SPELL_DH_FLAME_CRASH                           = 227322,
    SPELL_DH_FRAILTY                               = 224509,
    SPELL_DH_GLIDE                                 = 131347,
    SPELL_DH_GLIDE_DURATION                        = 197154,
    SPELL_DH_GLIDE_KNOCKBACK                       = 196353,
    SPELL_DH_GLUTTONY                              = 227327,
    SPELL_DH_GLUTTONY_BUFF                         = 227330,
    SPELL_DH_HAVOC_MASTERY                         = 185164,
    SPELL_DH_ILLIDANS_GRASP                        = 205630,
    SPELL_DH_ILLIDANS_GRASP_DAMAGE                 = 208618,
    SPELL_DH_ILLIDANS_GRASP_JUMP_DEST              = 208175,
    // real id (258920 is confirmed absent from this build - no Spell record at all); 178740 is
    // Havoc's, 219830 is Vengeance's, both structurally identical and both bound to this script.
    // Cross-referenced independently via SPELL_DH_IMMOLATION_AURA_VISUAL's own tooltip
    // ("$@spelldesc178740").
    SPELL_DH_IMMOLATION_AURA                       = 178740,
    SPELL_DH_IMMOLATION_AURA_VISUAL                = 201122,
    SPELL_DH_INFERNAL_STRIKE_CAST                  = 189110,
    SPELL_DH_INFERNAL_STRIKE_IMPACT_DAMAGE         = 189112,
    SPELL_DH_INFERNAL_STRIKE_JUMP                  = 189111,
    SPELL_DH_JAGGED_SPIKES                         = 205627,
    SPELL_DH_JAGGED_SPIKES_DMG                     = 208790,
    SPELL_DH_JAGGED_SPIKES_PROC                    = 208796,
    SPELL_DH_MANA_RIFT_DMG_POWER_BURN              = 235904,
    SPELL_DH_METAMORPHOSIS                         = 191428,
    SPELL_DH_METAMORPHOSIS_DUMMY                   = 191427,
    SPELL_DH_METAMORPHOSIS_IMMUNITY                = 201453,
    SPELL_DH_METAMORPHOSIS_IMPACT_DAMAGE           = 200166,
    SPELL_DH_METAMORPHOSIS_TRANSFORM               = 162264,
    SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM     = 187827,
    SPELL_DH_MOMENTUM                              = 208628,
    SPELL_DH_MOMENTUM_TALENT                       = 206476,
    SPELL_DH_NEMESIS_ABERRATIONS                   = 208607,
    SPELL_DH_NEMESIS_BEASTS                        = 208608,
    SPELL_DH_NEMESIS_CRITTERS                      = 208609,
    SPELL_DH_NEMESIS_DEMONS                        = 208579,
    SPELL_DH_NEMESIS_DRAGONKIN                     = 208610,
    SPELL_DH_NEMESIS_ELEMENTALS                    = 208611,
    SPELL_DH_NEMESIS_GIANTS                        = 208612,
    SPELL_DH_NEMESIS_HUMANOIDS                     = 208605,
    SPELL_DH_NEMESIS_MECHANICALS                   = 208613,
    SPELL_DH_NEMESIS_UNDEAD                        = 208614,
    SPELL_DH_NETHER_BOND                           = 207810,
    SPELL_DH_NETHER_BOND_DAMAGE                    = 207812,
    SPELL_DH_NETHER_BOND_PERIODIC                  = 207811,
    SPELL_DH_PREPARED                              = 203551,
    SPELL_DH_RAIN_FROM_ABOVE                       = 206803,
    SPELL_DH_RAIN_OF_CHAOS                         = 205628,
    SPELL_DH_RAIN_OF_CHAOS_IMPACT                  = 232538,
    SPELL_DH_RAZOR_SPIKES                          = 210003,
    SPELL_DH_SEVER                                 = 235964,
    SPELL_DH_SHATTER_SOUL                          = 209980,
    SPELL_DH_SHATTER_SOUL_1                        = 209981,
    SPELL_DH_SHATTER_SOUL_2                        = 210038,
    SPELL_DH_SHATTERED_SOUL                        = 226258,
    SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_1 = 228533,
    SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_2 = 237867,
    SPELL_DH_SHEAR                                 = 203782,
    SPELL_DH_SIGIL_OF_CHAINS_AREA_SELECTOR         = 204834,
    SPELL_DH_SIGIL_OF_CHAINS_GRIP                  = 208674,
    SPELL_DH_SIGIL_OF_CHAINS_JUMP                  = 208674,
    SPELL_DH_SIGIL_OF_CHAINS_SLOW                  = 204843,
    SPELL_DH_SIGIL_OF_CHAINS_SNARE                 = 204843,
    SPELL_DH_SIGIL_OF_CHAINS_TARGET_SELECT         = 204834,
    SPELL_DH_SIGIL_OF_CHAINS_VISUAL                = 208673,
    SPELL_DH_SIGIL_OF_FLAME                        = 204596,
    SPELL_DH_SIGIL_OF_FLAME_AOE                    = 204598,
    SPELL_DH_SIGIL_OF_FLAME_FLAME_CRASH            = 228973,
    SPELL_DH_SIGIL_OF_FLAME_VISUAL                 = 208710,
    SPELL_DH_SIGIL_OF_MISERY                       = 207685,
    SPELL_DH_SIGIL_OF_MISERY_AOE                   = 207685,
    SPELL_DH_SIGIL_OF_SILENCE                      = 204490,
    SPELL_DH_SIGIL_OF_SILENCE_AOE                  = 204490,
    SPELL_DH_SOUL_BARRIER                          = 227225,
    SPELL_DH_SOUL_CLEAVE                           = 228477,
    SPELL_DH_SOUL_CLEAVE_DMG                       = 228478,
    SPELL_DH_SOUL_FRAGMENT_COUNTER                 = 203981,
    SPELL_DH_SOUL_RENDING                          = 204909,
    SPELL_DH_SOUL_RENDING_VENGEANCE                = 217996,
    // NOTE: SPIRIT_BOMB_DAMAGE corrected from 218677 (this constant's original value, never
    // referenced anywhere in this file and thus unverified) to 247455, confirmed by two
    // independent reference sources agreeing exactly; SPIRIT_BOMB_HEAL (227255) already matched
    // both references and needed no change.
    SPELL_DH_SPIRIT_BOMB                           = 247454,
    SPELL_DH_SPIRIT_BOMB_DAMAGE                     = 247455,
    SPELL_DH_SPIRIT_BOMB_HEAL                      = 227255,
    SPELL_DH_SPIRIT_BOMB_VISUAL                    = 218678,
    SPELL_DH_THROW_GLAIVE                          = 185123,
    SPELL_DH_UNCONTAINED_FEL                       = 209261,
    SPELL_DH_VENGEANCE_DEMON_HUNTER                = 212613,
    SPELL_DH_VENGEFUL_RETREAT                      = 198813,
    SPELL_DH_VENGEFUL_RETREAT_FURY                 = 203650,
    SPELL_DH_VENGEFUL_RETREAT_TRIGGER              = 198793,

    SPELL_DH_ANGUISH                            = 202443,
    SPELL_DH_ANGUISH_DAMAGE                     = 202446,
    SPELL_DH_DESPERATE_INSTINCTS_TALENT         = 205478,
    SPELL_DH_EYE_OF_LEOTHERAS_MARK              = 206649,
    SPELL_DH_FEL_LANCE                          = 206966,
    SPELL_DH_FLAMING_SOUL                       = 238118,
    SPELL_DH_FUELED_BY_PAIN_TALENT              = 213017,
    SPELL_DH_FUELED_BY_PAIN_LINGERING_ORDEAL    = 238046,
    SPELL_DH_MANA_BREAK                         = 203704,
    SPELL_DH_NEMESIS_TALENT                     = 206491,
    SPELL_DH_REVERSE_MAGIC                      = 205604,
    SPELL_DH_SHATTERED_SOULS_HAVOC_KILL         = 178940,
    SPELL_DH_SHATTERED_SOULS_VENGEANCE_KILL     = 204254,
    SPELL_DH_SHATTER_SOUL_AT_HAVOC_LESSER       = 228536,
    SPELL_DH_SHATTER_SOUL_AT_HAVOC_NORMAL       = 226370,
    SPELL_DH_SHATTER_SOUL_AT_HAVOC_DEMON        = 226259,
    SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_NORMAL   = 226263,
    SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_DEMON    = 226264,
    SPELL_DH_SHATTER_SOUL_HAVOC_NORMAL_CAST     = 209651,
    SPELL_DH_METAMORPHOSIS_IMPACT_PLAYER        = 247121,
};

enum DemonHunterSpellCategories
{
    SPELL_CATEGORY_DH_EYE_BEAM      = 1582,
    SPELL_CATEGORY_DH_BLADE_DANCE   = 1640
};

// 179057 - Chaos Nova
// AoE stun; the stun itself is applied by this spell's own DB2 effect data (EFFECT_0). This
// only computes EFFECT_1's damage, scaled off attack power.
class spell_dh_chaos_nova : public SpellScript
{
    void HandleDamage(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        int32 ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
        AddPct(ap, 25);
        int32 damage = 1 + ap;

        damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE, GetEffectInfo(effIndex));
        damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE);

        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_chaos_nova::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 201427 - Annihilation
// Metamorphosis (Havoc) form's replacement for Chaos Strike - splits into separate mainhand/
// offhand trigger casts.
class spell_dh_annihilation : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_ANNIHILATION, SPELL_DH_ANNIHILATION_MH, SPELL_DH_ANNIHILATION_OH });
    }

    void HandleMainhand(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();
        if (caster && target)
            caster->CastSpell(target, SPELL_DH_ANNIHILATION_MH, true);
    }

    void HandleOffhand(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();
        if (caster && target)
            caster->CastSpell(target, SPELL_DH_ANNIHILATION_OH, true);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dh_annihilation::HandleMainhand, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectHit += SpellEffectFn(spell_dh_annihilation::HandleOffhand, EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

// 201428, 227518 - Annihilation (mainhand/offhand damage)
// Guards against dealing damage to self, matching the reference's own safety check.
class spell_dh_annihilation_damage : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (caster && target && caster == target)
            PreventHitDamage();
    }

    void Register() override
    {
        // real Annihilation (201428/227518) EFFECT_1 is SPELL_EFFECT_WEAPON_PERCENT_DAMAGE, not WEAPON_DAMAGE
        OnEffectHitTarget += SpellEffectFn(spell_dh_annihilation_damage::HandleHit, EFFECT_0, SPELL_EFFECT_NORMALIZED_WEAPON_DMG);
        OnEffectHitTarget += SpellEffectFn(spell_dh_annihilation_damage::HandleHit, EFFECT_1, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// 197125 - Chaos Strike
class spell_dh_chaos_strike : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_CHAOS_STRIKE_ENERGIZE });
    }

    void HandleEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SPELL_DH_CHAOS_STRIKE_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = eventInfo.GetProcSpell()
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dh_chaos_strike::HandleEffectProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// Blade Turning
// Gates the talent aura's own DB2 proc data to a parried hit only.
class spell_dh_blade_turning : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return (eventInfo.GetHitMask() & PROC_HIT_PARRY) != 0;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_blade_turning::CheckProc);
    }
};

// 213010 - Charred Warblades
class spell_dh_charred_warblades : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_CHARRED_WARBLADES_HEAL });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetDamageInfo() && eventInfo.GetDamageInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE;
    }

    void HandleAfterProc(ProcEventInfo& eventInfo)
    {
        _healAmount += CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), GetEffect(EFFECT_0)->GetAmount());
    }

    void HandleDummyTick(AuraEffect const* aurEff)
    {
        if (_healAmount == 0)
            return;

        GetTarget()->CastSpell(GetTarget(), SPELL_DH_CHARRED_WARBLADES_HEAL,
            CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR)
            .SetTriggeringAura(aurEff)
            .AddSpellBP0(_healAmount));

        _healAmount = 0;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_charred_warblades::CheckProc);
        AfterProc += AuraProcFn(spell_dh_charred_warblades::HandleAfterProc);
        // FIXME: real Charred Warblades (213010) EFFECT_0 is a plain (non-periodic - no
        // amplitude set) SPELL_AURA_DUMMY, not PERIODIC_DUMMY - this build's version doesn't
        // accumulate-then-release on a timer; left unresolved rather than guess the real trigger.
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dh_charred_warblades::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    uint32 _healAmount = 0;
};

// 209426 - Darkness
// Real 209426 (confirmed via SpellEffect.db2) has only 1 effect - EFFECT_0 is the SCHOOL_ABSORB
// aura itself, with EffectBasePoints 20 (the "20% chance" value) - there is no separate EFFECT_1
// to read the chance from. CalculateAmount() below overwrites the aura's live amount to -1
// (unlimited absorb), so the chance is instead read from the spell's own effect data directly.
class spell_dh_darkness : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        // Set absorbtion amount to unlimited
        amount = -1;
    }

    void Absorb(AuraEffect const* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount) const
    {
        int32 chance = GetEffectInfo(EFFECT_0).CalcValue(GetCaster());
        if (roll_chance_i(chance))
            absorbAmount = dmgInfo.GetDamage();
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dh_darkness::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_dh_darkness::Absorb, EFFECT_0);
    }
};

// 196718 - Darkness
// Id: 6615
struct areatrigger_dh_darkness : AreaTriggerAI
{
    areatrigger_dh_darkness(AreaTrigger* areaTrigger) : AreaTriggerAI(areaTrigger),
        _absorbAuraInfo(sSpellMgr->GetSpellInfo(SPELL_DH_DARKNESS_ABSORB, DIFFICULTY_NONE)) { }

    void OnUnitEnter(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster || !caster->IsValidAssistTarget(unit, _absorbAuraInfo))
            return;

        caster->CastSpell(unit, SPELL_DH_DARKNESS_ABSORB, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .SpellValueOverrides = { { SPELLVALUE_DURATION, at->GetDuration() } }
        });
    }

    void OnUnitExit(Unit* unit) override
    {
        unit->RemoveAura(SPELL_DH_DARKNESS_ABSORB, at->GetCasterGuid());
    }

private:
    SpellInfo const* _absorbAuraInfo;
};

// 213410 - Demonic (attached to 212084 - Fel Devastation and 198013 - Eye Beam)
class spell_dh_demonic : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ _transformSpellId })
            && ValidateSpellEffect({ { SPELL_DH_DEMONIC, EFFECT_0 } })
            && sSpellMgr->AssertSpellInfo(SPELL_DH_DEMONIC, DIFFICULTY_NONE)->GetEffect(EFFECT_0).IsAura();
    }

    bool Load() override
    {
        return GetCaster()->HasAuraEffect(SPELL_DH_DEMONIC, EFFECT_0);
    }

    void TriggerMetamorphosis() const
    {
        Unit* caster = GetCaster();
        AuraEffect const* demonic = caster->GetAuraEffect(SPELL_DH_DEMONIC, EFFECT_0);
        if (!demonic)
            return;

        int32 duration = demonic->GetAmount() + GetSpell()->GetChannelDuration();

        if (Aura* aura = caster->GetAura(_transformSpellId))
        {
            aura->SetMaxDuration(aura->GetDuration() + duration);
            aura->SetDuration(aura->GetMaxDuration());
            return;
        }

        SpellCastTargets targets;
        targets.SetUnitTarget(caster);

        Spell* spell = new Spell(caster, sSpellMgr->AssertSpellInfo(_transformSpellId, DIFFICULTY_NONE),
            TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            ObjectGuid::Empty, GetSpell()->m_castId);
        spell->m_SpellVisual.SpellXSpellVisualID = 0;
        spell->m_SpellVisual.ScriptVisualID = 0;
        spell->SetSpellValue({ SPELLVALUE_DURATION, duration });
        spell->prepare(targets);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_dh_demonic::TriggerMetamorphosis);
    }

    uint32 _transformSpellId;

public:
    explicit spell_dh_demonic(uint32 transformSpellId) : _transformSpellId(transformSpellId) { }
};

// Demonic Infusion
// Directly grants Demon Spikes.
class spell_dh_demonic_infusion : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_DEMON_SPIKES });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            caster->AddAura(SPELL_DH_DEMON_SPIKES, caster);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_demonic_infusion::HandleOnHit, EFFECT_0, SPELL_EFFECT_ENERGIZE);
    }
};

// 203720 - Demon Spikes
class spell_dh_demon_spikes : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_DEMON_SPIKES });
    }

    void HandleArmor(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_DH_DEMON_SPIKES, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_demon_spikes::HandleArmor, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 203819 - Demon Spikes (damage reduction)
// The actual defensive value of Demon Spikes: reduces damage taken by a flat percentage, offset
// slightly by the caster's own Mastery rating.
class spell_dh_demon_spikes_buff : public AuraScript
{
    void CalcAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* caster = aurEff->GetCaster();
        if (!caster)
            return;

        amount -= int32(std::ceil(caster->GetFloatValue(PLAYER_MASTERY)));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dh_demon_spikes_buff::CalcAmount, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    }
};

// 203555 - Demon Blades
// Havoc's baseline auto-attack replacement: every attack has a chance to trigger a Fury strike.
class spell_dh_demon_blades : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_DEMON_BLADES, SPELL_DH_DEMON_BLADES_DMG });
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target)
            return;

        SpellInfo const* demonBlades = sSpellMgr->GetSpellInfo(SPELL_DH_DEMON_BLADES, DIFFICULTY_NONE);
        if (demonBlades && roll_chance_i(demonBlades->GetEffect(EFFECT_0).CalcValue(caster)))
            caster->CastSpell(target, SPELL_DH_DEMON_BLADES_DMG, true);
    }

    void Register() override
    {
        // real Demon Blades (203555) EFFECT_0 is SPELL_AURA_PROC_TRIGGER_SPELL, not DUMMY
        OnEffectProc += AuraEffectProcFn(spell_dh_demon_blades::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 198813 - Vengeful Retreat
// If the Momentum talent is known, grants its movement speed buff.
class spell_dh_vengeful_retreat : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_MOMENTUM_TALENT, SPELL_DH_MOMENTUM });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(SPELL_DH_MOMENTUM_TALENT))
            caster->CastSpell(caster, SPELL_DH_MOMENTUM, true);
    }

    void Register() override
    {
        // real Vengeful Retreat (198813) EFFECT_0 is SPELL_EFFECT_APPLY_AURA (a snare), not TRIGGER_SPELL
        OnEffectHitTarget += SpellEffectFn(spell_dh_vengeful_retreat::HandleOnHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 198813 - Vengeful Retreat (Prepared talent trigger)
// A second, independent script bound to the same spell id as spell_dh_vengeful_retreat above
// (which handles the unrelated Momentum interaction) - if the Prepared talent is known, grants
// its Fury-regen buff.
class spell_dh_vengeful_retreat_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_VENGEFUL_RETREAT, SPELL_DH_VENGEFUL_RETREAT_FURY });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(SPELL_DH_PREPARED) && !caster->HasAura(SPELL_DH_VENGEFUL_RETREAT_FURY))
            caster->CastSpell(caster, SPELL_DH_VENGEFUL_RETREAT_FURY, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_vengeful_retreat_trigger::HandleOnHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 203650 - Prepared (Fury refiller)
// While active, periodically refunds Fury.
// Removed spell_dh_vengeful_retreat_fury_refiller: real 203650 EFFECT_0 is a plain
// SPELL_AURA_MOD_POWER_REGEN (not a periodic-tick aura at all) - already implemented natively by
// the engine (AuraEffect::HandleModPowerRegen, "implemented in Player::Regenerate"), so this
// script's manual "every tick, add 4 Fury" logic was modeling the wrong mechanism entirely and
// is redundant with what the base engine already does for this aura type.

// 198013 - Eye Beam
class spell_dh_eye_beam : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_EYE_BEAM_DAMAGE });
    }

    void HandleEffectPeriodic(AuraEffect const* aurEff)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_DH_EYE_BEAM_DAMAGE,
            CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR)
            .SetTriggeringAura(aurEff));
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dh_eye_beam::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 198013 - Eye Beam (initial cast, EFFECT_0 damage guard)
// The cast's own initial hit shouldn't deal damage directly - the periodic tick above
// (spell_dh_eye_beam) is what actually deals damage. Prevents a potential double-hit.
class spell_dh_eye_beam_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_EYE_BEAM, SPELL_DH_EYE_BEAM_DAMAGE });
    }

    void PreventHit(SpellEffIndex /*effIndex*/)
    {
        PreventHitDamage();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_eye_beam_trigger::PreventHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Called by 228477 - Soul Cleave
class spell_dh_feast_of_souls : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEAST_OF_SOULS, SPELL_DH_FEAST_OF_SOULS_PERIODIC_HEAL });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_DH_FEAST_OF_SOULS);
    }

    void HandleHeal() const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_DH_FEAST_OF_SOULS_PERIODIC_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_dh_feast_of_souls::HandleHeal);
    }
};

// 209400 - Razor Spikes
// While Demon Spikes is active, melee attacks also slow the target.
class spell_dh_razor_spikes : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_DEMON_SPIKES, SPELL_DH_RAZOR_SPIKES });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetDamageInfo() && (eventInfo.GetDamageInfo()->GetAttackType() == BASE_ATTACK || eventInfo.GetDamageInfo()->GetAttackType() == OFF_ATTACK);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = eventInfo.GetDamageInfo() ? eventInfo.GetDamageInfo()->GetAttacker() : nullptr;
        Unit* target = eventInfo.GetDamageInfo() ? eventInfo.GetDamageInfo()->GetVictim() : nullptr;
        if (!caster || !target)
            return;

        if (caster->HasAura(SPELL_DH_DEMON_SPIKES))
            caster->CastSpell(target, SPELL_DH_RAZOR_SPIKES, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_razor_spikes::CheckProc);
        // real 209400 EFFECT_0 is SPELL_AURA_ADD_FLAT_MODIFIER (the passive damage-increase half of
        // the tooltip); HandleProc was defined but never wired up, so the snare (210003) never fired
        OnEffectProc += AuraEffectProcFn(spell_dh_razor_spikes::HandleProc, EFFECT_0, SPELL_AURA_ADD_FLAT_MODIFIER);
    }
};

// 203556 - Master of the Glaive
// Gates the talent aura's own DB2 proc data to Throw Glaive only.
class spell_dh_master_of_the_glaive : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DH_THROW_GLAIVE;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_master_of_the_glaive::CheckProc);
    }
};

// 206473 - Bloodlet
// Throw Glaive has a chance to apply/refresh a bleed scaled off 40% of its hit damage, folding
// any remaining damage from the previous tick into the new application instead of losing it.
class spell_dh_bloodlet : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_THROW_GLAIVE, SPELL_DH_BLOODLET_DOT });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DH_THROW_GLAIVE;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target || !eventInfo.GetDamageInfo())
            return;

        int32 dmg = int32((eventInfo.GetDamageInfo()->GetDamage() * 2) / 5);

        if (AuraEffect* dot = target->GetAuraEffect(SPELL_DH_BLOODLET_DOT, EFFECT_0, caster->GetGUID()))
            dmg += int32(dot->GetAmount() * dot->GetRemainingTicks()) / 5;

        caster->CastSpell(target, SPELL_DH_BLOODLET_DOT, CastSpellExtraArgs(SPELLVALUE_BASE_POINT0, dmg).SetTriggerFlags(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_bloodlet::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dh_bloodlet::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 207810 - Nether Bond
// Casts the periodic health-linking tick.
class spell_dh_nether_bond : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_NETHER_BOND_PERIODIC });
    }

    void HandleCast()
    {
        Unit* caster = GetCaster();
        if (caster)
            caster->CastSpell(caster, SPELL_DH_NETHER_BOND_PERIODIC, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_dh_nether_bond::HandleCast);
    }
};

// 207811 - Nether Bond (periodic)
// Every tick, equalizes the caster's and the bonded ally's health percentage - whichever of the
// two is higher gets damaged down and the other healed up to the midpoint.
class spell_dh_nether_bond_periodic : public AuraScript
{
    Unit* _bondUnit = nullptr;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_NETHER_BOND, SPELL_DH_NETHER_BOND_DAMAGE });
    }

    Unit* FindBondUnit(Unit* caster)
    {
        std::list<Unit*> units;
        Trinity::AnyUnitInObjectRangeCheck check(caster, 100.0f);
        Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(caster, units, check);
        Cell::VisitAllObjects(caster, searcher, 100.0f);

        for (Unit* unit : units)
            if (unit->HasAura(SPELL_DH_NETHER_BOND, caster->GetGUID()))
                return unit;

        return nullptr;
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            _bondUnit = FindBondUnit(caster);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (!_bondUnit)
            _bondUnit = FindBondUnit(caster);
        if (!_bondUnit)
            return;

        float healthPct = (caster->GetHealthPct() + _bondUnit->GetHealthPct()) / 2.0f;

        int32 casterHealBp = 0, casterDamageBp = 0, bondHealBp = 0, bondDamageBp = 0;

        if (caster->GetHealthPct() < _bondUnit->GetHealthPct())
        {
            casterHealBp = int32(caster->CountPctFromMaxHealth(healthPct) - caster->GetHealth());
            bondDamageBp = int32(_bondUnit->GetHealth() - _bondUnit->CountPctFromMaxHealth(healthPct));
        }
        else
        {
            casterDamageBp = int32(caster->GetHealth() - caster->CountPctFromMaxHealth(healthPct));
            bondHealBp = int32(_bondUnit->CountPctFromMaxHealth(healthPct) - _bondUnit->GetHealth());
        }

        caster->CastSpell(caster, SPELL_DH_NETHER_BOND_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK)
            .AddSpellMod(SPELLVALUE_BASE_POINT0, casterDamageBp).AddSpellMod(SPELLVALUE_BASE_POINT1, casterHealBp));
        caster->CastSpell(_bondUnit, SPELL_DH_NETHER_BOND_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK)
            .AddSpellMod(SPELLVALUE_BASE_POINT0, bondDamageBp).AddSpellMod(SPELLVALUE_BASE_POINT1, bondHealBp));
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_dh_nether_bond_periodic::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dh_nether_bond_periodic::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 212084 - Fel Devastation
// NOTE: correction to an earlier version of this class - it previously only cast the heal each
// tick, never the damage, meaning Fel Devastation dealt zero damage in practice. Fixed to cast
// both, matching the reference's own periodic driver.
class spell_dh_fel_devastation : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEL_DEVASTATION_DMG, SPELL_DH_FEL_DEVASTATION_HEAL });
    }

    void HandlePeriodicEffect(AuraEffect const* aurEff) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        CastSpellExtraArgs args(CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
        caster->CastSpell(caster, SPELL_DH_FEL_DEVASTATION_DMG, args);
        caster->CastSpell(caster, SPELL_DH_FEL_DEVASTATION_HEAL, args);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dh_fel_devastation::HandlePeriodicEffect, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 212105 - Fel Devastation (damage)
class spell_dh_fel_devastation_damage : public SpellScript
{
    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        SetHitDamage(int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) + 1.0f));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_fel_devastation_damage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 212106 - Fel Devastation (heal)
class spell_dh_fel_devastation_heal : public SpellScript
{
    void HandleHeal(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        SetHitHeal(int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 2.5f));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_fel_devastation_heal::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// 204022 - Fiery Brand (damage reduction)
// While the target of this aura is being attacked by someone they have branded (i.e. who has
// applied the Fiery Brand DoT to them), reduces that incoming hit by 40% - the actual defensive
// payoff of using Fiery Brand as a tank cooldown.
class spell_dh_fiery_brand_absorb : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* attacker = dmgInfo.GetAttacker();
        if (attacker && attacker->HasAura(SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2))
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), 40);
    }

    // FIXME: real Fiery Brand (204022) EFFECT_0 is SPELL_AURA_PERIODIC_DUMMY, not SCHOOL_ABSORB -
    // its own tooltip in this build ("Branded enemies deal X% less damage to you") describes a
    // flat damage-taken reduction, not a self-absorb shield. This class's mechanic doesn't match
    // the real spell; left unresolved rather than guess at a redesign.
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dh_fiery_brand_absorb::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_dh_fiery_brand_absorb::HandleAbsorb, EFFECT_0);
    }
};

// 211881 - Fel Eruption
// The stun applies via this spell's own DB2 effect data; this only fires the accompanying
// damage payload on the same target.
class spell_dh_fel_eruption : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEL_ERUPTION, SPELL_DH_FEL_ERUPTION_DAMAGE });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();
        if (caster && target)
            caster->CastSpell(target, SPELL_DH_FEL_ERUPTION_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_fel_eruption::HandleOnHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 195072 - Fel Rush
// The Momentum talent reads "Vengeful Retreat or Fel Rush increase movement speed" - this half
// of that grant lives here; the Vengeful Retreat half is in spell_dh_vengeful_retreat below.
class spell_dh_fel_rush : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEL_RUSH_GROUND, SPELL_DH_FEL_RUSH_WATER_AIR, SPELL_DH_FEL_RUSH_DMG, SPELL_DH_MOMENTUM_TALENT, SPELL_DH_MOMENTUM });
    }

    void HandleDash()
    {
        Unit* caster = GetCaster();

        if (caster->HasAura(SPELL_DH_MOMENTUM_TALENT))
            caster->CastSpell(caster, SPELL_DH_MOMENTUM, true);

        // Cancel any active walking/running movement so W-key momentum doesn't add to the
        // charge velocity (most visible in the air where there's no ground collision).
        caster->StopMoving();

        // 197922/197923 both use SPELL_EFFECT_CHARGE_DEST which requires an explicit destination;
        // without one the effect returns early and no movement happens.
        uint32 spellId = (caster->IsInWater() || caster->IsFlying() || caster->IsFalling())
            ? SPELL_DH_FEL_RUSH_WATER_AIR
            : SPELL_DH_FEL_RUSH_GROUND;

        Position dest = caster->GetFirstCollisionPosition(15.0f, 0.0f);

        caster->CastSpell(dest, spellId, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_dh_fel_rush::HandleDash);
    }
};

// 197922 - Fel Rush (ground variant)
// 197923 - Fel Rush (air/water variant)
// Both carry SPELL_EFFECT_CHARGE_DEST. EffectChargeDest delays its HIT phase until the
// charge movement finishes (UpdateDelayMomentForDst), so OnEffectHit here fires only
// after the player has landed — giving correct damage timing.
class spell_dh_fel_rush_charge : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEL_RUSH_DMG });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/) const
    {
        // real Fel Rush Damage (192611) targets TARGET_DEST_DEST + TARGET_UNIT_LINE_CASTER_TO_DEST_ENEMY
        // (a line from caster to the stored destination) - it needs an actual destination to search
        // along, or its own implicit-target resolution finds zero enemies and nobody ever takes
        // damage. 197922/197923 were themselves cast at an explicit Position (see
        // spell_dh_fel_rush::HandleDash), so that same destination is still available here.
        WorldLocation const* dest = GetExplTargetDest();
        if (!dest)
            return;

        GetCaster()->CastSpell(*dest, SPELL_DH_FEL_RUSH_DMG, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        // correction: real Fel Rush (197922/197923) EFFECT_0 is plain SPELL_EFFECT_DUMMY - unlike
        // Infernal Strike, it has no JUMP_CHARGE effect at all (7 effects total, mostly speed/
        // movement auras matching the same "Roll"-style dash shape used elsewhere in this file)
        OnEffectHit += SpellEffectFn(spell_dh_fel_rush_charge::HandleDamage, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 197923 - Fel Rush (air/water dash)
// Governs using Fel Rush while airborne/underwater: a fixed dash speed while active, and
// restoring normal gravity/fall/hover state once the dash ends.
class spell_dh_fel_rush_dash : public SpellScript
{
    void PreventTrigger(SpellEffIndex effIndex)
    {
        PreventHitEffect(effIndex);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_dh_fel_rush_dash::PreventTrigger, EFFECT_6, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectHit += SpellEffectFn(spell_dh_fel_rush_dash::PreventTrigger, EFFECT_6, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

class spell_dh_fel_rush_dash_AuraScript : public AuraScript
{
    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->SetDisableGravity(false);
        caster->SetFall(true);
        caster->SetPlayHoverAnim(false);
    }

    // Previously overrode EFFECT_3 (SPELL_AURA_MOD_MINIMUM_SPEED) to a hardcoded 1400 - real
    // 197923 already carries 650 natively on both EFFECT_1 (SPELL_AURA_MOD_SPEED_NO_CONTROL) and
    // EFFECT_3 (SPELL_AURA_MOD_MINIMUM_SPEED), a matched pair (same shape Monk's Roll uses in
    // spell_monk.cpp, just without needing Roll's own "Values need manual correction" fixup - this
    // spell's real data needed no correction at all). The 1400 override was simply wrong, not a
    // needed fix, and combined with MOD_SPEED_NO_CONTROL previously being unimplemented at the
    // engine level (see AuraEffect::HandleAuraModSpeedNoControl / Unit::UpdateSpeed), nothing ever
    // capped the resulting speed - player-driven input kept compounding on top while airborne.
    // No script-side amount override needed now that both are read correctly by the engine.

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_dh_fel_rush_dash_AuraScript::AfterRemove, EFFECT_9, SPELL_AURA_MOD_MINIMUM_SPEED_RATE, AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK);
    }
};

// 189110 - Infernal Strike
// Vengeance's mobility/defensive cooldown: leap to the target destination and slam down,
// dealing impact damage. The reference also grants a brief landing immunity from this same
// spell's OnCast, but that's the same missing SPELL_AURA_ALLOW_ONLY_ABILITY-driven mechanic
// already left unported for Metamorphosis's landing burst (see spell_dh_metamorphosis_immunity
// above) - the leap and impact damage below are unaffected.
class spell_dh_infernal_strike : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_INFERNAL_STRIKE_JUMP, SPELL_DH_INFERNAL_STRIKE_IMPACT_DAMAGE });
    }

    SpellCastResult CheckDestination()
    {
        if (WorldLocation const* dest = GetExplTargetDest())
        {
            if (GetCaster()->HasUnitMovementFlag(MOVEMENTFLAG_ROOT))
                return SPELL_FAILED_ROOTED;

            if (GetCaster()->GetMap()->Instanceable())
            {
                float range = GetSpellInfo()->GetMaxRange(true, GetCaster()) * 1.5f;

                PathGenerator generatedPath(GetCaster());
                generatedPath.SetPathLengthLimit(range);

                bool result = generatedPath.CalculatePath(dest->GetPositionX(), dest->GetPositionY(), dest->GetPositionZ(), false);
                if (generatedPath.GetPathType() & PATHFIND_SHORT)
                    return SPELL_FAILED_OUT_OF_RANGE;
                else if (!result || generatedPath.GetPathType() & PATHFIND_NOPATH)
                    return SPELL_FAILED_NOPATH;
            }
            else if (dest->GetPositionZ() > GetCaster()->GetPositionZ() + 4.0f)
                return SPELL_FAILED_NOPATH;

            return SPELL_CAST_OK;
        }

        return SPELL_FAILED_NO_VALID_TARGETS;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        WorldLocation const* dest = GetHitDest();
        if (!caster || !dest)
            return;

        caster->CastSpell(*dest, SPELL_DH_INFERNAL_STRIKE_JUMP, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dh_infernal_strike::CheckDestination);
        OnEffectHit += SpellEffectFn(spell_dh_infernal_strike::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 189111 - Infernal Strike (jump)
// Mirrors spell_dh_fel_rush_charge below: SPELL_EFFECT_CHARGE_DEST delays the HIT phase until
// the leap's movement finishes, so the impact damage only lands once the caster touches down.
class spell_dh_infernal_strike_jump : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_INFERNAL_STRIKE_IMPACT_DAMAGE });
    }

    void HandleImpact(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_DH_INFERNAL_STRIKE_IMPACT_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        // real Infernal Strike (189111) EFFECT_0 is SPELL_EFFECT_JUMP_CHARGE, not CHARGE_DEST
        OnEffectHit += SpellEffectFn(spell_dh_infernal_strike_jump::HandleImpact, EFFECT_0, SPELL_EFFECT_JUMP_CHARGE);
    }
};

// 205630 - Illidan's Grasp
// Vengeance's signature mobility/utility spike: casting it on an unmarked enemy sticks a spike
// in them (the mark itself is this spell's own DB2 apply-aura effect data, no script needed for
// that half); casting it again on an already-marked target instead pulls them to the caster and
// deals damage.
// NOTE: the only available reference source tracks "did I mark this target" via
// a non-standard per-unit "who has my aura" list (TargetsWhoHasMyAuras/m_whoHasMyAuras) that
// doesn't exist anywhere in ArgusCore's engine. Substituted with a plain
// HasAura(SPELL_DH_ILLIDANS_GRASP, casterGuid) check on the explicit hit target, which answers
// the same "did I personally mark this target" question through a standard, already-used-
// throughout-this-file API instead.
class spell_dh_illidans_grasp : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_ILLIDANS_GRASP, SPELL_DH_ILLIDANS_GRASP_JUMP_DEST, SPELL_DH_ILLIDANS_GRASP_DAMAGE });
    }

    void HandleHit(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (!target->HasAura(SPELL_DH_ILLIDANS_GRASP, caster->GetGUID()))
            return;

        PreventHitDefaultEffect(effIndex);
        target->RemoveAura(SPELL_DH_ILLIDANS_GRASP, caster->GetGUID());
        target->CastSpell(caster, SPELL_DH_ILLIDANS_GRASP_JUMP_DEST, true);
        target->CastSpell(caster, SPELL_DH_ILLIDANS_GRASP_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_illidans_grasp::HandleHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 192939 - Fel Mastery
// Fel Rush's damage tick has a chance to generate Fury.
class spell_dh_fel_mastery : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DH_FEL_RUSH_DMG;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 amountToAdd = GetSpellInfo()->GetEffect(EFFECT_0).CalcValue(caster);
        int32 fury = caster->GetPower(POWER_FURY);
        int32 maxFury = caster->GetMaxPower(POWER_FURY);
        caster->SetPower(POWER_FURY, std::min(fury + amountToAdd, maxFury));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_fel_mastery::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dh_fel_mastery::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 232893 - Felblade
class spell_dh_felblade : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FELBLADE_CHARGE });
    }

    void HandleCharge(SpellEffIndex /*effIndex*/) const
    {
        uint32 spellToCast = GetCaster()->IsWithinMeleeRange(GetHitUnit()) ? SPELL_DH_FELBLADE_DAMAGE : SPELL_DH_FELBLADE_CHARGE;
        GetCaster()->CastSpell(GetHitUnit(), spellToCast, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_felblade::HandleCharge, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 213241 - Felblade Charge
class spell_dh_felblade_charge : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FELBLADE_DAMAGE });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_DH_FELBLADE_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_felblade_charge::HandleDamage, EFFECT_0, SPELL_EFFECT_CHARGE);
    }
};

// 203557 - Felblade (Vengeance cooldow reset proc aura)
// 236167 - Felblade (Havoc cooldow reset proc aura)
class spell_dh_felblade_cooldown_reset_proc : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FELBLADE });
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& /*eventInfo*/) const
    {
        GetTarget()->GetSpellHistory()->ResetCooldown(SPELL_DH_FELBLADE, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dh_felblade_cooldown_reset_proc::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 204021 - Fiery Brand
class spell_dh_fiery_brand : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FIERY_BRAND_DEBUFF_RANK_1 });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_DH_FIERY_BRAND_DEBUFF_RANK_1, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_fiery_brand::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 206416 - First Blood
class spell_dh_first_blood : public AuraScript
{
public:
    ObjectGuid const& GetFirstTarget() const { return _firstTargetGUID; }
    void SetFirstTarget(ObjectGuid const& targetGuid) { _firstTargetGUID = targetGuid; }

private:
    void Register() override
    {
    }

private:
    ObjectGuid _firstTargetGUID;
};

// 209258 - Last Resort
// 178963, 203794, 228532 - Consume Soul
// The Fury/Pain refund is gated behind Demonic Appetite - without that talent, this effect
// shouldn't fire at all.
class spell_dh_consume_soul : public SpellScript
{
    void PreventPower(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        if (caster && !caster->HasAura(SPELL_DH_DEMONIC_APPETITE))
            PreventHitEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dh_consume_soul::PreventPower, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectHitTarget += SpellEffectFn(spell_dh_consume_soul::PreventPower, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunch += SpellEffectFn(spell_dh_consume_soul::PreventPower, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunchTarget += SpellEffectFn(spell_dh_consume_soul::PreventPower, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

// 206478 - Demonic Appetite (Havoc talent passive): Chaos Strike/Annihilation main-hand crits
// spawn a Lesser Soul Fragment. The Fury refund on consuming it is already handled by
// spell_dh_consume_soul above (which already gates Consume Soul's own EFFECT_1 Fury trigger
// behind HasAura(SPELL_DH_DEMONIC_APPETITE)) - this class only needs to add the missing
// crit-triggers-fragment-spawn half, bound directly to the talent's own always-active passive
// aura, matching the file's established "baseline passive with proc logic on its own id" idiom
// (e.g. Frozen Veins in spell_mage.cpp). Confirmed via two independent reference sources (identical
// implementations). Casts SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_1 (228533), the
// Havoc "lesser" fragment trigger - not SPELL_DH_SHATTER_SOUL_HAVOC_NORMAL_CAST, which the
// on-kill passive already uses for the "big" fragment. Checks ArgusCore's own
// SPELL_DH_CHAOS_STRIKE_OH/SPELL_DH_ANNIHILATION_OH (199547/201428) - despite the differing
// names, these are the exact same ids the references check under their own
// (inverted-relative-to-ArgusCore) MAIN_HAND-labeled constants; checking only one hand's hit
// id per cast avoids double-proccing per Chaos Strike/Annihilation cast.
class spell_dh_demonic_appetite : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_1 });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() &&
              (eventInfo.GetSpellInfo()->Id == SPELL_DH_CHAOS_STRIKE_OH ||
               eventInfo.GetSpellInfo()->Id == SPELL_DH_ANNIHILATION_OH);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_1, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_demonic_appetite::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dh_demonic_appetite::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 211053 - Fel Barrage (Havoc capstone talent): channeled AOE whose duration/damage scales
// with how many Fel Barrage charges the caster had banked when the channel began (built up via
// spell_dh_fel_barrage_proc below). Confirmed via two independent reference sources (identical
// implementations). ArgusCore's SpellHistory has no direct "current charge count" getter
// (unlike the reference's GetChargeCount) - counted the same way this project's own
// spell_paladin.cpp already does elsewhere (loop HasCharge/ConsumeCharge), which also
// naturally consumes them - matching Fel Barrage's real "consumes all banked charges on cast"
// design. Translated Unit::CastCustomSpell (doesn't exist in ArgusCore) to
// CastSpellExtraArgs/AddSpellMod.
class spell_dh_fel_barrage : public AuraScript
{
    int32 _charges = 0;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEL_BARRAGE_DMG });
    }

    bool Load() override
    {
        Unit* caster = GetCaster();
        if (!caster)
            return true;

        SpellHistory* history = caster->GetSpellHistory();
        uint32 chargeCategoryId = GetSpellInfo()->ChargeCategoryId;

        _charges = 0;
        while (history->HasCharge(chargeCategoryId))
        {
            history->ConsumeCharge(chargeCategoryId);
            ++_charges;
        }

        return true;
    }

    void HandleTrigger(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget(), SPELL_DH_FEL_BARRAGE_DMG, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT1, _charges));
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dh_fel_barrage::HandleTrigger, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 211052 - Fel Barrage (damage): scales per tick by the charge count passed via
// SPELLVALUE_BASE_POINT1 from spell_dh_fel_barrage above - deliberately NOT
// SPELLVALUE_BASE_POINT0/GetEffectValue(), since EFFECT_0 on this spell is itself the
// SCHOOL_DAMAGE effect being scaled (the same EFFECT_0-collision problem already solved for
// Spirit Bomb above, avoided here more directly via GetSpellValue()->EffectBasePoints[1],
// which exists in ArgusCore and isn't tied to any single effect index).
class spell_dh_fel_barrage_damage : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        int32 chargesUsed = GetSpellValue()->EffectBasePoints[EFFECT_1];
        if (chargesUsed > 0)
            SetHitDamage(GetHitDamage() * chargesUsed / 5);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_fel_barrage_damage::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 222703 - Fel Mastery (the Fel Barrage charge-builder passive): Blade Dance/Death Sweep,
// Chaos Strike/Annihilation, and Eye Beam landing restores a Fel Barrage charge, banked for
// the next Fel Barrage cast above. Excludes Blade Dance/Death Sweep's own multiple per-cast
// sub-hits (199552/200685/210153/210155 - no named constants exist for these in ArgusCore
// either) and Fel Barrage's own damage (SPELL_DH_FEL_BARRAGE_DMG) from re-triggering itself.
// Confirmed via two independent reference sources (identical implementations).
class spell_dh_fel_barrage_proc : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEL_BARRAGE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (!eventInfo.GetSpellInfo())
            return false;

        static std::vector<uint32> const excluded =
        {
            199552, 200685, 210153, 210155, SPELL_DH_FEL_BARRAGE_DMG
        };
        return std::find(excluded.begin(), excluded.end(), eventInfo.GetSpellInfo()->Id) == excluded.end();
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->GetSpellHistory()->RestoreCharge(sSpellMgr->AssertSpellInfo(SPELL_DH_FEL_BARRAGE, DIFFICULTY_NONE)->ChargeCategoryId);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_fel_barrage_proc::CheckProc);
        // real Fel Barrage Proc (222703) EFFECT_0 is SPELL_AURA_DUMMY, not PROC_TRIGGER_SPELL
        OnEffectProc += AuraEffectProcFn(spell_dh_fel_barrage_proc::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 178963, 203794, 228532 - Consume Soul (Vengeance)
// A second, independent script bound to the same spell ids as spell_dh_consume_soul above
// (which gates the Fury/Pain refund behind Demonic Appetite) - this adds two unrelated
// interactions: granting Gluttony's buff if talented, and boosting Soul Barrier's absorb amount
// if the shield is currently up.
class spell_dh_consume_soul_vengeance : public SpellScript
{
    void HandleCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(SPELL_DH_GLUTTONY))
            caster->CastSpell(caster, SPELL_DH_GLUTTONY_BUFF, true);

        if (Aura* soulBarrier = caster->GetAura(SPELL_DH_SOUL_BARRIER))
            if (AuraEffect* aurEff = soulBarrier->GetEffect(EFFECT_0))
                aurEff->SetAmount(aurEff->GetAmount() + int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 2.5f));
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_dh_consume_soul_vengeance::HandleCast);
    }
};

// 207760 - Burning Alive (Fiery Brand rank 2 spread)
// When the branded target takes another hit, spreads the debuff to one random nearby enemy that
// doesn't already have it, scaling the new application's damage off the caster's attack power.
class spell_dh_burning_alive : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2 });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2));
        Trinity::Containers::RandomResize(targets, 1);
    }

    void OnHitEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (target)
            target->CastSpell(target, SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2, true);
    }

    void HandleAfterHit()
    {
        Unit* caster = GetOriginalCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!player || !target || !target->HasAura(SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2))
            return;

        if (AuraEffect* eff = target->GetAuraEffect(SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2, EFFECT_0))
            eff->SetAmount(int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dh_burning_alive::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_dh_burning_alive::OnHitEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        AfterHit += SpellHitFn(spell_dh_burning_alive::HandleAfterHit);
    }
};

// 227225 - Soul Barrier
// The absorb shield scales with 15x attack power.
// NOTE: the reference also has this spell consume nearby Soul Fragment AreaTriggers for a bonus
// effect, using the same hardcoded-AreaTrigger-entry-id/old fragment-consumption approach
// already found and deferred for Soul Cleave (see that fix's writeup) - not ported here for the
// same reason: it's a broader missing system, not something specific to this ability.
class spell_dh_soul_barrier : public AuraScript
{
    void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (player)
            amount = int32(15.0f * player->GetTotalAttackPowerValue(BASE_ATTACK));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dh_soul_barrier::CalcAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

class spell_dh_last_resort : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_DH_UNCONTAINED_FEL, SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void HandleAbsorb(AuraEffect const* /*aurEff*/, DamageInfo const& /*dmgInfo*/, uint32& absorbAmount)
    {
        Unit* target = GetTarget();
        if (target->HasAura(SPELL_DH_UNCONTAINED_FEL))
        {
            absorbAmount = 0;
            return;
        }

        PreventDefaultAction();

        CastSpellExtraArgs castArgs = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD;

        target->CastSpell(target, SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM, castArgs);
        target->CastSpell(target, SPELL_DH_UNCONTAINED_FEL, castArgs);

        target->SetHealth(target->CountPctFromMaxHealth(GetEffectInfo(EFFECT_1).CalcValue(target)));
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_dh_last_resort::HandleAbsorb, EFFECT_0);
    }
};

// 188499 - Blade Dance
// 210152 - Death Sweep
class spell_dh_blade_dance : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FIRST_BLOOD });
    }

    void DecideFirstTarget(std::list<WorldObject*>& targetList)
    {
        if (targetList.empty())
            return;

        Aura* aura = GetCaster()->GetAura(SPELL_DH_FIRST_BLOOD);
        if (!aura)
            return;

        ObjectGuid firstTargetGUID = ObjectGuid::Empty;
        ObjectGuid selectedTarget = GetCaster()->GetTarget();

        // Prefer the selected target if he is one of the enemies
        if (targetList.size() > 1 && !selectedTarget.IsEmpty())
        {
            auto it = std::find_if(targetList.begin(), targetList.end(), [selectedTarget](WorldObject* object)
            {
                return object->GetGUID() == selectedTarget;
            });
            if (it != targetList.end())
                firstTargetGUID = (*it)->GetGUID();
        }

        if (firstTargetGUID.IsEmpty())
            firstTargetGUID = targetList.front()->GetGUID();

        if (spell_dh_first_blood* script = aura->GetScript<spell_dh_first_blood>())
            script->SetFirstTarget(firstTargetGUID);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dh_blade_dance::DecideFirstTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 199552 - Blade Dance
// 200685 - Blade Dance
// 210153 - Death Sweep
// 210155 - Death Sweep
class spell_dh_blade_dance_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FIRST_BLOOD });
    }

    void HandleHitTarget()
    {
        int32 damage = GetHitDamage();

        // First Blood: bonus damage to the caster's selected target (same logic as LegionCore reference).
        // Using GetTarget() is reliable regardless of whether spell_dh_blade_dance fired on the main cast.
        if (AuraEffect const* aurEff = GetCaster()->GetAuraEffect(SPELL_DH_FIRST_BLOOD, EFFECT_0))
            if (GetHitUnit()->GetGUID() == GetCaster()->GetTarget())
                AddPct(damage, aurEff->GetAmount());

        SetHitDamage(damage);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dh_blade_dance_damage::HandleHitTarget);
    }
};

// 131347 - Glide
class spell_dh_glide : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_GLIDE_KNOCKBACK, SPELL_DH_GLIDE_DURATION, SPELL_DH_VENGEFUL_RETREAT_TRIGGER, SPELL_DH_FEL_RUSH });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        if (caster->IsMounted() || caster->GetVehicleBase())
            return SPELL_FAILED_DONT_REPORT;

        if (!caster->IsFalling())
            return SPELL_FAILED_NOT_ON_GROUND;

        return SPELL_CAST_OK;
    }

    void HandleCast()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_DH_GLIDE_KNOCKBACK, true);
        caster->CastSpell(caster, SPELL_DH_GLIDE_DURATION, true);

        caster->GetSpellHistory()->StartCooldown(sSpellMgr->AssertSpellInfo(SPELL_DH_VENGEFUL_RETREAT_TRIGGER, GetCastDifficulty()), 0, nullptr, false, 250ms);
        caster->GetSpellHistory()->StartCooldown(sSpellMgr->AssertSpellInfo(SPELL_DH_FEL_RUSH, GetCastDifficulty()), 0, nullptr, false, 250ms);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dh_glide::CheckCast);
        BeforeCast += SpellCastFn(spell_dh_glide::HandleCast);
    }
};

// 131347 - Glide
class spell_dh_glide_AuraScript : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_GLIDE_DURATION });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAura(SPELL_DH_GLIDE_DURATION);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_dh_glide_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_FEATHER_FALL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 197154 - Glide
class spell_dh_glide_timer : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_GLIDE });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAura(SPELL_DH_GLIDE);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_dh_glide_timer::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 258920 - Immolation Aura
// The periodic damage tick is handled entirely by this spell's own DB2 effect data - this only
// shortens the purely-cosmetic glow visual (201122) to match the buff's real 6s duration
// instead of the visual spell's own default 10s, then hides its (now-redundant) buff icon.
class spell_dh_immolation_aura : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_IMMOLATION_AURA, SPELL_DH_IMMOLATION_AURA_VISUAL });
    }

    void ApplyVisual(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_DH_IMMOLATION_AURA_VISUAL, true);

        if (Aura* visual = caster->GetAura(SPELL_DH_IMMOLATION_AURA_VISUAL))
        {
            visual->SetDuration(6000);
            caster->RemoveVisibleAura(visual->GetApplicationOfTarget(caster->GetGUID()));
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dh_immolation_aura::ApplyVisual, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

// 202138 - Sigil of Chains
// 204596 - Sigil of Flame
// 207684 - Sigil of Misery
// 202137 - Sigil of Silence
template<uint32 TriggerSpellId, uint32 TriggerSpellId2 = 0>
struct areatrigger_dh_generic_sigil : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnRemove() override
    {
        if (Unit* caster = at->GetCaster())
        {
            caster->CastSpell(at->GetPosition(), TriggerSpellId);
            if constexpr (TriggerSpellId2 != 0)
                caster->CastSpell(at->GetPosition(), TriggerSpellId2);
        }
    }
};

// 208673 - Sigil of Chains
class spell_dh_sigil_of_chains : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_SIGIL_OF_CHAINS_SLOW, SPELL_DH_SIGIL_OF_CHAINS_GRIP });
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/)
    {
        if (WorldLocation const* loc = GetExplTargetDest())
        {
            GetCaster()->CastSpell(GetHitUnit(), SPELL_DH_SIGIL_OF_CHAINS_SLOW, true);
            GetHitUnit()->CastSpell(loc->GetPosition(), SPELL_DH_SIGIL_OF_CHAINS_GRIP, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_sigil_of_chains::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};


// 207685 - Sigil of Misery (fear)
// Breaks the fear early once the target has taken more than 10% of its max health in cumulative
// damage while feared - standard "fear breaks on sufficient damage" behavior, which isn't
// automatic engine behavior and needs to be tracked explicitly.
// NOTE: the reference tracks the running damage total via `Aura::Variables`, a generic runtime
// key-value scripting-storage member that doesn't exist anywhere in ArgusCore's engine. Since
// an AuraScript instance already lives for exactly one aura application's lifetime, a plain
// member variable on the script class itself does the same job with no functional difference.
class spell_dh_sigil_of_misery_fear : public AuraScript
{
    uint64 _damage = 0;

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* target = eventInfo.GetActionTarget();
        if (!target || !eventInfo.GetDamageInfo())
            return false;

        _damage += eventInfo.GetDamageInfo()->GetDamage();
        if (_damage > target->CountPctFromMaxHealth(10))
            if (Aura* fear = GetAura())
                fear->SetDuration(0);

        return true;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dh_sigil_of_misery_fear::CheckProc);
    }
};

// 202443 - Anguish
// On aura removal fires accumulated stacks as a single burst hit.
class spell_dh_anguish : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_ANGUISH_DAMAGE });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_DH_ANGUISH_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, int32(GetStackAmount()) } }
        });
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_dh_anguish::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 202446 - Anguish
// Multiplies base damage by the stack count forwarded via SPELLVALUE_BASE_POINT0.
class spell_dh_anguish_damage : public SpellScript
{
    void HandleOnHit()
    {
        if (GetHitUnit())
            SetHitDamage(GetHitDamage() * GetSpellValue()->EffectBasePoints[EFFECT_0]);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dh_anguish_damage::HandleOnHit);
    }
};

// 206491 - Nemesis
// When the marked target dies, apply a long-duration damage bonus matching its creature type.
class spell_dh_nemesis : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            SPELL_DH_NEMESIS_HUMANOIDS,  SPELL_DH_NEMESIS_DEMONS,     SPELL_DH_NEMESIS_ABERRATIONS,
            SPELL_DH_NEMESIS_BEASTS,     SPELL_DH_NEMESIS_CRITTERS,   SPELL_DH_NEMESIS_DRAGONKIN,
            SPELL_DH_NEMESIS_ELEMENTALS, SPELL_DH_NEMESIS_GIANTS,     SPELL_DH_NEMESIS_MECHANICALS,
            SPELL_DH_NEMESIS_UNDEAD
        });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            return;

        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        uint32 spellId = 0;
        switch (target->GetCreatureType())
        {
            case CREATURE_TYPE_HUMANOID:   spellId = SPELL_DH_NEMESIS_HUMANOIDS;   break;
            case CREATURE_TYPE_DEMON:      spellId = SPELL_DH_NEMESIS_DEMONS;      break;
            case CREATURE_TYPE_ABERRATION: spellId = SPELL_DH_NEMESIS_ABERRATIONS; break;
            case CREATURE_TYPE_BEAST:      spellId = SPELL_DH_NEMESIS_BEASTS;      break;
            case CREATURE_TYPE_CRITTER:    spellId = SPELL_DH_NEMESIS_CRITTERS;    break;
            case CREATURE_TYPE_DRAGONKIN:  spellId = SPELL_DH_NEMESIS_DRAGONKIN;   break;
            case CREATURE_TYPE_ELEMENTAL:  spellId = SPELL_DH_NEMESIS_ELEMENTALS;  break;
            case CREATURE_TYPE_GIANT:      spellId = SPELL_DH_NEMESIS_GIANTS;      break;
            case CREATURE_TYPE_MECHANICAL: spellId = SPELL_DH_NEMESIS_MECHANICALS; break;
            case CREATURE_TYPE_UNDEAD:     spellId = SPELL_DH_NEMESIS_UNDEAD;      break;
            default: return;
        }

        // Duration comes from the buff spell data (300 sec in 7.3.5).
        caster->CastSpell(caster, spellId, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR
        });
    }

    void Register() override
    {
        // real Nemesis (206491) EFFECT_0 is SPELL_AURA_MOD_SCHOOL_MASK_DAMAGE_FROM_CASTER, not MOD_DAMAGE_PERCENT_TAKEN
        AfterEffectRemove += AuraEffectRemoveFn(spell_dh_nemesis::OnRemove, EFFECT_0, SPELL_AURA_MOD_SCHOOL_MASK_DAMAGE_FROM_CASTER, AURA_EFFECT_HANDLE_REAL);
    }
};

// 205478 - Desperate Instincts
// Automatically casts Blur when the DH falls below 35% HP (respects Blur's cooldown).
class spell_dh_desperate_instincts : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_BLUR_TRIGGER });
    }

    void HandleTrigger(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->GetHealthPct() >= 35.f)
        {
            Remove();
            return;
        }

        if (!caster->GetSpellHistory()->HasCooldown(SPELL_DH_BLUR_TRIGGER))
            caster->CastSpell(caster, SPELL_DH_BLUR_TRIGGER, CastSpellExtraArgsInit{
                // Respect category cooldown so Blur can only auto-fire once per real cooldown.
                .TriggerFlags = TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD)
            });
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dh_desperate_instincts::HandleTrigger, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 205604 - Reverse Magic (Honor Talent)
// Strips all dispellable magic/poison effects from the target and copies them back to their original caster.
class spell_dh_reverse_magic : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        uint32 const dispelMask = (1 << DISPEL_MAGIC) | (1 << DISPEL_POISON);
        DispelChargesList dispelList;
        target->GetDispellableAuraList(caster, dispelMask, dispelList);
        if (dispelList.empty())
            return;

        float const range = GetSpellInfo()->GetEffect(EFFECT_0).CalcValue(caster);

        for (DispelableAura& dispelEntry : dispelList)
        {
            Aura* aura = dispelEntry.GetAura();
            if (!aura)
                continue;

            Unit* origCaster = aura->GetCaster();
            if (origCaster && caster->GetDistance(origCaster) <= range)
            {
                if (Aura* reflected = origCaster->AddAura(aura->GetSpellInfo()->Id, origCaster))
                {
                    reflected->SetStackAmount(aura->GetStackAmount());
                    reflected->SetDuration(aura->GetDuration());
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        if (AuraEffect* dst = reflected->GetEffect(i))
                            if (AuraEffect const* src = aura->GetEffect(i))
                            {
                                dst->SetAmount(src->GetAmount());
                                dst->SetPeriodicTimer(src->GetPeriodicTimer());
                            }
                }
            }

            target->RemoveAurasDueToSpellByDispel(aura->GetId(), SPELL_DH_REVERSE_MAGIC, aura->GetCasterGUID(), caster);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dh_reverse_magic::HandleOnHit);
    }
};

// 206966 - Fel Lance (Honor Talent)
// Deals damage equal to a percentage of the target's maximum health.
class spell_dh_fel_lance : public SpellScript
{
    void HandleDamage(SpellEffIndex effIndex)
    {
        if (Unit* target = GetHitUnit())
            SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->GetEffect(effIndex).CalcValue(GetCaster())));
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dh_fel_lance::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 206650 - Eye of Leotheras (Honor Talent)
// Damages the target for a percentage of their max HP and refreshes the Eye of Leotheras mark.
class spell_dh_eye_of_leotheras : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_EYE_OF_LEOTHERAS_MARK });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        SpellInfo const* markInfo = sSpellMgr->GetSpellInfo(SPELL_DH_EYE_OF_LEOTHERAS_MARK, DIFFICULTY_NONE);
        if (!markInfo)
            return;

        SetHitDamage(target->CountPctFromMaxHealth(markInfo->GetEffect(EFFECT_0).CalcValue(GetCaster())));

        if (Aura* mark = target->GetAura(SPELL_DH_EYE_OF_LEOTHERAS_MARK))
            mark->SetDuration(mark->GetMaxDuration());
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dh_eye_of_leotheras::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 203704 - Mana Break (Honor Talent)
// Deals damage scaled by how much mana the target is missing (more missing mana = more damage).
class spell_dh_mana_break : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_3 } });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        int32 basePercent = GetSpellInfo()->GetEffect(EFFECT_0).CalcValue(GetCaster());

        if (target->GetMaxPower(POWER_MANA) > 0)
        {
            float missingManaPct = 100.f - target->GetPowerPct(POWER_MANA);
            basePercent += int32(missingManaPct * 0.2f);

            int32 const cap = GetSpellInfo()->GetEffect(EFFECT_3).CalcValue(GetCaster());
            if (basePercent > cap)
                basePercent = cap;
        }

        SetHitDamage(target->CountPctFromMaxHealth(basePercent));
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dh_mana_break::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 235904 - Mana Rift (Honor Talent)
// Deals max HP damage and burns mana proportional to the target's max mana pool.
class spell_dh_mana_rift : public SpellScript
{
    void HandleDamage(SpellEffIndex effIndex)
    {
        if (Unit* target = GetHitUnit())
            SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->GetEffect(effIndex).CalcValue(GetCaster())));
    }

    void HandleMana(SpellEffIndex effIndex)
    {
        if (Unit* target = GetHitUnit())
            SetEffectValue(CalculatePct(target->GetMaxPower(POWER_MANA), GetSpellInfo()->GetEffect(effIndex).CalcValue(GetCaster())));
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dh_mana_rift::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectLaunchTarget += SpellEffectFn(spell_dh_mana_rift::HandleMana, EFFECT_1, SPELL_EFFECT_POWER_BURN);
    }
};

// 203782 - Shear (Vengeance's basic Chi-generator): 20% chance on hit to spawn a Lesser Soul
// Fragment. Confirmed via two independent reference sources, which drive this off a separate always-on
// "Shear Proc" passive (203783) with its own proc-check against Shear's spell id - simplified
// here to a direct OnHit roll on Shear itself instead, avoiding a dependency on unconfirmed
// passive-granting data for 203783. Casts SPELL_DH_SHATTER_SOUL (209980), the Vengeance
// "lesser" fragment trigger - not currently used by the on-kill path below (which only uses
// the "big" _1/_2 variants) - routing through the already-existing spell_dh_shatter_soul
// AreaTrigger-spawn chain two-stages below.
class spell_dh_shear : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_SHATTER_SOUL });
    }

    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (roll_chance_i(20))
            caster->CastSpell(caster, SPELL_DH_SHATTER_SOUL, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dh_shear::HandleOnHit);
    }
};

// 209795 - Fracture (Vengeance's alternate builder): spawns 2 Soul Fragments directly, at
// higher cost/cooldown than Shear's chance-based 1. Confirmed via two independent reference sources
// (identical implementations - both use CastCustomSpell with SPELLVALUE_TRIGGER_SPELL against
// a shared "missile" spell, neither of which exists in ArgusCore; simplified here to casting
// SPELL_DH_SHATTER_SOUL directly twice, matching the already-established
// spell_dh_shattered_souls::OnProc pattern below instead).
class spell_dh_fracture : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_SHATTER_SOUL });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_DH_SHATTER_SOUL, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
        caster->CastSpell(caster, SPELL_DH_SHATTER_SOUL, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_fracture::HandleOnHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 178940 - Shattered Souls (Havoc passive)
// 204254 - Shattered Souls (Vengeance passive)
// On kill, spawns a soul fragment AreaTrigger at the enemy's location.
class spell_dh_shattered_souls : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            SPELL_DH_SHATTER_SOUL_HAVOC_NORMAL_CAST, SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_2,
            SPELL_DH_SHATTER_SOUL_2, SPELL_DH_SHATTER_SOUL_1
        });
    }

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo) const
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target)
            return;

        bool isDemon = target->GetCreatureType() == CREATURE_TYPE_DEMON;

        if (GetId() == SPELL_DH_SHATTERED_SOULS_HAVOC_KILL)
        {
            uint32 fragSpell = isDemon ? SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_2 : SPELL_DH_SHATTER_SOUL_HAVOC_NORMAL_CAST;
            caster->CastSpell(caster, fragSpell, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
        }
        else // Vengeance
        {
            uint32 fragSpell = isDemon ? SPELL_DH_SHATTER_SOUL_2 : SPELL_DH_SHATTER_SOUL_1;
            caster->CastSpell(caster, fragSpell, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dh_shattered_souls::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 209980 - Shattered Soul (lesser, Vengeance)  209981 - (big, Vengeance)
// 210038 - (big demon, Vengeance)  228533 - (lesser, Havoc)
// 237867 - (big demon, Havoc)      209651 - (big, Havoc)
// Spawns the matching soul fragment AreaTrigger at the spell destination.
class spell_dh_shatter_soul : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            SPELL_DH_SHATTERED_SOUL,
            SPELL_DH_SHATTER_SOUL_AT_HAVOC_LESSER,  SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_DEMON,
            SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_NORMAL, SPELL_DH_SHATTER_SOUL_AT_HAVOC_DEMON,
            SPELL_DH_SHATTER_SOUL_AT_HAVOC_NORMAL
        });
    }

    void HandleHitTarget(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        WorldLocation const* loc = GetExplTargetDest();
        if (!caster || !loc)
            return;

        uint32 atSpell = 0;
        switch (GetSpellInfo()->Id)
        {
            case SPELL_DH_SHATTER_SOUL:                           atSpell = SPELL_DH_SHATTERED_SOUL;                  break; // lesser Vengeance
            case SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_1: atSpell = SPELL_DH_SHATTER_SOUL_AT_HAVOC_LESSER;    break; // lesser Havoc
            case SPELL_DH_SHATTER_SOUL_2:                         atSpell = SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_DEMON; break; // big demon Vengeance
            case SPELL_DH_SHATTER_SOUL_1:                         atSpell = SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_NORMAL;break; // big Vengeance
            case SPELL_DH_SHATTERED_SOUL_LESSER_SOUL_FRAGMENT_2: atSpell = SPELL_DH_SHATTER_SOUL_AT_HAVOC_DEMON;     break; // big demon Havoc
            case SPELL_DH_SHATTER_SOUL_HAVOC_NORMAL_CAST:         atSpell = SPELL_DH_SHATTER_SOUL_AT_HAVOC_NORMAL;   break; // big Havoc
            default: return;
        }

        caster->CastSpell(*loc, atSpell, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dh_shatter_soul::HandleHitTarget, EFFECT_1, SPELL_EFFECT_TRIGGER_MISSILE);
    }
};

// 247454 - Spirit Bomb (Vengeance AOE Soul Fragment consumer): consumes all nearby Soul
// Fragment AreaTriggers within range, dealing AOE damage scaled by the count consumed and
// applying Frailty (below) to everything hit. Confirmed via two independent reference sources
// (count-then-multi-cast shape) and structurally corroborated by a third source
// (count-then-scale-single-cast shape, reused here since it maps directly onto ArgusCore's own
// SetHitDamage idiom). Two engine APIs used by the references don't exist in ArgusCore:
// Unit::GetAreaTriggerListWithSpellIDInRange (no range-filtered overload here - translated to
// Unit::GetAreaTriggers(spellId) plus a manual GetDistance check, the same idiom already used
// by DK's Defile) and GetEffectInfo(EFFECT_0)->BasePoints/Effects[EFFECT_0]->BasePoints
// (SpellEffectInfo has no operator-> - use GetEffectInfo(EFFECT_0).CalcValue(caster) instead).
// Only scans the Vengeance-side fragment AreaTrigger ids (SPELL_DH_SHATTERED_SOUL/
// SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_NORMAL/_DEMON) - Havoc has its own separate fragment
// chain that this ability doesn't touch.
//
// The consumed count is handed to the separate damage spell (247455) via the caster's own
// SPELL_DH_SOUL_FRAGMENT_COUNTER stack count rather than a SPELLVALUE_BASE_POINT0 override -
// EFFECT_0 on 247455 is itself the SCHOOL_DAMAGE effect, so overriding its base points would
// have corrupted the intrinsic damage value instead of just carrying the multiplier across the
// two casts. TrinityCore's current master branch confirms SPELL_DH_SOUL_FRAGMENT_COUNTER
// (203981, already declared in ArgusCore but previously unused) is exactly this kind of
// Vengeance-side fragment-count stacking aura, though its full design maintains the stack
// automatically via AreaTrigger OnCreate/OnRemove hooks that don't exist in ArgusCore yet
// (a materially bigger undertaking, out of scope here) - reused narrowly here as a two-cast
// scratch value instead, since this script already computes the true count itself every cast.
class spell_dh_spirit_bomb : public SpellScript
{
    uint8 _fragmentsConsumed = 0;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            SPELL_DH_SPIRIT_BOMB_DAMAGE, SPELL_DH_SOUL_FRAGMENT_COUNTER, SPELL_DH_SHATTERED_SOUL,
            SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_NORMAL, SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_DEMON
        });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return SPELL_FAILED_DONT_REPORT;

        float range = GetEffectInfo(EFFECT_0).CalcValue(caster);

        _fragmentsConsumed = 0;
        for (uint32 fragSpellId : { SPELL_DH_SHATTERED_SOUL, SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_NORMAL, SPELL_DH_SHATTER_SOUL_AT_VENGEANCE_DEMON })
        {
            for (AreaTrigger* at : caster->GetAreaTriggers(fragSpellId))
            {
                if (caster->GetDistance(at) > range)
                    continue;

                at->Remove();
                ++_fragmentsConsumed;
            }
        }

        return _fragmentsConsumed ? SPELL_CAST_OK : SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
    }

    void HandleOnCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_DH_SOUL_FRAGMENT_COUNTER, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_AURA_STACK, int32(_fragmentsConsumed)));
        caster->CastSpell(caster, SPELL_DH_SPIRIT_BOMB_DAMAGE, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dh_spirit_bomb::CheckCast);
        OnCast += SpellCastFn(spell_dh_spirit_bomb::HandleOnCast);
    }
};

// 247455 - Spirit Bomb (damage): scales by the fragment count carried via the caster's
// SPELL_DH_SOUL_FRAGMENT_COUNTER stack amount (set by spell_dh_spirit_bomb above immediately
// before this cast), and applies Frailty to every target hit.
class spell_dh_spirit_bomb_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FRAILTY, SPELL_DH_SOUL_FRAGMENT_COUNTER });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (Aura const* counter = caster->GetAura(SPELL_DH_SOUL_FRAGMENT_COUNTER))
        {
            int32 fragmentsConsumed = counter->GetStackAmount();
            if (fragmentsConsumed > 1)
                SetHitDamage(GetHitDamage() * fragmentsConsumed);
        }

        caster->CastSpell(target, SPELL_DH_FRAILTY, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_spirit_bomb_damage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 224509 - Frailty (Spirit Bomb's debuff): whenever an affected enemy takes damage, heals
// whichever Demon Hunter applied Frailty to them for a percentage of that damage. Confirmed
// via two independent reference sources (identical implementations, modulo the CastCustomSpell
// translation below).
class aura_dh_frailty : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_SPIRIT_BOMB_HEAL });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Unit* caster = aurEff->GetCaster();
        DamageInfo const* damageInfo = eventInfo.GetDamageInfo();
        if (!caster || !damageInfo)
            return;

        int32 healAmount = CalculatePct(int32(damageInfo->GetDamage()), GetEffectInfo(EFFECT_0).CalcValue(caster));
        caster->CastSpell(caster, SPELL_DH_SPIRIT_BOMB_HEAL, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, healAmount));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(aura_dh_frailty::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 228477 - Soul Cleave (self-heal)
// NOTE: correction to an earlier version of this fix - this does NOT cover the damage effect;
// that's a separate script (spell_dh_soul_cleave_damage, below) which also handles actually
// spending the Pain, not this one. This class only computes the resource-scaled self-heal
// (Effect 3), which additionally benefits from how much Pain is currently banked, plus the
// Feast of Souls interaction.
// NOTE: the reference also consumes nearby Soul Fragment AreaTriggers from this same spell
// (Effect 0) using an old WORLD_TRIGGER-summon hack. ArgusCore's own kept fragment-spawning
// classes (spell_dh_shattered_souls/spell_dh_shatter_soul above) already use a cleaner, modern
// AreaTrigger-cast approach instead, but there's no corresponding "consume a nearby fragment"
// mechanism anywhere in this codebase yet to hook into - that's a broader missing system, not
// something specific to Soul Cleave, and wasn't invented here. Soul Cleave's own healing works
// without it; fragment consumption for Vengeance's resource loop remains a follow-up.
class spell_dh_soul_cleave : public SpellScript
{
    int32 _extraSpellCost = 0;

    bool Load() override
    {
        Unit* caster = GetCaster();
        if (!caster)
            return false;

        _extraSpellCost = std::min<int32>(caster->GetPower(POWER_PAIN), 600);
        return true;
    }

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_FEAST_OF_SOULS, SPELL_DH_FEAST_OF_SOULS_PERIODIC_HEAL });
    }

    void HandleHeal(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        int32 heal = int32(11.0f * caster->GetTotalAttackPowerValue(BASE_ATTACK));
        heal = caster->SpellHealingBonusDone(caster, GetSpellInfo(), heal, HEAL, GetEffectInfo(effIndex));
        heal = int32(heal * (float(_extraSpellCost) / 600.0f));

        SetHitHeal(heal);

        if (caster->HasAura(SPELL_DH_FEAST_OF_SOULS))
            caster->CastSpell(caster, SPELL_DH_FEAST_OF_SOULS_PERIODIC_HEAL, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_soul_cleave::HandleHeal, EFFECT_3, SPELL_EFFECT_HEAL);
    }
};

// 228478 - Soul Cleave (damage, triggered by 228477's own EFFECT_1 SPELL_EFFECT_TRIGGER_SPELL -
// confirmed correctly bound to 228478 in spell_script_names; this header previously mislabeled
// it as 228477 itself, whose real EFFECT_1 is TRIGGER_SPELL, not WEAPON_PERCENT_DAMAGE)
// Doubles the base weapon-percent damage, then scales the total by how much Pain is being spent
// (0-300, floored at 50% since spending zero Pain still deals half as much again as the base
// hit) - and is the one that actually spends the Pain, clearing Gluttony's buff in the process.
class spell_dh_soul_cleave_damage : public SpellScript
{
    int32 _extraSpellCost = 0;

    bool Load() override
    {
        Unit* caster = GetCaster();
        if (!caster)
            return false;

        _extraSpellCost = std::min<int32>(caster->GetPower(POWER_PAIN), 300);
        return true;
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 dmg = GetHitDamage() * 2;
        dmg = int32(dmg * ((float(_extraSpellCost) + 300.0f) / 600.0f));
        SetHitDamage(dmg);

        caster->SetPower(POWER_PAIN, caster->GetPower(POWER_PAIN) - _extraSpellCost);

        if (caster->HasAura(SPELL_DH_GLUTTONY_BUFF))
            caster->RemoveAurasDueToSpell(SPELL_DH_GLUTTONY_BUFF);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_soul_cleave_damage::HandleDamage, EFFECT_1, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// 238118 - Flaming Soul
// Extends Fiery Brand's duration on the target each time the aura procs.
class spell_dh_flaming_soul : public AuraScript
{
    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo) const
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target)
            return;

        uint32 brandId = caster->HasAura(SPELL_DH_BURNING_ALIVE) ? SPELL_DH_FIERY_BRAND_DEBUFF_RANK_2 : SPELL_DH_FIERY_BRAND_DEBUFF_RANK_1;
        if (Aura* brand = target->GetAura(brandId, caster->GetGUID()))
        {
            int32 newDur = brand->GetDuration() + aurEff->GetAmount();
            brand->SetDuration(newDur);

            // 212818 is the periodic damage component of Fiery Brand — extend it too.
            if (Aura* periodic = target->GetAura(212818, caster->GetGUID()))
                periodic->SetDuration(newDur);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dh_flaming_soul::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 213017 - Fueled by Pain
// Each time the Vengeance DH takes sufficient damage, extends (or starts) Metamorphosis.
class spell_dh_fueled_by_pain : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM });
    }

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo) const
    {
        Unit* caster = eventInfo.GetActor();
        if (!caster)
            return;

        int32 extension = aurEff->GetAmount() * IN_MILLISECONDS;
        if (AuraEffect const* lingering = caster->GetAuraEffect(SPELL_DH_FUELED_BY_PAIN_LINGERING_ORDEAL, EFFECT_0))
            extension += lingering->GetAmount();

        if (Aura* meta = caster->GetAura(SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM))
        {
            meta->SetDuration(meta->GetDuration() + extension, true);
        }
        else
        {
            caster->CastSpell(caster, SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                .SpellValueOverrides = { { SPELLVALUE_DURATION, extension } }
            });
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dh_fueled_by_pain::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 191427 - Metamorphosis
// Leaps to the target destination and, for Vengeance, briefly transforms; the actual buff aura
// (162264 Havoc / 187827 Vengeance) is applied by this spell's own DB2 effect data, not by this
// script - this only drives the leap itself and the Demon Reborn interaction.
class spell_dh_metamorphosis : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_METAMORPHOSIS });
    }

    void HandleLeap()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        if (WorldLocation const* dest = GetExplTargetDest())
            player->CastSpell(*dest, SPELL_DH_METAMORPHOSIS, true);

        if (player->HasAura(SPELL_DH_DEMON_REBORN))
        {
            player->GetSpellHistory()->ResetCooldown(SPELL_DH_CHAOS_NOVA, true);
            if (SpellInfo const* blur = sSpellMgr->GetSpellInfo(SPELL_DH_BLUR, DIFFICULTY_NONE))
                player->GetSpellHistory()->ResetCharges(blur->ChargeCategoryId);
            player->GetSpellHistory()->ResetCooldown(SPELL_DH_EYE_BEAM, true);
        }
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_dh_metamorphosis::HandleLeap);
    }
};

// 201453 - Metamorphosis (temporary immunity while leaping)
// Havoc gets a brief immunity window during the leap animation; Vengeance doesn't.
class spell_dh_metamorphosis_immunity : public SpellScript
{
    void PreventImmunity(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        if (caster && caster->HasAura(SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM))
            PreventHitEffect(effIndex);
    }

    // real 201453 EFFECT_0 (not EFFECT_1) is the APPLY_AURA effect carrying SCHOOL_IMMUNITY;
    // the second SpellEffectFn arg needs a SPELL_EFFECT_x effect-type value, not an aura type -
    // SPELL_AURA_SCHOOL_IMMUNITY's numeric value (39) coincidentally matched SPELL_EFFECT_LANGUAGE
    // instead of the real APPLY_AURA(6) effect type here.
    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_dh_metamorphosis_immunity::PreventImmunity, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        OnEffectHit += SpellEffectFn(spell_dh_metamorphosis_immunity::PreventImmunity, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 201453 - Metamorphosis (landing trigger)
// A previous pass claimed real 201453 EFFECT_1's aura type "doesn't exist anywhere in ArgusCore's
// engine" and left the landing burst unported - that was wrong. Real EFFECT_1 is aura 263, which
// is SPELL_AURA_DISABLE_CASTING_EXCEPT_ABILITIES in ArgusCore (the same restriction the reference
// calls SPELL_AURA_ALLOW_ONLY_ABILITY under a different name - it's implemented, via
// AuraEffect::HandleNoImmediateEffect / Spell::CheckCast, confirmed in SpellAuraEffects.cpp) and
// it's only active for the leap's duration, so its removal is exactly "the leap animation ended,
// caster has landed". Fires Havoc's impact burst (200166) there; Vengeance's own landing
// interaction (proccing Infernal Strike's impact, plus Sigil of Flame if Flame Crash is talented)
// isn't confirmed against real data here and is left unimplemented rather than guessed at.
class spell_dh_metamorphosis_immunity_AuraScript : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_METAMORPHOSIS_IMPACT_DAMAGE, SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM });
    }

    void HandleLanding(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
            return;

        Unit* caster = GetTarget();
        if (!caster || caster->HasAura(SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM))
            return;

        caster->CastSpell(caster, SPELL_DH_METAMORPHOSIS_IMPACT_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR
        });
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_dh_metamorphosis_immunity_AuraScript::HandleLanding, EFFECT_1, SPELL_AURA_DISABLE_CASTING_EXCEPT_ABILITIES, AURA_EFFECT_HANDLE_REAL);
    }
};

// 162264 - Metamorphosis (Havoc buffs)
// Adds Leech while transformed, boosted further if Soul Rending is talented.
// NOTE: real 162264 EFFECT_3 is SPELL_AURA_MOD_LEECH (443), confirmed matching this hook exactly -
// but SPELL_AURA_MOD_LEECH is bound to AuraEffect::HandleNULL in ArgusCore's own
// AuraEffectHandler[] array (SpellAuraEffects.cpp), i.e. genuinely unimplemented at the engine
// level. CalcAmount here computes the correct bonus, but nothing currently applies it - the Leech
// stat itself never changes. Left as-is rather than invent a lifesteal mechanism; needs a real
// HandleModLeech engine implementation, out of scope for a script-level fix. Same gap affects
// spell_dh_metamorphosis_buffs_veng below (EFFECT_2, same aura type).
class spell_dh_metamorphosis_buffs : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_METAMORPHOSIS_TRANSFORM });
    }

    void CalcLeech(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        amount += caster->HasAura(SPELL_DH_SOUL_RENDING) ? 100 : 30;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dh_metamorphosis_buffs::CalcLeech, EFFECT_3, SPELL_AURA_MOD_LEECH);
    }
};

// 187827 - Metamorphosis (Vengeance buffs)
// Adds Leech while transformed if Soul Rending is talented.
class spell_dh_metamorphosis_buffs_veng : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM });
    }

    void CalcLeech(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(SPELL_DH_SOUL_RENDING_VENGEANCE))
            amount += 50;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dh_metamorphosis_buffs_veng::CalcLeech, EFFECT_2, SPELL_AURA_MOD_LEECH);
    }
};

// 200166 - Metamorphosis (Havoc impact damage)
// Filters players from the AoE knockback — players hit by Metamorphosis receive a different spell.
class spell_dh_metamorphosis_impact : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DH_METAMORPHOSIS_IMPACT_PLAYER });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        targets.remove_if([caster, this](WorldObject* obj)
        {
            if (Player* plr = obj->ToPlayer())
            {
                caster->CastSpell(plr, SPELL_DH_METAMORPHOSIS_IMPACT_PLAYER,
                    TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
                return true;
            }
            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dh_metamorphosis_impact::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

void AddSC_demon_hunter_spell_scripts()
{
    RegisterSpellScript(spell_dh_annihilation);
    RegisterSpellScript(spell_dh_annihilation_damage);
    RegisterSpellScript(spell_dh_blade_turning);
    RegisterSpellScript(spell_dh_chaos_nova);
    RegisterSpellScript(spell_dh_chaos_strike);
    RegisterSpellScript(spell_dh_charred_warblades);
    RegisterSpellScript(spell_dh_darkness);
    RegisterSpellScriptWithArgs(spell_dh_demonic, "spell_dh_demonic_havoc", SPELL_DH_METAMORPHOSIS_TRANSFORM);
    RegisterSpellScriptWithArgs(spell_dh_demonic, "spell_dh_demonic_vengeance", SPELL_DH_METAMORPHOSIS_VENGEANCE_TRANSFORM);
    RegisterSpellScript(spell_dh_demonic_infusion);
    RegisterSpellScript(spell_dh_demon_blades);
    RegisterSpellScript(spell_dh_demon_spikes);
    RegisterSpellScript(spell_dh_demon_spikes_buff);
    RegisterSpellScript(spell_dh_vengeful_retreat);
    RegisterSpellScript(spell_dh_vengeful_retreat_trigger);
    RegisterSpellScript(spell_dh_razor_spikes);
    RegisterSpellScript(spell_dh_master_of_the_glaive);
    RegisterSpellScript(spell_dh_bloodlet);
    RegisterSpellScript(spell_dh_nether_bond);
    RegisterSpellScript(spell_dh_nether_bond_periodic);
    RegisterSpellScript(spell_dh_eye_beam);
    RegisterSpellScript(spell_dh_eye_beam_trigger);
    RegisterSpellScript(spell_dh_feast_of_souls);
    RegisterSpellScript(spell_dh_fel_devastation);
    RegisterSpellScript(spell_dh_fel_devastation_damage);
    RegisterSpellScript(spell_dh_fel_devastation_heal);
    RegisterSpellScript(spell_dh_fel_eruption);
    RegisterSpellScript(spell_dh_infernal_strike);
    RegisterSpellScript(spell_dh_infernal_strike_jump);
    RegisterSpellScript(spell_dh_illidans_grasp);
    RegisterSpellScript(spell_dh_fel_mastery);
    RegisterSpellScript(spell_dh_fel_rush);
    RegisterSpellScript(spell_dh_fel_rush_charge);
    RegisterSpellScript(spell_dh_felblade);
    RegisterSpellScript(spell_dh_felblade_charge);
    RegisterSpellScript(spell_dh_felblade_cooldown_reset_proc);
    RegisterSpellScript(spell_dh_fiery_brand);
    RegisterSpellScript(spell_dh_fiery_brand_absorb);
    RegisterSpellScript(spell_dh_consume_soul);
    RegisterSpellScript(spell_dh_demonic_appetite);
    RegisterSpellScript(spell_dh_fel_barrage);
    RegisterSpellScript(spell_dh_fel_barrage_damage);
    RegisterSpellScript(spell_dh_fel_barrage_proc);
    RegisterSpellScript(spell_dh_consume_soul_vengeance);
    RegisterSpellScript(spell_dh_burning_alive);
    RegisterSpellScript(spell_dh_soul_barrier);
    RegisterSpellScript(spell_dh_last_resort);
    RegisterSpellScript(spell_dh_immolation_aura);
    RegisterSpellScript(spell_dh_sigil_of_chains);

    RegisterAreaTriggerAI(areatrigger_dh_darkness);
    new GenericAreaTriggerEntityScript<areatrigger_dh_generic_sigil<SPELL_DH_SIGIL_OF_CHAINS_TARGET_SELECT, SPELL_DH_SIGIL_OF_CHAINS_VISUAL>>("areatrigger_dh_sigil_of_chains");
    new GenericAreaTriggerEntityScript<areatrigger_dh_generic_sigil<SPELL_DH_SIGIL_OF_FLAME_AOE, SPELL_DH_SIGIL_OF_FLAME_VISUAL>>("areatrigger_dh_sigil_of_flame");
    new GenericAreaTriggerEntityScript<areatrigger_dh_generic_sigil<SPELL_DH_SIGIL_OF_SILENCE_AOE>>("areatrigger_dh_sigil_of_silence");
    new GenericAreaTriggerEntityScript<areatrigger_dh_generic_sigil<SPELL_DH_SIGIL_OF_MISERY_AOE>>("areatrigger_dh_sigil_of_misery");

    // Havoc

    /* Spells & Auras */

    /* Auras */

    RegisterSpellScript(spell_dh_first_blood);

    /* AreaTrigger */

    /* Spells */

    RegisterSpellScript(spell_dh_blade_dance);
    RegisterSpellScript(spell_dh_blade_dance_damage);

    // Vengeance & Havoc

    RegisterSpellAndAuraScriptPair(spell_dh_glide, spell_dh_glide_AuraScript);
    RegisterSpellAndAuraScriptPair(spell_dh_fel_rush_dash, spell_dh_fel_rush_dash_AuraScript);
    RegisterSpellScript(spell_dh_glide_timer);

    RegisterSpellScript(spell_dh_sigil_of_misery_fear);
    RegisterSpellScript(spell_dh_anguish);
    RegisterSpellScript(spell_dh_anguish_damage);
    RegisterSpellScript(spell_dh_nemesis);
    RegisterSpellScript(spell_dh_desperate_instincts);
    RegisterSpellScript(spell_dh_reverse_magic);
    RegisterSpellScript(spell_dh_fel_lance);
    RegisterSpellScript(spell_dh_eye_of_leotheras);
    RegisterSpellScript(spell_dh_mana_break);
    RegisterSpellScript(spell_dh_mana_rift);
    RegisterSpellScript(spell_dh_shattered_souls);
    RegisterSpellScript(spell_dh_shatter_soul);
    RegisterSpellScript(spell_dh_shear);
    RegisterSpellScript(spell_dh_fracture);
    RegisterSpellScript(spell_dh_spirit_bomb);
    RegisterSpellScript(spell_dh_spirit_bomb_damage);
    RegisterSpellScript(aura_dh_frailty);
    RegisterSpellScript(spell_dh_soul_cleave);
    RegisterSpellScript(spell_dh_soul_cleave_damage);
    RegisterSpellScript(spell_dh_flaming_soul);
    RegisterSpellScript(spell_dh_fueled_by_pain);
    RegisterSpellScript(spell_dh_metamorphosis);
    RegisterSpellAndAuraScriptPair(spell_dh_metamorphosis_immunity, spell_dh_metamorphosis_immunity_AuraScript);
    RegisterSpellScript(spell_dh_metamorphosis_buffs);
    RegisterSpellScript(spell_dh_metamorphosis_buffs_veng);
    RegisterSpellScript(spell_dh_metamorphosis_impact);
}
