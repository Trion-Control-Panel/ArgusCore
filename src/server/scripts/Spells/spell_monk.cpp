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
#include "Group.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include <unordered_map>

enum MonkSpells
{
    SPELL_MONK_BLACKOUT_STRIKE                          = 205523,
    SPELL_MONK_BREATH_OF_FIRE                           = 115181,
    SPELL_MONK_BREATH_OF_FIRE_DOT                       = 123725,
    SPELL_MONK_DISABLE                                  = 116095,
    SPELL_MONK_DISABLE_ROOT                             = 116706,
    SPELL_MONK_ELUSIVE_BRAWLER                          = 195630,
    SPELL_MONK_EXPEL_HARM_DAMAGE                        = 115129,
    SPELL_MONK_MANA_TEA_STACKS                          = 115867,
    SPELL_MONK_MEDITATE_VISUAL                          = 124416,
    SPELL_MONK_PLUS_ONE_MANA_TEA                        = 123760,
    SPELL_MONK_TRANSCENDENCE_CLONE_TARGET                = 119051,
    // Transcendence spirit clone creature entry - confirmed from AshamaneCore's own dedicated
    // migration (2018_02_05_01_world_spell_transcendence.sql), not independently re-verified
    // beyond that. Guarded at every use site via sObjectMgr->GetCreatureTemplate() so the
    // scripts below simply no-op rather than assuming this blindly.
    NPC_MONK_TRANSCENDENCE_SPIRIT                        = 54569,
    SPELL_MONK_SEF                                       = 137639,
    SPELL_MONK_SEF_STORM_VISUAL                          = 138080,
    SPELL_MONK_SEF_FIRE_VISUAL                           = 138081,
    SPELL_MONK_SEF_EARTH_VISUAL                          = 138083,
    SPELL_MONK_SEF_CHARGE                                = 138104,
    SPELL_MONK_SEF_SUMMON_EARTH                          = 138121,
    SPELL_MONK_SEF_SUMMON_FIRE                           = 138123,
    SPELL_MONK_SEF_SUMMONS_STATS                         = 138130,
    // DB note: creature_template.ScriptName must be 'npc_monk_sef_spirit' for entries 69791
    // (Fire) and 69792 (Earth). Confirmed via DestinyCore/AshamaneCore.
    NPC_MONK_SEF_FIRE_SPIRIT                             = 69791,
    NPC_MONK_SEF_EARTH_SPIRIT                            = 69792,
    SPELL_MONK_BURST_OF_LIFE_TALENT                     = 399226,
    SPELL_MONK_BURST_OF_LIFE_HEAL                       = 399230,
    SPELL_MONK_CALMING_COALESCENCE                      = 388220,
    SPELL_MONK_CHI_BURST_DAMAGE                         = 148135,
    SPELL_MONK_CHI_BURST_HEAL                           = 130654,
    SPELL_MONK_CHI_TORPEDO_DAMAGE                       = 117993,
    SPELL_MONK_CHI_TORPEDO_HEAL                         = 124040,
    SPELL_MONK_CHI_WAVE_DAMAGE_MISSILE                  = 132467,
    SPELL_MONK_CHI_WAVE_HEAL                            = 132463,
    SPELL_MONK_CHI_WAVE_HEAL_MISSILE                    = 132464,
    SPELL_MONK_CHI_WAVE_TARGET_SELECTOR                 = 132466,
    SPELL_MONK_COMBAT_CONDITIONING                      = 128595,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHANNEL         = 117952,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_CHI_PROC        = 123333,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK       = 117962,
    SPELL_MONK_CRACKLING_JADE_LIGHTNING_KNOCKBACK_CD    = 117953,
    SPELL_MONK_DIZZYING_HAZE                            = 116330,
    SPELL_MONK_ENVELOPING_MIST                          = 124682,
    SPELL_MONK_ENVELOPING_MIST_HEAL                     = 132120,
    SPELL_MONK_ESSENCE_FONT_HEAL                        = 191840,
    SPELL_MONK_FISTS_OF_FURY                            = 113656,
    SPELL_MONK_FISTS_OF_FURY_DAMAGE                     = 117418,
    SPELL_MONK_FISTS_OF_FURY_VISUAL                     = 123154,
    SPELL_MONK_FLYING_SERPENT_KICK                      = 101545,
    SPELL_MONK_FLYING_SERPENT_KICK_AOE                  = 123586,
    SPELL_MONK_FLYING_SERPENT_KICK_NEW                  = 115057,
    SPELL_MONK_FORTIFYING_BREW                          = 120954,
    SPELL_MONK_GIFT_OF_THE_OX_AURA                       = 124502,
    SPELL_MONK_GIFT_OF_THE_OX_AT_RIGHT                   = 124503,
    SPELL_MONK_GIFT_OF_THE_OX_AT_LEFT                    = 124506,
    SPELL_MONK_GIFT_OF_THE_OX_HEAL                       = 178173,
    SPELL_MONK_HEALING_ELIXIRS_RESTORE_HEALTH            = 122281,
    SPELL_MONK_HEALING_SPHERE_COOLDOWN                   = 224863,
    SPELL_MONK_HIT_COMBO                                = 196740,
    SPELL_MONK_HIT_COMBO_AURA                           = 196741,
    SPELL_MONK_ITEM_PVP_GLOVES_BONUS                    = 124489,
    SPELL_MONK_JADE_WALK                                = 450552,
    SPELL_MONK_KEG_SMASH_AURA                           = 121253,
    SPELL_MONK_KEG_SMASH_ENERGIZE                       = 127796,
    SPELL_MONK_KEG_SMASH_VISUAL                         = 123662,
    SPELL_MONK_LEGACY_OF_THE_EMPEROR                    = 117667,
    SPELL_MONK_LIFECYCLES_ENVELOPING_MIST               = 197919,
    SPELL_MONK_LIFECYCLES_VIVIFY                        = 197916,
    SPELL_MONK_MASTERY_COMBO_STRIKES                    = 115636,
    SPELL_MONK_MISTS_OF_LIFE                            = 388548,
    SPELL_MONK_MORTAL_WOUNDS                            = 115804,
    SPELL_MONK_POWER_STRIKE_PROC                        = 129914,
    SPELL_MONK_POWER_STRIKE_ENERGIZE                    = 121283,
    SPELL_MONK_PRESSURE_POINTS                          = 450432,
    SPELL_MONK_PROVOKE_SINGLE_TARGET                    = 116189,
    SPELL_MONK_PROVOKE_AOE                              = 118635,
    SPELL_MONK_NO_FEATHER_FALL                          = 79636,
    SPELL_MONK_OPEN_PALM_STRIKES_TALENT                 = 392970,
    SPELL_MONK_PURIFYING_BREW                           = 119582,
    SPELL_MONK_RENEWING_MIST                            = 119611,
    SPELL_MONK_RING_OF_PEACE_DISARM                     = 137461,
    SPELL_MONK_RING_OF_PEACE_SILENCE                    = 137460,
    SPELL_MONK_RISING_SUN_KICK                          = 107428,
    SPELL_MONK_RISING_THUNDER                           = 210804,
    SPELL_MONK_ROLL_BACKWARD                            = 109131,
    SPELL_MONK_ROLL_FORWARD                             = 107427,
    SPELL_MONK_SAVE_THEM_ALL_HEAL_BONUS                 = 390105,
    SPELL_MONK_SONG_OF_CHI_JI_STUN                      = 198909,
    SPELL_MONK_SOOTHING_MIST                            = 115175,
    SPELL_MONK_SPEAR_HAND_STRIKE_SILENCE                = 116709,
    SPELL_MONK_SPINNING_CRANE_KICK                      = 101546,
    SPELL_MONK_SPINNING_CRANE_KICK_DAMAGE               = 107270,
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
    SPELL_MONK_TIGER_PALM                               = 100780,
    SPELL_MONK_TOUCH_OF_DEATH                           = 115080,
    SPELL_MONK_TOUCH_OF_KARMA_REDIRECT_DAMAGE           = 124280,
    SPELL_MONK_VIVIFY                                   = 116670,
    SPELL_MONK_WEAKENED_BLOWS                           = 115798,
    SPELL_MONK_WHIRLING_DRAGON_PUNCH                    = 152175,
    SPELL_MONK_WHIRLING_DRAGON_PUNCH_DAMAGE             = 158221,
    SPELL_MONK_ZEN_PILGRIMAGE                           = 126892,
    SPELL_MONK_ZEN_PILGRIMAGE_RETURN                    = 126895,
    SPELL_MONK_ZEN_PILGRIMAGE_RETURN_AURA               = 126896,
    SPELL_MONK_ZEN_PULSE_HEAL                           = 198487,
};

// 100784 - Blackout Kick
// Windwalker/Mistweaver builder-consumer interaction with Teachings of the Monastery: Tiger
// Palm grants stacks of a buff (202090) that Blackout Kick consumes for bonus damage, and
// separately has a chance to reset Rising Sun Kick's cooldown/charges if the talent (116645)
// is known. Mistweavers with Spirit of the Crane (210802) refund mana based on stacks consumed.
// NOTE: the reference implementation re-deals the hit's damage once per stack via a manually
// constructed SpellNonMeleeDamage (producing N separate combat-log entries) - that low-level
// damage-dealing pattern has no precedent anywhere in ArgusCore's engine and couldn't be
// verified to behave correctly here, so this instead multiplies the single hit's damage by
// (stacks + 1), which delivers the same total damage as one combat-log entry rather than N.
// The RSK-reset percentage (15%) is the reference's own value, not independently verified
// against Legion 7.3.5 client data - flagged the same way Tactician's proc-chance was earlier.

// 115399 - Black Ox Brew
// Resets Purifying Brew's charges.
class spell_monk_black_ox_brew : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_PURIFYING_BREW });
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (SpellInfo const* purifyingBrew = sSpellMgr->GetSpellInfo(SPELL_MONK_PURIFYING_BREW, DIFFICULTY_NONE))
            caster->GetSpellHistory()->ResetCharges(purifyingBrew->ChargeCategoryId);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_black_ox_brew::HandleHit, EFFECT_0, SPELL_EFFECT_ENERGIZE);
    }
};

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

// 116095 - Disable (baseline snare): upgrades to a root effect if the target is already
// snared; the resulting snare/root refreshes its own duration on the caster's subsequent
// melee hits. Confirmed via DestinyCore and AshamaneCore (identical implementations).
class spell_monk_disable : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_DISABLE, SPELL_MONK_DISABLE_ROOT });
    }

    void HandleOnEffectHitTarget(SpellEffIndex /*effectIndex*/)
    {
        if (Unit* target = GetExplTargetUnit())
            if (target->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED))
                GetCaster()->CastSpell(target, SPELL_MONK_DISABLE_ROOT, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_disable::HandleOnEffectHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

class aura_monk_disable : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo)
            return false;

        if ((damageInfo->GetAttackType() == BASE_ATTACK || damageInfo->GetAttackType() == OFF_ATTACK)
            && damageInfo->GetAttacker() == GetCaster())
        {
            GetAura()->RefreshDuration();
            return true;
        }

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_monk_disable::CheckProc);
    }
};

// 115181 - Breath of Fire
// Applies a burning DoT if the target already has Dizzying Haze or is showing Keg Smash's own
// DB2-driven debuff aura (SPELL_MONK_KEG_SMASH_AURA shares the same id as Keg Smash's own outer
// spell, 121253 - it's whatever aura Keg Smash's own client-defined effects leave on the
// target, not something this script or spell_monk_keg_smash needs to cast separately).
class spell_monk_breath_of_fire : public SpellScript
{
    void HandleAfterHit()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (target->HasAura(SPELL_MONK_DIZZYING_HAZE) || target->HasAura(SPELL_MONK_KEG_SMASH_AURA))
            caster->CastSpell(target, SPELL_MONK_BREATH_OF_FIRE_DOT, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_monk_breath_of_fire::HandleAfterHit);
    }
};

// 117906 - Mastery: Elusive Brawler. Grants a stack of the dodge-chance buff (195630) on
// Blackout Strike/Breath of Fire hits and on being hit. Confirmed via DestinyCore and
// AshamaneCore (identical implementations).
class spell_monk_elusive_brawler_mastery : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK)
            return true;

        return eventInfo.GetProcSpell() &&
              (eventInfo.GetProcSpell()->GetSpellInfo()->Id == SPELL_MONK_BLACKOUT_STRIKE ||
               eventInfo.GetProcSpell()->GetSpellInfo()->Id == SPELL_MONK_BREATH_OF_FIRE);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_monk_elusive_brawler_mastery::CheckProc);
    }
};

// 195630 - Elusive Brawler: the stacking dodge-chance buff itself. Consumed (duration zeroed,
// letting the normal expiry pipeline remove it) on a successful dodge. Confirmed via
// DestinyCore and AshamaneCore (identical implementations).
class spell_monk_elusive_brawler_stacks : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (!(eventInfo.GetHitMask() & PROC_HIT_DODGE))
            return false;

        if (Aura* elusiveBrawler = GetCaster()->GetAura(SPELL_MONK_ELUSIVE_BRAWLER, GetCaster()->GetGUID()))
            elusiveBrawler->SetDuration(0);

        return true;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_monk_elusive_brawler_stacks::CheckProc);
    }
};

// 115072 - Expel Harm (baseline, level 26): heals the caster and discharges half the amount
// healed as damage to nearby attackable enemies. Confirmed via DestinyCore and AshamaneCore
// (identical implementations). Id corrected from 322101 (this doc's originally-recorded id,
// which is a much later/modern-retail id per Wowhead) to 115072, the id both reference cores
// agree on and the one that has existed since Mists of Pandaria through Legion.
// Player::GetAttackableUnitListInRange doesn't exist in ArgusCore - translated to the
// established Trinity::AnyUnfriendlyUnitInObjectRangeCheck/UnitListSearcher/
// Cell::VisitAllObjects idiom already used elsewhere in this codebase (e.g. spell_warlock.cpp).
class spell_monk_expel_harm : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_EXPEL_HARM_DAMAGE });
    }

    void HandleOnHit()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        std::list<Unit*> enemies;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck checker(caster, caster, 10.0f);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(caster, enemies, checker);
        Cell::VisitAllObjects(caster, searcher, 10.0f);

        int32 bp = CalculatePct(-GetHitDamage(), 50);
        for (Unit* enemy : enemies)
            if (caster->IsValidAttackTarget(enemy))
                caster->CastSpell(enemy, SPELL_MONK_EXPEL_HARM_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, bp));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_monk_expel_harm::HandleOnHit);
    }
};

// 115294 - Mana Tea (channel): duration scales with the caster's current Mana Tea stack
// count (1 sec channeled per stack), consuming 1 stack per tick rather than the whole stack
// at once (so cancelling early doesn't waste unconsumed stacks). Confirmed via DestinyCore and
// AshamaneCore (identical implementations), but both drive the variable duration by injecting
// a raw SpellModifier with hand-set flag128 mask bits before the cast - the same pre-refactor
// pattern this project's CLAUDE.md already flags as unsafe to guess (see the Fire Mage
// Passive precedent). Reused this session's own established alternative instead: apply the
// aura at its default duration, then adjust it directly via Aura::SetDuration once applied -
// the same idiom already used for DK's Bonestorm and Mage's Thermal Void elsewhere in this
// project.
class spell_monk_mana_tea : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_MANA_TEA_STACKS });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Aura* stacks = caster->GetAura(SPELL_MONK_MANA_TEA_STACKS))
            GetAura()->SetDuration(stacks->GetStackAmount() * IN_MILLISECONDS);
    }

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Aura* stacks = caster->GetAura(SPELL_MONK_MANA_TEA_STACKS))
        {
            if (stacks->GetStackAmount() > 1)
                stacks->SetStackAmount(stacks->GetStackAmount() - 1);
            else
                caster->RemoveAura(SPELL_MONK_MANA_TEA_STACKS);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_monk_mana_tea::OnApply, EFFECT_0, SPELL_AURA_OBS_MOD_POWER, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_mana_tea::OnTick, EFFECT_0, SPELL_AURA_OBS_MOD_POWER);
    }
};

// 123766 - Brewing: Mana Tea (Mistweaver passive): every 4 Chi spent grants 1 Mana Tea stack
// (capped at 20 by the buff's own DB2 MaxStack). The reference drives this via
// AuraEffect::SetData, a legacy virtual-callback shape with no equivalent call site in
// ArgusCore. Uses the same generic "any Chi-costing spell" proc idiom already established for
// DK's Runic Empowerment/Blood Charge instead.
class spell_monk_mana_tea_stacks : public AuraScript
{
    int32 _chiSpent = 0;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_MANA_TEA_STACKS, SPELL_MONK_PLUS_ONE_MANA_TEA });
    }

    static bool CheckProc(AuraScript const&, AuraEffect const* /*aurEff*/, ProcEventInfo const& procEvent)
    {
        Spell const* procSpell = procEvent.GetProcSpell();
        return procSpell && procSpell->GetPowerTypeCostAmount(POWER_CHI) > 0;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& procInfo)
    {
        _chiSpent += *procInfo.GetProcSpell()->GetPowerTypeCostAmount(POWER_CHI);
        while (_chiSpent >= 4)
        {
            _chiSpent -= 4;
            GetTarget()->CastSpell(GetTarget(), SPELL_MONK_MANA_TEA_STACKS, true);
            GetTarget()->CastSpell(GetTarget(), SPELL_MONK_PLUS_ONE_MANA_TEA, true);
        }
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_monk_mana_tea_stacks::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_monk_mana_tea_stacks::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 101643 - Transcendence: summons a spirit clone, positions swappable with the caster via
// Transcendence: Transfer (119996) below. Confirmed via DestinyCore/AshamaneCore, but
// redesigned: neither Unit::OnEffectSummon (the hook the references use) nor Object::Variables
// (used to cache the spirit's GUID across hooks) exist anywhere in ArgusCore. Uses this
// project's own established idioms instead - GetSpell()->GetExecuteLogEffectTargets(
// SPELL_EFFECT_SUMMON, ...) to find what was just summoned (matching
// spell_pal_light_hammer_init_summon in spell_paladin.cpp), and Unit::m_Controlled to find the
// spirit again later rather than caching its GUID (matching Warlock's Implosion/DK's Dancing
// Rune Weapon lookups elsewhere this session). Guarded via Validate() checking
// sObjectMgr->GetCreatureTemplate() first.
class spell_monk_transcendence : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sObjectMgr->GetCreatureTemplate(NPC_MONK_TRANSCENDENCE_SPIRIT))
            return false;
        return ValidateSpellInfo({ SPELL_MONK_TRANSCENDENCE_CLONE_TARGET, SPELL_MONK_MEDITATE_VISUAL });
    }

    // Only one spirit at a time - despawn any existing one before the new one is summoned.
    void DespawnPrevious()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        for (Unit* controlled : caster->m_Controlled)
            if (controlled->GetEntry() == NPC_MONK_TRANSCENDENCE_SPIRIT)
                if (Creature* oldSpirit = controlled->ToCreature())
                    oldSpirit->DespawnOrUnsummon();
    }

    void HandleSummon()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        for (SpellLogEffectGenericVictimParams const& summoned : GetSpell()->GetExecuteLogEffectTargets(SPELL_EFFECT_SUMMON, &SpellLogEffect::GenericVictimTargets))
        {
            if (Unit* spirit = ObjectAccessor::GetUnit(*caster, summoned.Victim))
            {
                caster->CastSpell(spirit, SPELL_MONK_TRANSCENDENCE_CLONE_TARGET, true);
                spirit->CastSpell(spirit, SPELL_MONK_MEDITATE_VISUAL, true);
            }
        }
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_monk_transcendence::DespawnPrevious);
        AfterCast += SpellCastFn(spell_monk_transcendence::HandleSummon);
    }
};

// 119996 - Transcendence: Transfer: swaps the caster and their Transcendence spirit's
// positions, if the spirit is within range. Confirmed via DestinyCore/AshamaneCore.
class spell_monk_transcendence_transfer : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return sObjectMgr->GetCreatureTemplate(NPC_MONK_TRANSCENDENCE_SPIRIT) != nullptr;
    }

    Creature* FindSpirit() const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return nullptr;

        for (Unit* controlled : caster->m_Controlled)
            if (controlled->GetEntry() == NPC_MONK_TRANSCENDENCE_SPIRIT)
                if (Creature* spirit = controlled->ToCreature())
                    return spirit;

        return nullptr;
    }

    SpellCastResult CheckCast()
    {
        Creature* spirit = FindSpirit();
        if (!spirit)
            return SPELL_FAILED_NO_PET;

        if (!spirit->IsWithinDist(GetCaster(), GetSpellInfo()->GetMaxRange(true, GetCaster(), GetSpell())))
            return SPELL_FAILED_OUT_OF_RANGE;

        return SPELL_CAST_OK;
    }

    void HandleOnCast()
    {
        Unit* caster = GetCaster();
        Creature* spirit = FindSpirit();
        if (!caster || !spirit)
            return;

        Position casterPos = caster->GetPosition();
        Position spiritPos = spirit->GetPosition();
        caster->NearTeleportTo(spiritPos, true);
        spirit->NearTeleportTo(casterPos, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_monk_transcendence_transfer::CheckCast);
        OnCast += SpellCastFn(spell_monk_transcendence_transfer::HandleOnCast);
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

// 130654 - Chi Burst (heal)
// The heal payload needs its own attack-power-scaled formula rather than relying on the base
// spell data's default scaling - ported the reference's own coefficient (4.125x attack power)
// verbatim, not independently re-derived.
class spell_monk_chi_burst_heal : public SpellScript
{
    void HandleHeal(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        SpellInfo const* spellInfo = GetSpellInfo();
        SpellEffectInfo const& effectInfo = spellInfo->GetEffect(EFFECT_0);

        int32 healing = int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 4.125f);
        healing = caster->SpellDamageBonusDone(target, spellInfo, healing, HEAL, effectInfo);
        healing = target->SpellDamageBonusTaken(caster, spellInfo, healing, HEAL);

        SetHitHeal(healing);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_chi_burst_heal::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// 115008 - Chi Torpedo
// Roll replacement talent: while rolling forward, damages enemies and heals allies (including
// the Monk) caught in a 60-degree cone in front of the Monk within 20 yards.
class spell_monk_chi_torpedo : public SpellScript
{
    void HandleAfterCast()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        std::list<Unit*> targets;
        Trinity::AnyUnitInObjectRangeCheck check(caster, 20.0f);
        Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(caster, targets, check);
        Cell::VisitAllObjects(caster, searcher, 20.0f);

        for (Unit* target : targets)
        {
            if (target->GetGUID() != caster->GetGUID() && !target->isInFront(caster, float(M_PI / 3)))
                continue;

            uint32 spellId = caster->IsValidAttackTarget(target) ? SPELL_MONK_CHI_TORPEDO_DAMAGE : SPELL_MONK_CHI_TORPEDO_HEAL;
            caster->CastSpell(target, spellId, true);
        }

        if (caster->HasAura(SPELL_MONK_ITEM_PVP_GLOVES_BONUS))
            caster->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_monk_chi_torpedo::HandleAfterCast);
    }
};

// 115098 - Chi Wave
// Fires a bouncing dummy missile that alternates between healing the lowest-health ally and
// damaging the nearest enemy, chaining through the 132466 target-selector spell up to
// GetEffectValue() times before it runs out of bounces.
class spell_monk_chi_wave : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        CastSpellExtraArgs args(SPELLVALUE_BASE_POINT1, GetEffectValue());
        args.SetTriggerFlags(TRIGGERED_FULL_MASK);

        if (caster->IsFriendlyTo(target))
            caster->CastSpell(target, SPELL_MONK_CHI_WAVE_HEAL_MISSILE, args);
        else if (caster->IsValidAttackTarget(target))
            caster->CastSpell(target, SPELL_MONK_CHI_WAVE_DAMAGE_MISSILE, args);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_chi_wave::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 132467 - Chi Wave (damage missile)
// The direct-damage effect on this spell is handled entirely by its own DB2-defined effect;
// this only continues the bounce chain once the missile's marker aura wears off.
class spell_monk_chi_wave_damage_missile : public AuraScript
{
    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        CastSpellExtraArgs args(SPELLVALUE_BASE_POINT1, aurEff->GetAmount() - 1);
        args.SetTriggerFlags(TRIGGERED_FULL_MASK);
        args.SetTriggeringAura(aurEff);
        caster->CastSpell(target, SPELL_MONK_CHI_WAVE_TARGET_SELECTOR, args);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_monk_chi_wave_damage_missile::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 132464 - Chi Wave (heal missile)
class spell_monk_chi_wave_heal_missile : public AuraScript
{
    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_MONK_CHI_WAVE_HEAL, true);

        CastSpellExtraArgs args(SPELLVALUE_BASE_POINT1, aurEff->GetAmount() - 1);
        args.SetTriggerFlags(TRIGGERED_FULL_MASK);
        args.SetTriggeringAura(aurEff);
        caster->CastSpell(target, SPELL_MONK_CHI_WAVE_TARGET_SELECTOR, args);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_monk_chi_wave_heal_missile::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

namespace
{
    // Narrows an area-search target list down to the single nearest valid attack target within
    // range, by mutating its own search radius as it walks the list (matches remove_if's
    // guaranteed single left-to-right pass over the sequence).
    class ChiWaveDamageTargetCheck
    {
    public:
        ChiWaveDamageTargetCheck(Unit const* source, float range) : _source(source), _range(range) { }

        bool operator()(WorldObject* object)
        {
            Unit* unit = object->ToUnit();
            if (!unit)
                return true;

            if (_source->IsValidAttackTarget(unit) && unit->isTargetableForAttack() && _source->IsWithinDistInMap(unit, _range))
            {
                _range = _source->GetDistance(unit);
                return false;
            }

            return true;
        }

    private:
        Unit const* _source;
        float _range;
    };

    class ChiWaveHealTargetCheck
    {
    public:
        ChiWaveHealTargetCheck(Unit const* source) : _source(source) { }

        bool operator()(WorldObject* object)
        {
            Unit* unit = object->ToUnit();
            if (!unit)
                return true;

            return !_source->IsFriendlyTo(unit);
        }

    private:
        Unit const* _source;
    };
}

// 132466 - Chi Wave (target selector)
// After a heal missile lands, the next hop searches for the nearest valid enemy within 25
// yards; after a damage missile lands, the next hop searches for the lowest-health ally in the
// original search radius. The relay unit (GetExplTargetUnit(), the spot the missile just
// landed on) performs the next hop's cast so the travel animation visually chains from body to
// body, while GetOriginalCaster() is threaded through explicitly so scaling/attribution always
// stays on the real caster regardless of how many bodies the missile has bounced off of.
class spell_monk_chi_wave_target_selector : public SpellScript
{
    void SelectTarget(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        SpellInfo const* triggeringSpell = GetTriggeringSpell();
        if (!triggeringSpell)
            return;

        if (triggeringSpell->Id == SPELL_MONK_CHI_WAVE_DAMAGE_MISSILE)
        {
            targets.remove_if(ChiWaveHealTargetCheck(GetCaster()));
            targets.sort(Trinity::Predicates::HealthPctOrderPred(false));
            _shouldHeal = true;
        }
        else if (triggeringSpell->Id == SPELL_MONK_CHI_WAVE_HEAL_MISSILE)
        {
            targets.remove_if(ChiWaveDamageTargetCheck(GetCaster(), 25.0f));
            _shouldHeal = false;
        }
        else
            return;

        if (targets.empty())
            return;

        WorldObject* target = targets.back();
        targets.clear();
        targets.push_back(target);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (!GetEffectValue())
            return;

        Unit* relay = GetExplTargetUnit();
        Unit* target = GetHitUnit();
        Unit* originalCaster = GetOriginalCaster();
        if (!relay || !target || !originalCaster)
            return;

        CastSpellExtraArgs args(SPELLVALUE_BASE_POINT1, GetEffectValue());
        args.SetTriggerFlags(TRIGGERED_FULL_MASK);
        args.SetOriginalCaster(originalCaster->GetGUID());
        relay->CastSpell(target, _shouldHeal ? SPELL_MONK_CHI_WAVE_HEAL_MISSILE : SPELL_MONK_CHI_WAVE_DAMAGE_MISSILE, args);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_chi_wave_target_selector::SelectTarget, EFFECT_1, TARGET_UNIT_DEST_AREA_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_monk_chi_wave_target_selector::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }

    bool _shouldHeal = true;
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
// NOTE: the reference implementation computes the absorb as
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

// 191840 - Essence Font (heal)
// Redistributes the heal to whichever nearby ally most needs it: excludes the caster and
// anyone already freshly affected (aura still has more than 5 sec remaining, meaning it was
// applied less than 1 sec ago given the spell's own duration), then picks the single lowest-
// health remaining candidate.
class spell_monk_essence_font_heal : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        targets.remove_if([caster](WorldObject* object)
        {
            Unit* unit = object ? object->ToUnit() : nullptr;
            if (!unit || unit == caster)
                return true;

            Aura* existing = unit->GetAura(SPELL_MONK_ESSENCE_FONT_HEAL);
            return existing && existing->GetDuration() > 5 * IN_MILLISECONDS;
        });

        if (targets.size() > 1)
        {
            targets.sort(Trinity::Predicates::HealthPctOrderPred());
            targets.resize(1);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_essence_font_heal::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_essence_font_heal::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
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
// coefficient itself is the reference implementation's own value, not independently verified
// against Legion 7.3.5 client data.
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
// workaround from a reference implementation, not a guessed value; the actual channel length
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

// 115057 - Flying Serpent Kick
// Cleans up a legacy/superseded aura (101545) if present - the same "id renamed mid-expansion"
// pattern already seen with Bladestorm earlier this session - removes a PvP-glove-item-driven
// slow if the relevant set bonus is active, then triggers the AoE damage/knockback sub-spell
// (123586, the same id Mastery: Combo Strikes already tracks bonus damage for).
class spell_monk_flying_serpent_kick : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_FLYING_SERPENT_KICK_NEW });
    }

    void HandleOnCast()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        if (caster->HasAura(SPELL_MONK_FLYING_SERPENT_KICK))
            caster->RemoveAura(SPELL_MONK_FLYING_SERPENT_KICK);

        if (caster->HasAura(SPELL_MONK_ITEM_PVP_GLOVES_BONUS))
            caster->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);

        caster->CastSpell(caster, SPELL_MONK_FLYING_SERPENT_KICK_AOE, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_monk_flying_serpent_kick::HandleOnCast);
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

// 121253 - Keg Smash
// Brewmaster's core AoE ability: applies a visual, the Weakened Blows debuff (shared with
// Warrior's Thunder Clap - same spell id, 115798), energizes 2 Chi, and applies Dizzying Haze
// (a slow/threat debuff). The 1-second internal cooldown on the energize prevents multi-target
// cleave hits from granting more than one application of Chi per cast.
class spell_monk_keg_smash : public SpellScript
{
    void HandleOnHit()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_MONK_KEG_SMASH_VISUAL, true);
        caster->CastSpell(target, SPELL_MONK_WEAKENED_BLOWS, true);
        caster->CastSpell(caster, SPELL_MONK_KEG_SMASH_ENERGIZE, true);
        caster->GetSpellHistory()->AddCooldown(SPELL_MONK_KEG_SMASH_ENERGIZE, 0, std::chrono::seconds(1));
        caster->CastSpell(target, SPELL_MONK_DIZZYING_HAZE, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_monk_keg_smash::HandleOnHit);
    }
};

// 115921 - Legacy of the Emperor
// Applies the raid buff to all party members.
// NOTE: the reference implementation uses Player::GetPartyMembers(), which doesn't exist in
// ArgusCore - iterates the caster's Group directly instead (Group::GetMembers(), the standard
// pattern already used internally by Group::BroadcastWorker in this engine).
class spell_monk_legacy_of_the_emperor : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        Group* group = caster->GetGroup();
        if (!group)
            return;

        for (GroupReference const& itr : group->GetMembers())
            if (Player* member = itr.GetSource())
                caster->CastSpell(member, SPELL_MONK_LEGACY_OF_THE_EMPEROR, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_legacy_of_the_emperor::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
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

// 197915 - Lifecycles
// Passive: casting Vivify grants a buff reducing the mana cost of the next Enveloping Mist,
// and casting Enveloping Mist grants the reverse - rewards alternating between the two heals
// instead of spamming either one. The mana-cost-reduction itself lives entirely in the
// granted buff's own DB2 aura data; this only needs to apply the correct buff on proc.
class spell_monk_lifecycles : public AuraScript
{
    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!caster || !procSpell)
            return;

        if (procSpell->Id == SPELL_MONK_VIVIFY)
            caster->CastSpell(caster, SPELL_MONK_LIFECYCLES_ENVELOPING_MIST, true);
        else if (procSpell->Id == SPELL_MONK_ENVELOPING_MIST_HEAL)
            caster->CastSpell(caster, SPELL_MONK_LIFECYCLES_VIVIFY, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_lifecycles::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
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

// 140023 - Ring of Peace
// Proc-driven: applies a silence and a disarm to whoever triggers the effect.
class spell_monk_ring_of_peace_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_RING_OF_PEACE_SILENCE, SPELL_MONK_RING_OF_PEACE_DISARM });
    }

    void HandleDummyProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        caster->CastSpell(target, SPELL_MONK_RING_OF_PEACE_SILENCE, true);
        caster->CastSpell(target, SPELL_MONK_RING_OF_PEACE_DISARM, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_ring_of_peace_aura::HandleDummyProc, EFFECT_0, SPELL_AURA_DUMMY);
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

// 210804 - Rising Thunder
// A dedicated, self-contained proc-driven aura on the talent itself that also resets Thunder
// Focus Tea's cooldown, independent of whatever proc-eligible ability triggers it via this
// spell's own DB2 proc data.
// NOTE: this overlaps with the explicit HasAura(SPELL_MONK_RISING_THUNDER) check already added
// to spell_warr_rising_sun_kick... (spell_monk_rising_sun_kick) earlier this session, which
// hardcodes the same reset specifically on Rising Sun Kick hits. Kept both rather than removing
// the earlier one: resetting an already-ready cooldown is a harmless no-op, and it's not
// certain the two trigger on exactly the same conditions (this aura's own proc scope wasn't
// independently verified) - if RSK is the only real trigger, this is a harmless duplicate; if
// this aura's DB2 proc data covers other cases too, this fix adds real coverage the earlier one
// didn't have.
class spell_monk_rising_thunder : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_THUNDER_FOCUS_TEA });
    }

    void HandleEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (caster)
            caster->GetSpellHistory()->ResetCooldown(SPELL_MONK_THUNDER_FOCUS_TEA, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_rising_thunder::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
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

// Chi Burst (damage half) - spawned by 123986, AreaTriggerId 5302
// (areatrigger_create_properties.Id 1315 in the base world DB, ScriptName column empty until
// this binding - unlike Gift of the Ox's orb, this row's visual/shape/curve data already ships
// with the base database, so no unverifiable DB2 asset guessing was needed here).
struct at_monk_chi_burst_damage : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
            if (caster->IsValidAttackTarget(unit))
                caster->CastSpell(unit, SPELL_MONK_CHI_BURST_DAMAGE, true);
    }
};

// Chi Burst (heal half) - spawned by 123986, AreaTriggerId 5300
// (areatrigger_create_properties.Id 1316 - see at_monk_chi_burst_damage above).
struct at_monk_chi_burst_heal : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
            if (caster->IsValidAssistTarget(unit))
                caster->CastSpell(unit, SPELL_MONK_CHI_BURST_HEAL, true);
    }
};

// 124502 - Gift of the Ox
// Chance to spawn a healing sphere when taking damage, scaling with damage taken relative to
// max health and increasing as the Monk's own health drops.
// NOTE: the reference implementation implements this as a global PlayerScript::OnTakeDamage hook -
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

// 122280 - Healing Elixirs
// Proc-driven self-heal when damage taken drops the caster below 35% health, gated by a
// charge system (consuming one charge of the heal spell's own charge category per trigger).
class spell_monk_healing_elixirs_aura : public AuraScript
{
    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        DamageInfo const* dmgInfo = eventInfo.GetDamageInfo();
        if (!caster || !dmgInfo || !dmgInfo->GetDamage())
            return;

        if (!caster->HealthBelowPctDamaged(35, dmgInfo->GetDamage()))
            return;

        caster->CastSpell(caster, SPELL_MONK_HEALING_ELIXIRS_RESTORE_HEALTH, true);

        if (SpellInfo const* healSpell = sSpellMgr->GetSpellInfo(SPELL_MONK_HEALING_ELIXIRS_RESTORE_HEALTH, DIFFICULTY_NONE))
            caster->GetSpellHistory()->ConsumeCharge(healSpell->ChargeCategoryId);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_healing_elixirs_aura::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
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

// Mastery: Combo Strikes (115636) - shared infrastructure.
// Windwalker's core mastery: consecutive uses of the *same* ability deal no bonus damage;
// alternating between different abilities grants a damage bonus scaling with Mastery rating.
// The reference implementation tracks "last ability used" per player via a `PlayerStorage` system
// that does not exist anywhere in ArgusCore's engine (the same category of gap as Execute's
// missing OnTakePower and Gift of the Ox's missing OnTakeDamage earlier this session). Rather
// than requiring a new engine-wide storage system, this uses a lightweight file-scope
// std::unordered_map keyed by player GUID instead - functionally equivalent for this single
// purpose (remembering one spell id per player) without needing broader engine changes.
// Entries are small (one int32 each) and only accumulate for players who have actually used a
// Combo-Strikes-tracked ability at least once; not cleared on logout, which is an accepted
// minor memory-retention tradeoff for this workaround, not a functional issue.
namespace MonkComboStrikes
{
    std::unordered_map<ObjectGuid, int32> lastAbilityUsed;

    bool HasEntry(ObjectGuid guid) { return lastAbilityUsed.contains(guid); }
    int32 GetEntry(ObjectGuid guid) { auto itr = lastAbilityUsed.find(guid); return itr != lastAbilityUsed.end() ? itr->second : 0; }
    void SetEntry(ObjectGuid guid, int32 spellId) { lastAbilityUsed[guid] = spellId; }

    // Applies/removes the "Hit Combo" talent's stacking haste buff (196740/196741) alongside
    // Combo Strikes tracking, matching the reference implementation's structure exactly.
    void HandleHitCombo(Unit* caster, bool apply = true)
    {
        if (!caster->HasAura(SPELL_MONK_HIT_COMBO) || !caster->IsAlive())
            return;

        if (apply)
            caster->CastSpell(caster, SPELL_MONK_HIT_COMBO_AURA, true);
        else
            caster->RemoveAura(SPELL_MONK_HIT_COMBO_AURA);
    }

    // Central Combo Strikes logic shared by all three binding classes below. Returns true (and
    // applies the damage bonus in-place) if this cast counts as "different from the last
    // ability used"; false if it was a repeat (no bonus) or the very first tracked cast since
    // login/death (nothing to compare against yet).
    bool TryToHandleDamage(Player* caster, int32 spellId, int32& damage, bool repeated = false)
    {
        if (!caster || !damage)
            return false;

        AuraEffect const* comboStrikes = caster->GetAuraEffect(SPELL_MONK_MASTERY_COMBO_STRIKES, EFFECT_0);
        if (!comboStrikes)
            return false;

        ObjectGuid guid = caster->GetGUID();

        // Don't handle first cast after login or death
        if (!MonkComboStrikes::HasEntry(guid))
        {
            MonkComboStrikes::SetEntry(guid, spellId);
            return false;
        }

        if (!repeated && MonkComboStrikes::GetEntry(guid) == spellId)
        {
            HandleHitCombo(caster, false);
            return false;
        }

        AddPct(damage, caster->GetFloatValue(PLAYER_MASTERY) * comboStrikes->GetSpellEffectInfo().BonusCoefficient);
        MonkComboStrikes::SetEntry(guid, spellId);

        if (!repeated)
            HandleHitCombo(caster);

        return true;
    }
}

// 115636 - Mastery: Combo Strikes (direct-hit abilities)
// Bound to each Windwalker direct-damage ability individually (Tiger Palm 100780, Blackout
// Kick 100784, Flying Serpent Kick AOE 123586, Rising Sun Kick's secondary id 185099) via SQL -
// there is no generic classmask-based way to hook "any Windwalker damage ability" in this
// engine, so each must be bound explicitly, matching the reference implementation's own approach.
class spell_monk_mastery_combo_strikes : public SpellScript
{
    bool _repeated = false;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_MASTERY_COMBO_STRIKES, SPELL_MONK_HIT_COMBO, SPELL_MONK_HIT_COMBO_AURA });
    }

    bool Load() override
    {
        return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER && GetCaster()->HasAura(SPELL_MONK_MASTERY_COMBO_STRIKES);
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 damage = GetHitDamage();
        if (!MonkComboStrikes::TryToHandleDamage(caster->ToPlayer(), int32(GetSpellInfo()->Id), damage, _repeated))
            return;

        _repeated = true;
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_mastery_combo_strikes::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 115636 - Mastery: Combo Strikes (periodic-channel abilities)
// Bound to the three channeled/periodic-driver Windwalker abilities (Fists of Fury 113656,
// Spinning Crane Kick 101546, Whirling Dragon Punch 152175). Tracks combo state on aura
// apply/remove rather than on-hit, since these abilities' actual damage comes from separate
// periodic sub-spells handled by spell_monk_mastery_combo_strikes_periodic_triggers below.
// The "spellId + 1" sentinel on removal marks "just ended" so a near-simultaneous reapply
// (e.g. a channel restarting) isn't miscounted as a fresh combo entry.
class spell_monk_mastery_combo_strikes_periodic_auras : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_MASTERY_COMBO_STRIKES });
    }

    bool Load() override
    {
        return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER && GetCaster()->HasAura(SPELL_MONK_MASTERY_COMBO_STRIKES);
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        ObjectGuid guid = caster->GetGUID();
        int32 spellId = int32(GetSpellInfo()->Id);

        if (!MonkComboStrikes::HasEntry(guid))
            return;

        if (MonkComboStrikes::GetEntry(guid) == spellId + 1)
        {
            MonkComboStrikes::HandleHitCombo(caster, false);
            return;
        }

        MonkComboStrikes::SetEntry(guid, spellId);
        MonkComboStrikes::HandleHitCombo(caster);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        // Prevent handling next cast
        MonkComboStrikes::SetEntry(caster->GetGUID(), int32(GetSpellInfo()->Id) + 1);
    }

    void Register() override
    {
        if (m_scriptSpellId == SPELL_MONK_SPINNING_CRANE_KICK)
        {
            AfterEffectApply += AuraEffectApplyFn(spell_monk_mastery_combo_strikes_periodic_auras::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_monk_mastery_combo_strikes_periodic_auras::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
        else
        {
            AfterEffectApply += AuraEffectApplyFn(spell_monk_mastery_combo_strikes_periodic_auras::HandleApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_monk_mastery_combo_strikes_periodic_auras::HandleRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    }
};

// 115636 - Mastery: Combo Strikes (periodic-channel damage sub-spells)
// Bound to the damage-dealing sub-spells of the three periodic-channel abilities above (Fists
// of Fury Damage 117418, Spinning Crane Kick Damage 107270, Whirling Dragon Punch Damage
// 158221) - applies the actual damage bonus for these periodic ticks, mapped back to their
// parent ability's id for the combo comparison.
class spell_monk_mastery_combo_strikes_periodic_triggers : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_MONK_MASTERY_COMBO_STRIKES,
            SPELL_MONK_FISTS_OF_FURY_DAMAGE,
            SPELL_MONK_FISTS_OF_FURY,
            SPELL_MONK_SPINNING_CRANE_KICK_DAMAGE,
            SPELL_MONK_SPINNING_CRANE_KICK,
            SPELL_MONK_WHIRLING_DRAGON_PUNCH_DAMAGE,
            SPELL_MONK_WHIRLING_DRAGON_PUNCH
        });
    }

    bool Load() override
    {
        return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER && GetCaster()->HasAura(SPELL_MONK_MASTERY_COMBO_STRIKES);
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        int32 spellHandleId;
        switch (GetSpellInfo()->Id)
        {
            case SPELL_MONK_FISTS_OF_FURY_DAMAGE:          spellHandleId = SPELL_MONK_FISTS_OF_FURY; break;
            case SPELL_MONK_SPINNING_CRANE_KICK_DAMAGE:    spellHandleId = SPELL_MONK_SPINNING_CRANE_KICK; break;
            case SPELL_MONK_WHIRLING_DRAGON_PUNCH_DAMAGE:  spellHandleId = SPELL_MONK_WHIRLING_DRAGON_PUNCH; break;
            default:                                        return;
        }

        int32 damage = GetHitDamage();
        MonkComboStrikes::TryToHandleDamage(caster, spellHandleId, damage, true);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_mastery_combo_strikes_periodic_triggers::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 115175 - Soothing Mist
// Mistweaver's healing channel: applies a visual on the target while channeling, has a 25%
// chance per tick to generate a Chi, and cleans up the visual when the channel ends.
// NOTE: the reference implementation also crosses over into the Jade Serpent Statue mechanic here
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

// 116694 - Surging Mist
// Redirects to whoever the caster is currently channeling Soothing Mist on, if any, then heals.
class spell_monk_surging_mist : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_SURGING_MIST_HEAL });
    }

    void SelectTarget(WorldObject*& target)
    {
        Unit* caster = GetCaster();
        if (!caster || caster->GetChannelSpellId() != SPELL_MONK_SOOTHING_MIST)
            return;

        DynamicFieldStructuredView<ObjectGuid> channelObjects = caster->GetChannelObjects();
        if (channelObjects.size() == 1)
            if (Unit* soothingMistTarget = ObjectAccessor::GetUnit(*caster, *channelObjects.begin()))
                target = soothingMistTarget;
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (caster && target)
            caster->CastSpell(target, SPELL_MONK_SURGING_MIST_HEAL, true);
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_monk_surging_mist::SelectTarget, EFFECT_0, TARGET_UNIT_TARGET_ALLY);
        OnEffectHitTarget += SpellEffectFn(spell_monk_surging_mist::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 123273 - Surging Mist (glyphed)
// While channeling Soothing Mist, redirects entirely to that target; otherwise heals the
// lowest-health raid member instead of requiring an explicit target, falling back to the
// caster if no valid target is found at all.
class spell_monk_surging_mist_glyphed : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_SURGING_MIST_HEAL, SPELL_MONK_SOOTHING_MIST });
    }

    void SelectTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->GetChannelSpellId() == SPELL_MONK_SOOTHING_MIST)
        {
            targets.clear();

            DynamicFieldStructuredView<ObjectGuid> channelObjects = caster->GetChannelObjects();
            if (channelObjects.size() == 1)
                if (Unit* soothingMistTarget = ObjectAccessor::GetUnit(*caster, *channelObjects.begin()))
                    targets.push_back(soothingMistTarget);
        }
        else
        {
            targets.remove_if([caster](WorldObject* target)
            {
                return target->GetTypeId() != TYPEID_UNIT || !target->ToUnit()->IsInRaidWith(caster);
            });
            targets.sort(Trinity::Predicates::HealthPctOrderPred());
            if (!targets.empty())
                targets.resize(1);
        }

        if (targets.empty())
            targets.push_back(caster);
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (caster && target)
            caster->CastSpell(target, SPELL_MONK_SURGING_MIST_HEAL, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_surging_mist_glyphed::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        OnEffectHitTarget += SpellEffectFn(spell_monk_surging_mist_glyphed::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 116705 - Spear Hand Strike
// Interrupt/silence: only applies if the target is in front of the caster, and self-applies a
// hardcoded 15s cooldown matching the outer spell's own id (mirrors the reference's literal
// value; not independently verified against Legion 7.3.5 tooltip data).
class spell_monk_spear_hand_strike : public SpellScript
{
    void HandleOnHit()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->isInFront(caster))
            return;

        caster->CastSpell(target, SPELL_MONK_SPEAR_HAND_STRIKE_SILENCE, true);
        caster->GetSpellHistory()->AddCooldown(116705, 0, std::chrono::seconds(15));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_monk_spear_hand_strike::HandleOnHit);
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
// NOTE: one reference implementation implements this as a flat self-absorb shield
// (AttackPower * 18) - that matches Guard's pre-Legion (Mists of Pandaria) mechanic, not
// 7.3.5's. Guard was redesigned into a PvP honor talent in patch 7.1.5 (well before 7.3.5):
// it no longer provides a self-absorb at all, instead redirecting 30% of a protected nearby
// ally's incoming damage into the Monk's own Stagger pool. That reference's own bound spell id
// (202162) matches this later PvP-talent version's real id, not the old MoP ability's id
// (115295) - meaning its code and its own binding actually disagree with each
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

// 116645 - Teachings of the Monastery
// Passive: Tiger Palm has a chance to grant a stacking buff (202090) that Blackout Kick later
// consumes for bonus damage (spell_monk_blackout_kick above already handles the consumption
// side) - this is the missing granting half. Without this, Blackout Kick's consumption logic
// would always find zero stacks and this entire talent would silently do nothing.
class spell_monk_teachings_of_the_monastery : public AuraScript
{
    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!caster || !procSpell || procSpell->Id != SPELL_MONK_TIGER_PALM)
            return;

        caster->CastSpell(caster, SPELL_MONK_TEACHINGS_OF_THE_MONASTERY_AURA, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_teachings_of_the_monastery::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
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
// UPDATE: the reference implementation's own CalculateAmount had a Mastery: Combo Strikes integration commented
// out (with an author TODO note, "need to merge, already did" — never finished). Initially left
// unported for that reason, but the user asked to complete it, so the full Combo Strikes system
// (see the "Mastery: Combo Strikes - shared infrastructure" block above) was built and this
// integration is now wired in below.
class spell_monk_touch_of_death : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_TOUCH_OF_DEATH, SPELL_MONK_MASTERY_COMBO_STRIKES });
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

        if (!caster->HasAura(SPELL_MONK_MASTERY_COMBO_STRIKES) || caster->GetTypeId() != TYPEID_PLAYER)
            return;

        MonkComboStrikes::TryToHandleDamage(caster->ToPlayer(), int32(GetSpellInfo()->Id), amount);
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

// 152175 - Whirling Dragon Punch
// Periodic driver: each tick casts the actual damage sub-spell (158221). Without this, the
// spell_monk_mastery_combo_strikes_periodic_auras binding added earlier this session tracks
// combo state on this aura's apply/remove, but nothing was actually driving its damage - a gap
// found while investigating this ability further, not caught when Combo Strikes was built.
// NOTE: a companion PlayerScript in the reference implementation ties Whirling Dragon Punch's
// "usable window" to Fists of Fury and Rising Sun Kick's cooldowns via
// PlayerScript::OnCooldownStart/OnChargeRecoveryTimeStart - neither hook exists in ArgusCore's
// PlayerScript at all (the same category of gap as Execute's missing OnTakePower and Gift of
// the Ox's missing OnTakeDamage). That gating mechanic is deferred; this fix covers only the
// actual damage-dealing half, which has no such dependency.
class spell_monk_whirling_dragon_punch : public AuraScript
{
    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_MONK_WHIRLING_DRAGON_PUNCH_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_whirling_dragon_punch::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 126892 - Zen Pilgrimage, 126895 - Zen Pilgrimage: Return
// Hearthstone-style pair: teleports to the class order hall (or Peak of Serenity below level
// 98, before Order Halls unlock) and saves a recall position, then teleports back and clears
// the travel-form aura on return. One shared class bound to both spell ids - each hook only
// fires for the effect index/type present on its own spell's data, so no cross-firing risk.
class spell_monk_zen_pilgrimage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_ZEN_PILGRIMAGE, SPELL_MONK_ZEN_PILGRIMAGE_RETURN });
    }

    void HandleTeleport(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        PreventHitEffect(effIndex);

        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        player->SaveRecallPosition();
        if (player->GetLevel() >= 98)
            player->TeleportTo(1514, 882.933f, 3605.61f, 192.218f, player->GetOrientation());
        else
            player->TeleportTo(870, 3818.55f, 1793.18f, 950.35f, player->GetOrientation());
    }

    void HandleReturn(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        PreventHitEffect(effIndex);

        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        player->Recall();
        player->RemoveAura(SPELL_MONK_ZEN_PILGRIMAGE_RETURN_AURA);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_zen_pilgrimage::HandleTeleport, EFFECT_0, SPELL_EFFECT_TELEPORT_UNITS);
        OnEffectHitTarget += SpellEffectFn(spell_monk_zen_pilgrimage::HandleReturn, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 124081 - Zen Pulse
// Casts a self-heal (198487) alongside its own damage effect.
class spell_monk_zen_pulse : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONK_ZEN_PULSE_HEAL });
    }

    void OnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_MONK_ZEN_PULSE_HEAL, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_zen_pulse::OnHit, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 137639 - Storm, Earth, and Fire (Windwalker signature cooldown): summons a Fire and an Earth
// spirit clone that mirror the caster's offensive casts. Confirmed via DestinyCore/AshamaneCore
// (identical implementations) - this turned out to be much smaller than the doc's original
// "large creature-AI undertaking" assessment suggested: one AuraScript for
// summon/cleanup, one small ScriptedAI struct with a single IsSummonedBy hook (no bespoke
// combat AI needed - the base ScriptedAI's default melee behavior handles the rest, matching
// the already-working precedent of Shaman's npc_sha_feral_spirit in spell_shaman.cpp), and one
// PlayerScript that mirrors offensive casts to both spirits. Translated
// Unit::GetSummonedCreatureByEntry (doesn't exist in ArgusCore) to the established
// Unit::m_Controlled filter idiom (matching Implosion/Dancing Rune Weapon/Transcendence
// elsewhere this session), and PlayerScript::OnSuccessfulSpellCast (doesn't exist) to
// ArgusCore's actual equivalent, PlayerScript::OnSpellCast (fires from Spell::_cast(), after
// cast checks pass and immediately before the spell executes - functionally equivalent to
// "successful cast" for this purpose).
class spell_monk_storm_earth_and_fire : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sObjectMgr->GetCreatureTemplate(NPC_MONK_SEF_FIRE_SPIRIT) || !sObjectMgr->GetCreatureTemplate(NPC_MONK_SEF_EARTH_SPIRIT))
            return false;
        return ValidateSpellInfo({ SPELL_MONK_SEF_STORM_VISUAL, SPELL_MONK_SEF_SUMMON_EARTH, SPELL_MONK_SEF_SUMMON_FIRE });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_MONK_SEF_STORM_VISUAL, true);
        target->CastSpell(target, SPELL_MONK_SEF_SUMMON_EARTH, true);
        target->CastSpell(target, SPELL_MONK_SEF_SUMMON_FIRE, true);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->RemoveAurasDueToSpell(SPELL_MONK_SEF_STORM_VISUAL);

        for (Unit* controlled : target->m_Controlled)
        {
            if (controlled->GetEntry() == NPC_MONK_SEF_FIRE_SPIRIT || controlled->GetEntry() == NPC_MONK_SEF_EARTH_SPIRIT)
                if (Creature* spirit = controlled->ToCreature())
                    spirit->DespawnOrUnsummon();
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_monk_storm_earth_and_fire::HandleApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_monk_storm_earth_and_fire::HandleRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

// 69791 (Fire)/69792 (Earth) - Storm, Earth, and Fire spirit clones.
struct npc_monk_sef_spirit : public ScriptedAI
{
    npc_monk_sef_spirit(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summonerObj) override
    {
        Unit* summoner = summonerObj->ToUnit();
        if (!summoner)
            return;

        me->SetLevel(summoner->GetLevel());
        summoner->CastSpell(me, SPELL_MONK_TRANSCENDENCE_CLONE_TARGET, true);
        me->CastSpell(me, me->GetEntry() == NPC_MONK_SEF_FIRE_SPIRIT ? SPELL_MONK_SEF_FIRE_VISUAL : SPELL_MONK_SEF_EARTH_VISUAL, true);
        me->CastSpell(me, SPELL_MONK_SEF_SUMMONS_STATS, true);

        if (Unit* target = ObjectAccessor::GetUnit(*summoner, summoner->GetTarget()))
            me->CastSpell(target, SPELL_MONK_SEF_CHARGE, true);
    }
};

// Mirrors the Windwalker's offensive casts onto both Storm, Earth, and Fire spirits.
class playerScript_monk_storm_earth_and_fire : public PlayerScript
{
public:
    playerScript_monk_storm_earth_and_fire() : PlayerScript("playerScript_monk_storm_earth_and_fire") { }

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        SpellInfo const* spellInfo = spell->GetSpellInfo();
        if (!player->HasAura(SPELL_MONK_SEF) || spellInfo->IsPositive())
            return;

        Unit* target = ObjectAccessor::GetUnit(*player, player->GetTarget());
        if (!target)
            return;

        for (Unit* controlled : player->m_Controlled)
        {
            if (controlled->GetEntry() == NPC_MONK_SEF_FIRE_SPIRIT || controlled->GetEntry() == NPC_MONK_SEF_EARTH_SPIRIT)
                controlled->CastSpell(target, spellInfo->Id, true);
        }
    }
};

void AddSC_monk_spell_scripts()
{
    RegisterSpellScript(spell_monk_black_ox_brew);
    RegisterSpellScript(spell_monk_blackout_kick);
    RegisterSpellAndAuraScriptPair(spell_monk_disable, aura_monk_disable);
    RegisterSpellScript(spell_monk_breath_of_fire);
    RegisterSpellScript(spell_monk_elusive_brawler_mastery);
    RegisterSpellScript(spell_monk_elusive_brawler_stacks);
    RegisterSpellScript(spell_monk_expel_harm);
    RegisterSpellScript(spell_monk_mana_tea);
    RegisterSpellScript(spell_monk_mana_tea_stacks);
    RegisterSpellScript(spell_monk_transcendence);
    RegisterSpellScript(spell_monk_transcendence_transfer);
    RegisterSpellScript(spell_monk_burst_of_life);
    RegisterSpellScript(spell_monk_burst_of_life_heal);
    RegisterSpellScript(spell_monk_chi_burst_heal);
    RegisterSpellScript(spell_monk_chi_torpedo);
    RegisterSpellScript(spell_monk_chi_wave);
    RegisterSpellScript(spell_monk_chi_wave_damage_missile);
    RegisterSpellScript(spell_monk_chi_wave_heal_missile);
    RegisterSpellScript(spell_monk_chi_wave_target_selector);
    RegisterSpellScript(spell_monk_crackling_jade_lightning);
    RegisterSpellScript(spell_monk_crackling_jade_lightning_knockback_proc_aura);
    RegisterSpellScript(spell_monk_dampen_harm);
    RegisterSpellScript(spell_monk_energizing_brew);
    RegisterSpellScript(spell_monk_enveloping_mist);
    RegisterSpellScript(spell_monk_essence_font_heal);
    RegisterSpellScript(spell_monk_fists_of_fury);
    RegisterSpellScript(spell_monk_fists_of_fury_damage);
    RegisterSpellScript(spell_monk_fists_of_fury_visual_filter);
    RegisterSpellScript(spell_monk_fists_of_fury_visual);
    RegisterSpellScript(spell_monk_flying_serpent_kick);
    RegisterSpellScript(spell_monk_fortifying_brew);
    RegisterSpellScript(spell_monk_gift_of_the_ox_aura);
    RegisterSpellScript(spell_monk_healing_elixirs_aura);
    RegisterSpellScript(spell_monk_jade_walk);
    RegisterSpellScript(spell_monk_keg_smash);
    RegisterSpellScript(spell_monk_legacy_of_the_emperor);
    RegisterSpellScript(spell_monk_life_cocoon);
    RegisterSpellScript(spell_monk_lifecycles);
    RegisterSpellScript(spell_monk_mastery_combo_strikes);
    RegisterSpellScript(spell_monk_mastery_combo_strikes_periodic_auras);
    RegisterSpellScript(spell_monk_mastery_combo_strikes_periodic_triggers);
    RegisterSpellScript(spell_monk_mists_of_life);
    RegisterSpellScript(spell_monk_open_palm_strikes);
    RegisterSpellScript(spell_monk_power_strike_periodic);
    RegisterSpellScript(spell_monk_power_strike_proc);
    RegisterSpellScript(spell_monk_pressure_points);
    RegisterSpellScript(spell_monk_provoke);
    RegisterSpellScript(spell_monk_purifying_brew);
    RegisterSpellScript(spell_monk_spear_hand_strike);
    RegisterSpellScript(spell_monk_surging_mist);
    RegisterSpellScript(spell_monk_surging_mist_glyphed);
    RegisterSpellScript(spell_monk_guard);
    RegisterSpellScript(spell_monk_renewing_mist);
    RegisterSpellScript(spell_monk_renewing_mist_periodic);
    RegisterSpellScript(spell_monk_ring_of_peace_aura);
    RegisterSpellScript(spell_monk_rising_sun_kick);
    RegisterSpellScript(spell_monk_rising_thunder);
    RegisterSpellScript(spell_monk_soothing_mist);
    RegisterSpellScript(spell_monk_soothing_mist_aura);
    RegisterSpellScript(spell_monk_roll);
    RegisterSpellScript(spell_monk_roll_aura);
    RegisterSpellScript(spell_monk_save_them_all);
    RegisterAreaTriggerAI(at_monk_song_of_chi_ji);
    RegisterAreaTriggerAI(at_monk_gift_of_the_ox_sphere);
    RegisterAreaTriggerAI(at_monk_chi_burst_damage);
    RegisterAreaTriggerAI(at_monk_chi_burst_heal);
    RegisterSpellScript(spell_monk_stagger);
    RegisterSpellScript(spell_monk_stagger_damage_aura);
    RegisterSpellScript(spell_monk_stagger_debuff_aura);
    RegisterSpellScript(spell_monk_teachings_of_the_monastery);
    RegisterSpellScript(spell_monk_tigers_lust);
    RegisterSpellScript(spell_monk_touch_of_death);
    RegisterSpellScript(spell_monk_touch_of_karma);
    RegisterSpellScript(spell_monk_whirling_dragon_punch);
    RegisterSpellScript(spell_monk_zen_pilgrimage);
    RegisterSpellScript(spell_monk_zen_pulse);
    RegisterSpellScript(spell_monk_storm_earth_and_fire);
    RegisterCreatureAI(npc_monk_sef_spirit);
    new playerScript_monk_storm_earth_and_fire();
}
