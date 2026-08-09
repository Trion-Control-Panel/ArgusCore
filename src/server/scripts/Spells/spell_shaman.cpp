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
 * Scripts for spells with SPELLFAMILY_SHAMAN and SPELLFAMILY_GENERIC spells used by shaman players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_sha_".
 */

#include "ScriptMgr.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "Containers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

enum ShamanSpells
{
    SPELL_SHAMAN_AFTERSHOCK_ENERGIZE            = 210712,
    SPELL_SHAMAN_ANCESTRAL_GUIDANCE             = 108281,
    SPELL_SHAMAN_ANCESTRAL_GUIDANCE_HEAL        = 114911,
    SPELL_SHAMAN_ASCENDANCE_ELEMENTAL           = 114050,
    SPELL_SHAMAN_ASCENDANCE_ENHANCEMENT         = 114051,
    SPELL_SHAMAN_ASCENDANCE_RESTORATION         = 114052,
    SPELL_SHAMAN_CHAIN_LIGHTNING                = 188443,
    SPELL_SHAMAN_CHAIN_LIGHTNING_ENERGIZE       = 195897,
    SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD       = 45297,
    SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_ENERGIZE = 218558,
    SPELL_SHAMAN_CHAINED_HEAL                   = 70809,
    SPELL_SHAMAN_CRASH_LIGHTNING                = 187874,
    SPELL_SHAMAN_CRASH_LIGHTNING_CLEAVE         = 187878,
    SPELL_SHAMAN_FURY_OF_AIR                    = 197211,
    SPELL_SHAMAN_CRASH_LIGHTNING_DAMAGE_BUFF    = 195592, // real id, referenced by Crash Lightning's (187874) own tooltip (333964 is a later-expansion remake id)
    SPELL_SHAMAN_DELUGE_AURA                    = 200075,
    SPELL_SHAMAN_DELUGE_TALENT                  = 200076,
    SPELL_SHAMAN_EARTH_ELEMENTAL_SUMMON         = 188616,
    SPELL_SHAMAN_EARTHBIND_FOR_EARTHGRAB_TOTEM  = 116947,
    SPELL_SHAMAN_EARTHGRAB_IMMUNITY             = 116946,
    SPELL_SHAMAN_EARTHQUAKE                     = 61882,
    SPELL_SHAMAN_EARTHQUAKE_KNOCKING_DOWN       = 77505,
    SPELL_SHAMAN_EARTHQUAKE_TICK                = 77478,
    SPELL_SHAMAN_EARTH_SHIELD_HEAL              = 204290,
    SPELL_SHAMAN_EARTHEN_RAGE_PASSIVE           = 170374,
    SPELL_SHAMAN_EARTHEN_RAGE_PERIODIC          = 170377,
    SPELL_SHAMAN_EARTHEN_RAGE_DAMAGE            = 170379,
    SPELL_SHAMAN_ELECTRIFIED                    = 64930,
    SPELL_SHAMAN_ELEMENTAL_BLAST                = 117014,
    SPELL_SHAMAN_ELEMENTAL_BLAST_CRIT           = 118522,
    SPELL_SHAMAN_ELEMENTAL_BLAST_HASTE          = 173183,
    SPELL_SHAMAN_ELEMENTAL_BLAST_MASTERY        = 173184,
    SPELL_SHAMAN_ELEMENTAL_BLAST_OVERLOAD       = 120588,
    SPELL_SHAMAN_ELEMENTAL_MASTERY              = 16166,
    SPELL_SHAMAN_ENERGY_SURGE                   = 40465,
    SPELL_SHAMAN_EARTHEN_SHIELD_ABSORB          = 201633,
    SPELL_SHAMAN_EARTHEN_SHIELD_ABSORB_DAMAGE   = 201657,
    // Earthen Shield Totem creature entry - confirmed via ArgusCore's own base world DB dump
    // (a VerifiedBuild row explicitly labels entry 100943 "Earthen Shield Totem" under a
    // genuine Legion build number), not merely a web lookup that could have surfaced the later
    // "Earthen Wall Totem" identity instead.
    NPC_SHAMAN_EARTHEN_SHIELD_TOTEM              = 100943,
    SPELL_SHAMAN_ENHANCED_ELEMENTS              = 77223,
    SPELL_SHAMAN_FERAL_LUNGE_DAMAGE             = 215802,
    SPELL_SHAMAN_FIRE_ELEMENTAL_SUMMON          = 188592,
    SPELL_SHAMAN_FLAME_SHOCK                    = 188389,
    SPELL_SHAMAN_FLAMETONGUE_ATTACK             = 10444,
    SPELL_SHAMAN_FLAMETONGUE_WEAPON_ENCHANT     = 334294,
    // real id "Flametongue Weapon" buff (319778 doesn't exist under any id); confirmed via
    // wago.tools SpellEffect data - APPLY_AURA MOD_DAMAGE_DONE + a PROC_TRIGGER_SPELL effect
    // matching its own tooltip ("increasing magical damage... each hit causes additional Fire
    // damage"), distinct from SPELL_SHAMAN_FLAMETONGUE_WEAPON_ENCHANT (the enchant grant itself)
    SPELL_SHAMAN_FLAMETONGUE_WEAPON_AURA        = 160098,
    SPELL_SHAMAN_FROST_SHOCK                    = 196840,
    SPELL_SHAMAN_FROSTBRAND_WEAPON_ENCHANT      = 196834,
    SPELL_SHAMAN_GATHERING_STORMS               = 198299,
    SPELL_SHAMAN_GATHERING_STORMS_BUFF          = 198300,
    SPELL_SHAMAN_GHOST_WOLF                     = 2645,
    SPELL_SHAMAN_HAILSTORM_BUFF                 = 210854, // real id, tooltip-linked to 210853 (334196 is a later-expansion remake id)
    SPELL_SHAMAN_HAILSTORM_TALENT               = 210853, // real id "Hailstorm" (334195 is a later-expansion remake id)
    SPELL_SHAMAN_HEALING_RAIN_VISUAL            = 147490,
    SPELL_SHAMAN_HEALING_RAIN                   = 73920,
    SPELL_SHAMAN_HEALING_RAIN_HEAL              = 73921,
    SPELL_SHAMAN_ICEFURY                        = 210714,
    SPELL_SHAMAN_ICEFURY_OVERLOAD               = 219271,
    SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD          = 23552,
    SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD_DAMAGE   = 27635,
    SPELL_SHAMAN_ITEM_MANA_SURGE                = 23571,
    SPELL_SHAMAN_LAVA_BURST                     = 51505,
    SPELL_SHAMAN_LAVA_BURST_BONUS_DAMAGE        = 71824,
    SPELL_SHAMAN_LAVA_BURST_OVERLOAD            = 77451,
    SPELL_SHAMAN_LAVA_BURST_RANK_2              = 231721,
    SPELL_SHAMAN_LAVA_LASH                      = 60103,
    SPELL_SHAMAN_LAVA_SURGE                     = 77762,
    SPELL_SHAMAN_LIGHTNING_BOLT                 = 188196,
    SPELL_SHAMAN_LIGHTNING_BOLT_ENERGIZE        = 214815,
    SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD        = 45284,
    SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_ENERGIZE = 214816,
    SPELL_SHAMAN_LIQUID_MAGMA_HIT               = 192231,
    // real id "Maelstrom Controller" (343725 is a later-expansion remake id); confirmed via
    // wago.tools SpellEffect data to hold Lightning Bolt's (EFFECT_0) and Lightning Bolt
    // Overload's (EFFECT_1) energize amounts, each effect's EffectTriggerSpell pointing at a
    // "Fulmination"/"Fulmination!" helper matching 190493 Fulmination's own real tooltip
    // ("Damage from your Lightning Bolt and Lava Burst spells generate Maelstrom... Chain
    // Lightning generates Maelstrom"). Only has 2 effects total - Chain Lightning/Chain Lightning
    // Overload do NOT use this spell (their EFFECT_4/EFFECT_5 don't exist here); their own
    // energize spells (195897/218558) are self-contained instead, see spell_sha_chain_lightning_energize
    // and spell_sha_chain_lightning_overload below.
    SPELL_SHAMAN_MAELSTROM_CONTROLLER           = 190488,
    SPELL_SHAMAN_MAELSTROM_WEAPON_ENERGIZE      = 187890,
    SPELL_SHAMAN_MASTERY_ELEMENTAL_OVERLOAD     = 168534,
    // FIXME: 334033 is confirmed absent from Spell.db2 under any id, and no real Legion 7.3.5
    // equivalent of "Lava Lash spreads Flame Shock to nearby enemies" was found (real Legion Lava
    // Lash instead has a separate talent, Lashing Flames/238142, doing a different thing - a
    // damage-amp debuff, not a spread). Left as-is rather than guessed at; the class itself
    // (spell_sha_molten_assault) stays bound to real Lava Lash (60103) with a correctly-verified
    // hook, only this gating talent id is unresolved.
    SPELL_SHAMAN_MOLTEN_ASSAULT                 = 334033,
    SPELL_SHAMAN_PATH_OF_FLAMES_SPREAD          = 210621,
    SPELL_SHAMAN_PATH_OF_FLAMES_TALENT          = 201909,
    SPELL_SHAMAN_POWER_SURGE                    = 40466,
    SPELL_SHAMAN_RESTORATIVE_MISTS              = 114083,
    SPELL_SHAMAN_RESTORATIVE_MISTS_INITIAL      = 294020,
    SPELL_SHAMAN_RIPTIDE                        = 61295,
    SPELL_SHAMAN_SPIRIT_LINK_HEAL                = 98021,
    SPELL_SHAMAN_STORMFLURRY_ARTIFACT           = 198367,
    SPELL_SHAMAN_STORMKEEPER                    = 191634,
    SPELL_SHAMAN_STORMLASH                      = 195255,
    SPELL_SHAMAN_STORMLASH_BUFF                 = 195222,
    SPELL_SHAMAN_STORMLASH_DAMAGE               = 213307,
    SPELL_SHAMAN_STORMSTRIKE                    = 17364,
    SPELL_SHAMAN_STORMSTRIKE_DAMAGE_MAIN_HAND   = 32175,
    SPELL_SHAMAN_STORMSTRIKE_DAMAGE_OFF_HAND    = 32176,
    SPELL_SHAMAN_STORMSURGE_PROC                = 201846,
    SPELL_TOTEM_CLOUDBURST                      = 157503,
    SPELL_SHAMAN_TIDAL_WAVES                    = 53390,
    SPELL_SHAMAN_TOTEMIC_POWER_ARMOR            = 28827,
    SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER     = 28826,
    SPELL_SHAMAN_TOTEMIC_POWER_MP5              = 28824,
    SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER      = 28825,
    SPELL_SHAMAN_UNDULATION_PROC                = 216251,
    SPELL_SHAMAN_WINDFURY_ATTACK                = 25504,
    // 33757 - Windfury Weapon: self-referential design, confirmed via build-pinned (7.3.5.26972)
    // SpellEffect data - the outer cast spell (33757) has a single EFFECT_0, SPELL_EFFECT_APPLY_AURA
    // with SPELL_AURA_DUMMY, self-target. That's the same id as the resulting buff/proc aura; there
    // is no separate enchant/buff id (319773, formerly used here, is confirmed absent - it doesn't
    // exist under any id across three separate 7.3.5 client builds, and neither does 334302, the
    // "enchant" id spell_sha_windfury_weapon used to cast onto the weapon item - real 33757 isn't a
    // weapon-item-enchant effect at all, so that whole cast-onto-item premise was wrong).
    SPELL_SHAMAN_WINDFURY_AURA                  = 33757,
    SPELL_SHAMAN_WIND_RUSH                      = 192082,
    SPELL_SHAMAN_WINDSTRIKE_DAMAGE_MAIN_HAND    = 115357,
    SPELL_SHAMAN_WINDSTRIKE_DAMAGE_OFF_HAND     = 115360,

    // Feral Spirit — integrated from TheLegionPreservationProject
    SPELL_SHAMAN_FERAL_SPIRIT                   = 51533,
    SPELL_SHAMAN_FERAL_SPIRIT_SUMMON            = 228562,
    SPELL_SHAMAN_FERAL_SPIRIT_ENERGIZE          = 190185,
    SPELL_SHAMAN_FERAL_SPIRIT_ENERGIZE_DUMMY    = 231723
};

enum MiscNpcs
{
    NPC_HEALING_RAIN_INVISIBLE_STALKER          = 73400,
    NPC_SPIRIT_WOLF                             = 29264
};

class WindfuryProcEvent : public BasicEvent
{
public:
    struct WindfuryProcEventInfo
    {
        Milliseconds Delay;
    };

    static constexpr std::array<WindfuryProcEventInfo, 3> Sequence =
    { {
        {.Delay = 500ms },
        {.Delay = 150ms },
        {.Delay = 250ms },
    } };

    WindfuryProcEvent(Unit* shaman, Unit* target, std::ptrdiff_t attacks) : _shaman(shaman), _target(target), _itr(Sequence.begin()), _end(_itr + attacks) { }

    bool Execute(uint64 time, uint32 /*diff*/) override;

    static void Trigger(Unit* shaman, Unit* target);

private:
    Unit* _shaman;
    CastSpellTargetArg _target;
    using EventInfoIterator = decltype(Sequence)::const_iterator;
    EventInfoIterator _itr;
    EventInfoIterator _end;
};

// 210707 - Aftershock
class spell_sha_aftershock : public AuraScript
{
    bool Validate(SpellInfo const* /*spellEntry*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_AFTERSHOCK_ENERGIZE });
    }

    static bool CheckProc(AuraScript const&, AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        if (Spell const* procSpell = eventInfo.GetProcSpell())
            if (Optional<int32> cost = procSpell->GetPowerTypeCostAmount(POWER_MAELSTROM))
                return cost > 0 && roll_chance_i(aurEff->GetAmount());

        return false;
    }

    static void HandleEffectProc(AuraScript const&, AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        Spell const* procSpell = eventInfo.GetProcSpell();
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_SHAMAN_AFTERSHOCK_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = procSpell,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, *procSpell->GetPowerTypeCostAmount(POWER_MAELSTROM) } }
        });
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_sha_aftershock::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_sha_aftershock::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 108281 - Ancestral Guidance
class spell_sha_ancestral_guidance : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_ANCESTRAL_GUIDANCE_HEAL });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_SHAMAN_ANCESTRAL_GUIDANCE_HEAL)
            return false;

        if (!eventInfo.GetHealInfo() && !eventInfo.GetDamageInfo())
            return false;

        return true;
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        int32 bp0 = CalculatePct(int32(eventInfo.GetDamageInfo() ? eventInfo.GetDamageInfo()->GetDamage() : eventInfo.GetHealInfo()->GetHeal()), aurEff->GetAmount());
        if (!bp0)
            return;

        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_SHAMAN_ANCESTRAL_GUIDANCE_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, bp0 } }
        });
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_sha_ancestral_guidance::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_sha_ancestral_guidance::HandleEffectProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 114911 - Ancestral Guidance Heal
class spell_sha_ancestral_guidance_heal : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_ANCESTRAL_GUIDANCE });
    }

    static void ResizeTargets(SpellScript const&, std::list<WorldObject*>& targets)
    {
        Trinity::SelectRandomInjuredTargets(targets, 3, true);
    }

    void Register() override
    {
        // real Ancestral Guidance Heal (114911) implicit target is TARGET_UNIT_DEST_AREA_ALLY,
        // not TARGET_UNIT_CASTER_AREA_RAID (per wago.tools SpellEffect data)
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_ancestral_guidance_heal::ResizeTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 198299 - Gathering Storms
class spell_sha_artifact_gathering_storms : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_GATHERING_STORMS, SPELL_SHAMAN_GATHERING_STORMS_BUFF });
    }

    bool Load() override
    {
        return GetCaster()->HasAuraEffect(SPELL_SHAMAN_GATHERING_STORMS, EFFECT_0);
    }

    void TriggerBuff(SpellEffIndex effIndex) const
    {
        AuraEffect const* gatheringStorms = GetCaster()->GetAuraEffect(SPELL_SHAMAN_GATHERING_STORMS, EFFECT_0);
        if (!gatheringStorms)
            return;

        GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_GATHERING_STORMS_BUFF, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, int32(gatheringStorms->GetAmount() * GetUnitTargetCountForEffect(effIndex)) } }
        });
    }

    void Register() override
    {
        // real Crash Lightning (187874) EFFECT_0 is SPELL_EFFECT_WEAPON_PERCENT_DAMAGE, not
        // SPELL_EFFECT_SCHOOL_DAMAGE (per wago.tools SpellEffect data)
        OnEffectHit += SpellEffectFn(spell_sha_artifact_gathering_storms::TriggerBuff, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// 114052 - Ascendance (Restoration)
class spell_sha_ascendance_restoration : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_RESTORATIVE_MISTS });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& procInfo)
    {
        return procInfo.GetHealInfo() && procInfo.GetHealInfo()->GetOriginalHeal() && procInfo.GetSpellInfo()->Id != SPELL_SHAMAN_RESTORATIVE_MISTS_INITIAL;
    }

    void OnProcHeal(AuraEffect const* aurEff, ProcEventInfo const& procInfo)
    {
        _healToDistribute += CalculatePct(int32(procInfo.GetHealInfo()->GetOriginalHeal()), aurEff->GetAmount());
    }

    void HandleEffectPeriodic(AuraEffect const* aurEff)
    {
        if (!_healToDistribute)
            return;

        GetTarget()->CastSpell(nullptr, SPELL_SHAMAN_RESTORATIVE_MISTS, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, _healToDistribute } }
        });
        _healToDistribute = 0;
    }

    // real Ascendance (Restoration, 114052) only has 2 effects (per wago.tools SpellEffect
    // data): EFFECT_0 (AuraName 56, not SPELL_AURA_DUMMY - the OnProcHeal hook below has no
    // confirmed matching effect/aura-type pair, left unresolved) and EFFECT_1 (SPELL_AURA_PERIODIC_DUMMY,
    // matching the periodic redistribution hook - was wrongly bound to EFFECT_6).
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_sha_ascendance_restoration::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_sha_ascendance_restoration::OnProcHeal, EFFECT_8, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_ascendance_restoration::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    int32 _healToDistribute = 0;
};

// 2825 - Bloodlust, 32182 - Heroism
// Removed: this class duplicated spell_gen_bloodlust (spell_generic.cpp) and both were
// registered under the same "spell_sha_bloodlust"/"spell_sha_heroism" names, crashing
// ScriptMgr::AddScript with a duplicate-registration abort. spell_gen_bloodlust is the one kept -
// it's reused across specs (shaman Bloodlust/Heroism, mage Time Warp, the Drums item) via a
// parameterized exhaustion-spell-id constructor, where this class was shaman-only/hardcoded. Its
// one real advantage (excluding Hunter's Insanity and the Netherwinds spirit beast's Fatigued
// debuff, which spell_gen_bloodlust's filter was missing) was folded into spell_gen_bloodlust
// instead of kept here.

// 196884 - Feral Lunge
class spell_sha_feral_lunge : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_FERAL_LUNGE_DAMAGE });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/) const
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        GetCaster()->CastSpell(target, SPELL_SHAMAN_FERAL_LUNGE_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_feral_lunge::HandleDamage, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 98020 - Spirit Link Totem
// Averages the health percentage of nearby raid members, then heals/damages each toward that
// average.
class spell_sha_spirit_link : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_SPIRIT_LINK_HEAL });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        float totalPct = 0.0f;
        uint32 targetCount = 0;
        for (WorldObject* obj : targets)
        {
            if (Unit* target = obj->ToUnit())
            {
                _healthPct[target->GetGUID()] = target->GetHealthPct();
                totalPct += target->GetHealthPct();
                ++targetCount;
            }
        }

        if (targetCount)
            _averagePct = totalPct / targetCount;
    }

    void HandleOnHit() const
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        auto itr = _healthPct.find(target->GetGUID());
        if (itr == _healthPct.end())
            return;

        uint64 currentHealth = CalculatePct(target->GetMaxHealth(), itr->second);
        uint64 desiredHealth = CalculatePct(target->GetMaxHealth(), _averagePct);

        GetCaster()->CastSpell(target, SPELL_SHAMAN_SPIRIT_LINK_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .SpellValueOverrides =
            {
                { SPELLVALUE_BASE_POINT0, int32(currentHealth > desiredHealth ? currentHealth - desiredHealth : 0) },
                { SPELLVALUE_BASE_POINT1, int32(desiredHealth > currentHealth ? desiredHealth - currentHealth : 0) }
            }
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_spirit_link::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
        OnHit += SpellHitFn(spell_sha_spirit_link::HandleOnHit);
    }

private:
    std::unordered_map<ObjectGuid, float> _healthPct;
    float _averagePct = 0.0f;
};

// 188443 - Chain Lightning
class spell_sha_chain_lightning_crash_lightning : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_CRASH_LIGHTNING, SPELL_SHAMAN_CRASH_LIGHTNING_DAMAGE_BUFF });
    }

    bool Load() override
    {
        return GetCaster()->HasSpell(SPELL_SHAMAN_CRASH_LIGHTNING);
    }

    void HandleCooldownReduction(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->GetSpellHistory()->ModifyCooldown(SPELL_SHAMAN_CRASH_LIGHTNING, Milliseconds(-GetEffectValue()) * GetUnitTargetCountForEffect(EFFECT_0));
    }

    void HandleDamageBuff(SpellEffIndex effIndex) const
    {
        int64 targetsHit = GetUnitTargetCountForEffect(effIndex);
        if (targetsHit > 1)
            GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_CRASH_LIGHTNING_DAMAGE_BUFF, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                .SpellValueOverrides = { { SPELLVALUE_AURA_STACK, int32(targetsHit) } }
            });
    }

    void Register() override
    {
        // real Chain Lightning (188443) only has 2 effects (EFFECT_0/EFFECT_1) - no EFFECT_2
        OnEffectLaunch += SpellEffectFn(spell_sha_chain_lightning_crash_lightning::HandleCooldownReduction, EFFECT_1, SPELL_EFFECT_DUMMY);
        OnEffectLaunch += SpellEffectFn(spell_sha_chain_lightning_crash_lightning::HandleDamageBuff, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 188443 - Chain Lightning
// Same fix as spell_sha_chain_lightning_overload below: real Chain Lightning energize (195897,
// confirmed via build-pinned 7.3.5.26972 data - "Gaining Maelstrom", a self-contained
// SPELL_EFFECT_ENERGIZE with its own base points, 6) needs no external MAELSTROM_CONTROLLER
// (190488) reading - that spell's real EFFECT_4 doesn't exist (only EFFECT_0/EFFECT_1 do, used
// correctly elsewhere for Lightning Bolt/Lightning Bolt Overload).
class spell_sha_chain_lightning_energize : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_CHAIN_LIGHTNING_ENERGIZE })
            && ValidateSpellEffect({ { SPELL_SHAMAN_CHAIN_LIGHTNING_ENERGIZE, EFFECT_0 } });
    }

    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        SpellInfo const* energizeSpell = sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_CHAIN_LIGHTNING_ENERGIZE, GetCastDifficulty());
        int32 perTarget = energizeSpell->GetEffect(EFFECT_0).CalcValue(caster);

        caster->CastSpell(caster, SPELL_SHAMAN_CHAIN_LIGHTNING_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell(),
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, perTarget * GetUnitTargetCountForEffect(EFFECT_0) } }
        });
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_sha_chain_lightning_energize::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 45297 - Chain Lightning Overload
// Real Chain Lightning Overload energize (218558, confirmed via build-pinned 7.3.5.26972 data) is
// a self-contained SPELL_EFFECT_ENERGIZE spell with its own base points (4 Maelstrom) - it needs
// no external "controller" aura to read a per-target amount from. SPELL_SHAMAN_MAELSTROM_CONTROLLER
// (190488) was the wrong spell for this: it only has EFFECT_0/EFFECT_1 in real data (not EFFECT_5
// as this class used to expect), and those two real effects trigger "Fulmination"/"Fulmination!",
// an unrelated Earth Shock mechanic. Matches Chain Lightning's own base spell (188443), whose
// tooltip is "Generates X Maelstrom per target hit" - same per-target-flat-amount design, mirrored
// here for the Overload copy.
class spell_sha_chain_lightning_overload : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_ENERGIZE })
            && ValidateSpellEffect({ { SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_ENERGIZE, EFFECT_0 } });
    }

    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        SpellInfo const* energizeSpell = sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_ENERGIZE, GetCastDifficulty());
        int32 perTarget = energizeSpell->GetEffect(EFFECT_0).CalcValue(caster);

        caster->CastSpell(caster, SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell(),
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, perTarget * GetUnitTargetCountForEffect(EFFECT_0) } }
        });
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_sha_chain_lightning_overload::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 157503 - Cloudburst Totem
// Splits its heal evenly across all targets hit.
class spell_sha_cloudburst : public SpellScript
{
    void CountTargets(std::list<WorldObject*>& targets)
    {
        _targetCount = uint32(targets.size());
    }

    void HandleHeal(SpellEffIndex /*effIndex*/)
    {
        if (_targetCount)
            SetHitHeal(GetHitHeal() / _targetCount);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_cloudburst::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        OnEffectHitTarget += SpellEffectFn(spell_sha_cloudburst::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
    }

private:
    uint32 _targetCount = 0;
};

// 157504 - Cloudburst Totem (storage effect)
// While active, stores a percentage (read from 157503's own EFFECT_0, which carries no other use
// of its own) of the shaman's healing into this aura's own accumulator, then releases the total
// as a single burst heal via 157503 (which itself splits it across nearby allies) when it ends.
class spell_sha_cloudburst_effect : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TOTEM_CLOUDBURST }) && ValidateSpellEffect({ { SPELL_TOTEM_CLOUDBURST, EFFECT_0 } });
    }

    void OnProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        int32 storagePct = sSpellMgr->AssertSpellInfo(SPELL_TOTEM_CLOUDBURST, DIFFICULTY_NONE)->GetEffect(EFFECT_0).CalcValue();
        aurEff->ChangeAmount(aurEff->GetAmount() + CalculatePct(int32(healInfo->GetHeal()), storagePct));
    }

    void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/) const
    {
        Unit* owner = GetUnitOwner();
        int32 amount = aurEff->GetAmount();
        if (!owner || amount <= 0)
            return;

        owner->CastSpell(owner, SPELL_TOTEM_CLOUDBURST, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, amount } }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_cloudburst_effect::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_sha_cloudburst_effect::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 202192 - Resonance Totem
class spell_sha_resonance_effect : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/) const
    {
        Unit* totem = GetCaster();
        if (Unit* owner = totem ? totem->GetOwner() : nullptr)
            owner->ModifyPower(POWER_MAELSTROM, 1);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_resonance_effect::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
    }
};

// 51485 - Earthgrab Totem
// The first root a target takes grants a DR-style immunity; while immune, further hits apply
// Earthbind (a snare) instead of re-rooting them.
class spell_sha_earthgrab : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_EARTHGRAB_IMMUNITY, SPELL_SHAMAN_EARTHBIND_FOR_EARTHGRAB_TOTEM });
    }

    void HandleOnHit() const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (target->HasAura(SPELL_SHAMAN_EARTHGRAB_IMMUNITY, caster->GetGUID()))
            caster->CastSpell(target, SPELL_SHAMAN_EARTHBIND_FOR_EARTHGRAB_TOTEM, true);
        else
            caster->CastSpell(target, SPELL_SHAMAN_EARTHGRAB_IMMUNITY, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_sha_earthgrab::HandleOnHit);
    }
};

// 201846 - Stormsurge
class spell_sha_delayed_stormstrike_mod_charge_drop_proc : public AuraScript
{
    void DropAura(ProcEventInfo const& /*eventInfo*/) const
    {
        GetAura()->DropChargeDelayed(1);
    }

    void Register() override
    {
        AfterProc += AuraProcFn(spell_sha_delayed_stormstrike_mod_charge_drop_proc::DropAura);
    }
};

// 187874 - Crash Lightning
class spell_sha_crash_lightning : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_CRASH_LIGHTNING_CLEAVE });
    }

    void TriggerCleaveBuff(SpellEffIndex effIndex) const
    {
        if (GetUnitTargetCountForEffect(effIndex) >= 2)
            GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_CRASH_LIGHTNING_CLEAVE, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_DONT_REPORT_CAST_ERROR,
                .TriggeringSpell = GetSpell()
            });
    }

    void Register() override
    {
        // real Crash Lightning (187874) EFFECT_0 is SPELL_EFFECT_WEAPON_PERCENT_DAMAGE, not
        // SPELL_EFFECT_SCHOOL_DAMAGE (per wago.tools SpellEffect data)
        OnEffectHit += SpellEffectFn(spell_sha_crash_lightning::TriggerCleaveBuff, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// 197211 - Fury of Air (Enhancement talent): periodic AOE wind damage, draining 5 Maelstrom
// per tick and cancelling itself once Maelstrom runs out. The damage tick itself is the
// aura's own SPELL_AURA_PERIODIC_TRIGGER_SPELL effect data, needing no script. Confirmed via
// two independent reference sources (identical implementations).
class spell_sha_fury_of_air : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->GetPower(POWER_MAELSTROM) >= 5)
            caster->ModifyPower(POWER_MAELSTROM, -5);
        else
            caster->RemoveAura(SPELL_SHAMAN_FURY_OF_AIR);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_fury_of_air::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 201633 - Earthen Shield Totem (redirect-absorb buff): applied to nearby allies by the
// totem's own AI (npc_pet_shaman_earthen_shield_totem in pet_shaman.cpp). Confirmed via
// two independent reference sources (identical implementations) - the absorb cap is the totem's own
// current health (self-limiting: the totem soaks damage until it runs out), and each hit's
// redirected amount is separately capped by the shaman's spell power before being dealt back
// to the totem as real damage. Unlike the reference, this does not rely on the buff's own
// GetCaster() literally being the totem (this project's version applies the buff with the
// shaman as caster, simpler) - both hooks instead resolve the shaman's own totem fresh via
// Unit::m_Controlled each time (the same idiom used for Implosion/Dancing Rune Weapon/
// Transcendence elsewhere this session), decoupling the redirect target from whoever cast the
// buff. Translated CastCustomSpell (doesn't exist in ArgusCore) to
// CastSpellExtraArgs/AddSpellMod, and Unit::GetTotalSpellPowerValue (doesn't exist) to
// Unit::SpellBaseDamageBonusDone, this project's established spell-power-reading idiom
// (already used elsewhere in this same file and for Mage's Frost Bomb this session).
class spell_sha_earthen_shield_absorb : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_EARTHEN_SHIELD_ABSORB_DAMAGE });
    }

    static Creature* FindTotem(Unit* shaman)
    {
        for (Unit* controlled : shaman->m_Controlled)
            if (controlled->GetEntry() == NPC_SHAMAN_EARTHEN_SHIELD_TOTEM)
                if (Creature* totem = controlled->ToCreature())
                    return totem;

        return nullptr;
    }

    void CalcAbsorb(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* caster = GetCaster();
        Creature* totem = caster ? FindTotem(caster) : nullptr;
        amount = totem ? int32(totem->GetHealth()) : 0;
    }

    void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        absorbAmount = 0;

        Unit* caster = GetCaster();
        Creature* totem = caster ? FindTotem(caster) : nullptr;
        if (!caster || !totem)
            return;

        int32 spellPower = int32(caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL));
        absorbAmount = std::min<uint32>(dmgInfo.GetDamage(), uint32(std::max(spellPower, 0)));

        if (totem->GetHealth() <= absorbAmount)
        {
            absorbAmount = uint32(totem->GetHealth());
            totem->DespawnOrUnsummon();
        }
        else
        {
            totem->CastSpell(totem, SPELL_SHAMAN_EARTHEN_SHIELD_ABSORB_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, int32(absorbAmount)));
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_sha_earthen_shield_absorb::CalcAbsorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_sha_earthen_shield_absorb::HandleAbsorb, EFFECT_0);
    }
};

// 200076 - Deluge (attached to 77472 - Healing Wave, 8004 - Healing Surge and 1064 - Chain Heal
class spell_sha_deluge : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_RIPTIDE, SPELL_SHAMAN_DELUGE_AURA })
            && ValidateSpellEffect({ { SPELL_SHAMAN_DELUGE_TALENT, EFFECT_0 } });
    }

    void CalculateHealingBonus(SpellEffectInfo const& /*spellEffectInfo*/, Unit const* victim, int32& /*healing*/, int32& /*flatMod*/, float& pctMod) const
    {
        if (AuraEffect const* deluge = GetCaster()->GetAuraEffect(SPELL_SHAMAN_DELUGE_TALENT, EFFECT_0))
            if (victim->GetAura(SPELL_SHAMAN_RIPTIDE, GetCaster()->GetGUID()) || victim->GetAura(SPELL_SHAMAN_DELUGE_AURA, GetCaster()->GetGUID()))
                AddPct(pctMod, deluge->GetAmount());
    }

    void Register() override
    {
        CalcHealing += SpellCalcHealingFn(spell_sha_deluge::CalculateHealingBonus);
    }
};

namespace HealingRain
{
Position GetHealingRainPosition(Aura const* healingRain);
}

// 200075 - Deluge (attached to 73920 - Healing Rain)
class spell_sha_deluge_healing_rain : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_DELUGE_TALENT, SPELL_SHAMAN_DELUGE_AURA });
    }

    bool Load() override
    {
        return GetUnitOwner()->HasAura(SPELL_SHAMAN_DELUGE_TALENT);
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/) const
    {
        GetCaster()->CastSpell(HealingRain::GetHealingRainPosition(GetAura()), SPELL_SHAMAN_DELUGE_AURA, TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_deluge_healing_rain::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 207778 - Downpour
class spell_sha_downpour : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    static void FilterTargets(SpellScript const&, std::list<WorldObject*>& targets)
    {
        Trinity::SelectRandomInjuredTargets(targets, 6, true);
    }

    void CountEffectivelyHealedTarget()
    {
        // Cooldown increased for each target effectively healed
        if (GetHitHeal())
            ++_healedTargets;
    }

    void HandleCooldown() const
    {
        SpellHistory::Duration cooldown = Milliseconds(GetSpellInfo()->RecoveryTime) + Seconds(GetEffectInfo(EFFECT_1).CalcValue() * _healedTargets);
        GetCaster()->GetSpellHistory()->StartCooldown(GetSpellInfo(), 0, GetSpell(), false, cooldown);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_downpour::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        AfterHit += SpellHitFn(spell_sha_downpour::CountEffectivelyHealedTarget);
        AfterCast += SpellCastFn(spell_sha_downpour::HandleCooldown);
    }

    int32 _healedTargets = 0;
};

// 198103 - Earth Elemental
class spell_sha_earth_elemental : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_EARTH_ELEMENTAL_SUMMON });
    }

    void HandleSummon(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_SHAMAN_EARTH_ELEMENTAL_SUMMON, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_earth_elemental::HandleSummon, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 198067 - Fire Elemental
class spell_sha_fire_elemental : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_FIRE_ELEMENTAL_SUMMON });
    }

    void HandleSummon(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_SHAMAN_FIRE_ELEMENTAL_SUMMON, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_fire_elemental::HandleSummon, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 204288 - Earth Shield
class spell_sha_earth_shield : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_EARTH_SHIELD_HEAL });
    }

    bool CheckProc(ProcEventInfo const& eventInfo) const
    {
        if (!eventInfo.GetDamageInfo() || !HasEffect(EFFECT_1) || eventInfo.GetDamageInfo()->GetDamage() < GetTarget()->CountPctFromMaxHealth(GetEffect(EFFECT_1)->GetAmount()))
            return false;
        return true;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/)
    {
        PreventDefaultAction();

        GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_EARTH_SHIELD_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff,
            .OriginalCaster = GetCasterGUID()
        });
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_sha_earth_shield::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_sha_earth_shield::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 170374 - Earthen Rage (Passive)
class spell_sha_earthen_rage_passive : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_EARTHEN_RAGE_PERIODIC, SPELL_SHAMAN_EARTHEN_RAGE_DAMAGE });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& procInfo)
    {
        return procInfo.GetSpellInfo() && procInfo.GetSpellInfo()->Id != SPELL_SHAMAN_EARTHEN_RAGE_DAMAGE;
    }

    void HandleEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        _procTargetGuid = eventInfo.GetProcTarget()->GetGUID();
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_SHAMAN_EARTHEN_RAGE_PERIODIC, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK
        });
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_sha_earthen_rage_passive::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_sha_earthen_rage_passive::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

    ObjectGuid _procTargetGuid;

public:
    ObjectGuid const& GetProcTargetGuid() const
    {
        return _procTargetGuid;
    }
};

// 170377 - Earthen Rage (Proc Aura)
class spell_sha_earthen_rage_proc_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_EARTHEN_RAGE_PASSIVE, SPELL_SHAMAN_EARTHEN_RAGE_DAMAGE });
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
        if (Aura const* aura = GetCaster()->GetAura(SPELL_SHAMAN_EARTHEN_RAGE_PASSIVE))
            if (spell_sha_earthen_rage_passive* script = aura->GetScript<spell_sha_earthen_rage_passive>())
                if (Unit* procTarget = ObjectAccessor::GetUnit(*GetCaster(), script->GetProcTargetGuid()))
                    GetTarget()->CastSpell(procTarget, SPELL_SHAMAN_EARTHEN_RAGE_DAMAGE, CastSpellExtraArgsInit{
                        .TriggerFlags = TRIGGERED_FULL_MASK
                    });
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_earthen_rage_proc_aura::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 61882 - Earthquake
//  8382 - AreaTriggerId
struct areatrigger_sha_earthquake : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnCreate(Spell const* creatingSpell) override
    {
        if (Unit* caster = at->GetCaster())
            if (AuraEffect const* earthquake = caster->GetAuraEffect(SPELL_SHAMAN_EARTHQUAKE, EFFECT_1))
                _period = Milliseconds(earthquake->GetPeriod());

        if (creatingSpell)
            if (float const* damageMultiplier = std::any_cast<float>(&creatingSpell->m_customArg))
                _damageMultiplier = *damageMultiplier;
    }

    void OnUpdate(uint32 diff) override
    {
        _refreshTimer -= Milliseconds(diff);
        while (_refreshTimer <= 0s)
        {
            if (Unit* caster = at->GetCaster())
                caster->CastSpell(at->GetPosition(), SPELL_SHAMAN_EARTHQUAKE_TICK, CastSpellExtraArgsInit{
                    .TriggerFlags = TRIGGERED_FULL_MASK,
                    .OriginalCaster = at->GetGUID(),
                    .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, int32(caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NATURE) * 0.213f * _damageMultiplier) } }
                });

            _refreshTimer += _period;
        }
    }

    // Each target can only be stunned once by each earthquake - keep track of who we already stunned
    bool AddStunnedTarget(ObjectGuid const& guid)
    {
        return _stunnedUnits.insert(guid).second;
    }

private:
    Milliseconds _refreshTimer = 0s;
    Milliseconds _period = 1s;
    GuidUnorderedSet _stunnedUnits;
    float _damageMultiplier = 1.0f;
};

// 77478 - Earthquake tick
class spell_sha_earthquake_tick : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_EARTHQUAKE_KNOCKING_DOWN })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void HandleOnHit() const
    {
        if (Unit* target = GetHitUnit())
        {
            if (roll_chance_i(GetEffectInfo(EFFECT_1).CalcValue()))
            {
                std::vector<AreaTrigger*> areaTriggers = GetCaster()->GetAreaTriggers(SPELL_SHAMAN_EARTHQUAKE);
                auto itr = std::ranges::find(areaTriggers, GetSpell()->GetOriginalCasterGUID(), [](AreaTrigger const* at) { return at->GetGUID(); });
                if (itr != areaTriggers.end())
                    if (areatrigger_sha_earthquake* eq = CAST_AI(areatrigger_sha_earthquake, (*itr)->AI()))
                        if (eq->AddStunnedTarget(target->GetGUID()))
                            GetCaster()->CastSpell(target, SPELL_SHAMAN_EARTHQUAKE_KNOCKING_DOWN, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            }
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_sha_earthquake_tick::HandleOnHit);
    }
};

// 117014 - Elemental Blast
// 120588 - Elemental Blast Overload
class spell_sha_elemental_blast : public SpellScript
{
    static constexpr std::array<uint32, 3> BuffSpells = { SPELL_SHAMAN_ELEMENTAL_BLAST_CRIT, SPELL_SHAMAN_ELEMENTAL_BLAST_HASTE, SPELL_SHAMAN_ELEMENTAL_BLAST_MASTERY };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_SHAMAN_ELEMENTAL_BLAST_CRIT,
            SPELL_SHAMAN_ELEMENTAL_BLAST_HASTE,
            SPELL_SHAMAN_ELEMENTAL_BLAST_MASTERY
        });
    }

    void TriggerBuff() const
    {
        Unit* caster = GetCaster();
        std::array<double, BuffSpells.size()> weights;
        double total = 0.0;
        for (std::size_t i = 0; i < BuffSpells.size(); ++i)
            total += weights[i] = !caster->HasAura(BuffSpells[i]) ? 1.0 : 0.0;

        uint32 spellId = [&]
        {
            if (total > 0.0)
                return *Trinity::Containers::SelectRandomWeightedContainerElement(BuffSpells, weights);

            // refresh random one if we have them all
            return Trinity::Containers::SelectRandomContainerElement(BuffSpells);
        }();

        GetCaster()->CastSpell(GetCaster(), spellId, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_sha_elemental_blast::TriggerBuff);
    }
};

// 318038 - Flametongue Weapon
class spell_sha_flametongue_weapon : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_FLAMETONGUE_WEAPON_ENCHANT });
    }

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetCaster()->ToPlayer();
        uint8 slot = EQUIPMENT_SLOT_MAINHAND;
        if (player->GetPrimarySpecialization() == ChrSpecialization::ShamanEnhancement)
            slot = EQUIPMENT_SLOT_OFFHAND;

        Item* targetItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!targetItem || !targetItem->GetTemplate()->IsWeapon())
            return;

        player->CastSpell(targetItem, SPELL_SHAMAN_FLAMETONGUE_WEAPON_ENCHANT, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_flametongue_weapon::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 319778  - Flametongue - SPELL_SHAMAN_FLAMETONGUE_WEAPON_AURA
class spell_sha_flametongue_weapon_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_FLAMETONGUE_ATTACK });
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        eventInfo.GetActor()->CastSpell(eventInfo.GetActionTarget(), SPELL_SHAMAN_FLAMETONGUE_ATTACK, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_flametongue_weapon_aura::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 196834 - Frostbrand (Hailstorm's proc source, 210853/210854)
class spell_sha_hailstorm : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_HAILSTORM_TALENT, SPELL_SHAMAN_HAILSTORM_BUFF });
    }

    void HandleProc(ProcEventInfo const& /*eventInfo*/) const
    {
        if (GetTarget()->HasAura(SPELL_SHAMAN_HAILSTORM_TALENT))
            GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_HAILSTORM_BUFF, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR
            });
    }

    void Register() override
    {
        OnProc += AuraProcFn(spell_sha_hailstorm::HandleProc);
    }
};

// 73920 - Healing Rain (Aura)
class spell_sha_healing_rain_aura : public AuraScript
{
public:
    void SetVisualDummy(TempSummon const* summon)
    {
        _visualDummy = summon->GetGUID();
        _dest = summon->GetPosition();
    }

    Position GetPosition() const { return _dest; }

private:
    void HandleEffectPeriodic(AuraEffect const* aurEff) const
    {
        GetTarget()->CastSpell(_dest, SPELL_SHAMAN_HEALING_RAIN_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff
        });
    }

    void HandleEffecRemoved(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (Creature* summon = ObjectAccessor::GetCreature(*GetTarget(), _visualDummy))
            summon->DespawnOrUnsummon();
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_sha_healing_rain_aura::HandleEffecRemoved, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_healing_rain_aura::HandleEffectPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }

    ObjectGuid _visualDummy;
    Position _dest;
};

Position HealingRain::GetHealingRainPosition(Aura const* healingRain)
{
    if (spell_sha_healing_rain_aura const* script = healingRain->GetScript<spell_sha_healing_rain_aura>())
        return script->GetPosition();

    return healingRain->GetUnitOwner()->GetPosition();
}

// 73920 - Healing Rain
class spell_sha_healing_rain : public SpellScript
{
    void InitializeVisualStalker() const
    {
        if (Aura* aura = GetHitAura())
        {
            if (WorldLocation const* dest = GetExplTargetDest())
            {
                Milliseconds duration = Milliseconds(GetSpellInfo()->CalcDuration(GetOriginalCaster()));
                TempSummon* summon = GetCaster()->GetMap()->SummonCreature(NPC_HEALING_RAIN_INVISIBLE_STALKER, *dest, nullptr, duration, GetOriginalCaster());
                if (!summon)
                    return;

                summon->CastSpell(summon, SPELL_SHAMAN_HEALING_RAIN_VISUAL, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });

                if (spell_sha_healing_rain_aura* script = aura->GetScript<spell_sha_healing_rain_aura>())
                    script->SetVisualDummy(summon);
            }
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_sha_healing_rain::InitializeVisualStalker);
    }
};

// 73921 - Healing Rain
class spell_sha_healing_rain_target_limit : public SpellScript
{
    static void SelectTargets(SpellScript const&, std::list<WorldObject*>& targets)
    {
        Trinity::SelectRandomInjuredTargets(targets, 6, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_healing_rain_target_limit::SelectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 52042 - Healing Stream Totem
class spell_sha_healing_stream_totem_heal : public SpellScript
{
    static void SelectTargets(SpellScript const&, std::list<WorldObject*>& targets)
    {
        Trinity::SelectRandomInjuredTargets(targets, 1, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_healing_stream_totem_heal::SelectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 201900 - Hot Hand
class spell_sha_hot_hand : public AuraScript
{
    static bool CheckProc(AuraScript const&, AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetActor()->HasAura(SPELL_SHAMAN_FLAMETONGUE_WEAPON_AURA);
    }

    void Register() override
    {
        // real Hot Hand (201900) EFFECT_0 is SPELL_AURA_PROC_TRIGGER_SPELL, not SPELL_AURA_DUMMY
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_sha_hot_hand::CheckProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};


// 210714 - Icefury: removed spell_sha_icefury entirely. Real 210714 has exactly 3 effects
// (confirmed via build-pinned 7.3.5.26972 SpellEffect data), not the 4 this class's premise
// assumed: EFFECT_0 is the direct Frost damage (native SCHOOL_DAMAGE), EFFECT_1 is a plain
// SPELL_EFFECT_ENERGIZE (Maelstrom, fires natively and immediately on cast - matches the tooltip's
// own "Generates X Maelstrom" line, no companion spell involved), and EFFECT_2 is a native
// SPELL_AURA_ADD_PCT_MODIFIER classmask-restricted to Frost Shock (the "next N Frost Shocks deal
// increased damage" buff, auto-applied/consumed by the engine's generic SpellMod system with no
// script needed). There is no fourth effect and no separate "energize on next Frost Shock cast"
// mechanic to reproduce - SPELL_SHAMAN_FROST_SHOCK_ENERGIZE (289439, confirmed absent under any id
// across three separate 7.3.5 client builds) was chasing a premise the real spell doesn't have.
// The class's own Validate() already permanently failed on that dependency, so this was already
// dead code with zero live impact.

// 23551 - Lightning Shield T2 Bonus
class spell_sha_item_lightning_shield : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_item_lightning_shield::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 23552 - Lightning Shield T2 Bonus
class spell_sha_item_lightning_shield_trigger : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD_DAMAGE });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_item_lightning_shield_trigger::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 23572 - Mana Surge
class spell_sha_item_mana_surge : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_ITEM_MANA_SURGE });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetProcSpell() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        if (Optional<int32> manaCost = eventInfo.GetProcSpell()->GetPowerTypeCostAmount(POWER_MANA))
        {
            int32 mana = CalculatePct(*manaCost, 35);
            if (mana > 0)
            {
                GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_ITEM_MANA_SURGE, CastSpellExtraArgsInit{
                    .TriggerFlags = TRIGGERED_FULL_MASK,
                    .TriggeringAura = aurEff,
                    .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, mana } }
                });
            }
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_sha_item_mana_surge::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_sha_item_mana_surge::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 40463 - Shaman Tier 6 Trinket
class spell_sha_item_t6_trinket : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_SHAMAN_ENERGY_SURGE,
            SPELL_SHAMAN_POWER_SURGE
        });
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return;

        uint32 spellId;
        int32 chance;

        // Lesser Healing Wave
        if (spellInfo->SpellFamilyFlags[0] & 0x00000080)
        {
            spellId = SPELL_SHAMAN_ENERGY_SURGE;
            chance = 10;
        }
        // Lightning Bolt
        else if (spellInfo->SpellFamilyFlags[0] & 0x00000001)
        {
            spellId = SPELL_SHAMAN_ENERGY_SURGE;
            chance = 15;
        }
        // Stormstrike
        else if (spellInfo->SpellFamilyFlags[1] & 0x00000010)
        {
            spellId = SPELL_SHAMAN_POWER_SURGE;
            chance = 50;
        }
        else
            return;

        if (roll_chance_i(chance))
            eventInfo.GetActor()->CastSpell(nullptr, spellId, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_item_t6_trinket::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 70811 - Item - Shaman T10 Elemental 2P Bonus
class spell_sha_item_t10_elemental_2p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_ELEMENTAL_MASTERY });
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/)
    {
        PreventDefaultAction();
        if (Player* target = GetTarget()->ToPlayer())
            target->GetSpellHistory()->ModifyCooldown(SPELL_SHAMAN_ELEMENTAL_MASTERY, Milliseconds(-aurEff->GetAmount()));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_item_t10_elemental_2p_bonus::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 189063 - Lightning Vortex (proc 185881 Item - Shaman T18 Elemental 4P Bonus)
class spell_sha_item_t18_elemental_4p_bonus : public AuraScript
{
    void DiminishHaste(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        if (AuraEffect* hasteBuff = GetEffect(EFFECT_0))
            hasteBuff->ChangeAmount(hasteBuff->GetAmount() - aurEff->GetAmount());
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_item_t18_elemental_4p_bonus::DiminishHaste, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 51505 - Lava burst
class spell_sha_lava_burst : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_PATH_OF_FLAMES_TALENT, SPELL_SHAMAN_PATH_OF_FLAMES_SPREAD, SPELL_SHAMAN_LAVA_SURGE });
    }

    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_SHAMAN_PATH_OF_FLAMES_TALENT))
                caster->CastSpell(GetHitUnit(), SPELL_SHAMAN_PATH_OF_FLAMES_SPREAD, CastSpellExtraArgsInit{
                    .TriggerFlags = TRIGGERED_FULL_MASK,
                    .TriggeringSpell = GetSpell()
                });
    }

    void EnsureLavaSurgeCanBeImmediatelyConsumed() const
    {
        Unit* caster = GetCaster();

        if (Aura* lavaSurge = caster->GetAura(SPELL_SHAMAN_LAVA_SURGE))
        {
            if (!GetSpell()->m_appliedMods.contains(lavaSurge))
            {
                uint32 chargeCategoryId = GetSpellInfo()->ChargeCategoryId;

                // Ensure we have at least 1 usable charge after cast to allow next cast immediately
                if (!caster->GetSpellHistory()->HasCharge(chargeCategoryId))
                    caster->GetSpellHistory()->RestoreCharge(chargeCategoryId);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_lava_burst::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterCast += SpellCastFn(spell_sha_lava_burst::EnsureLavaSurgeCanBeImmediatelyConsumed);
    }
};

// 285452 - Lava Burst damage
// 285466 - Lava Burst Overload damage
class spell_sha_lava_crit_chance : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_LAVA_BURST_RANK_2, SPELL_SHAMAN_FLAME_SHOCK });
    }

    void CalcCritChance(Unit const* victim, float& chance) const
    {
        Unit* caster = GetCaster();

        if (!caster || !victim)
            return;

        if (caster->HasAura(SPELL_SHAMAN_LAVA_BURST_RANK_2) && victim->HasAura(SPELL_SHAMAN_FLAME_SHOCK, caster->GetGUID()))
            if (victim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE) > -100)
                chance = 100.f;
    }

    void Register() override
    {
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_sha_lava_crit_chance::CalcCritChance);
    }
};

// 60103 - Lava Lash
class spell_sha_lava_lash : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } })
            && ValidateSpellInfo({ SPELL_SHAMAN_FLAMETONGUE_WEAPON_AURA });
    }

    bool Load() override
    {
        return GetCaster()->IsPlayer();
    }

    void AddBonusFlametongueDamage(SpellEffectInfo const& /*effectInfo*/, Unit const* victim, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        Player* caster = GetCaster()->ToPlayer();
        ObjectGuid offhandItemGuid = Object::GetGUID(caster->GetWeaponForAttack(GetSpellInfo()->GetAttackType()));
        if (GetCaster()->HasAura(SPELL_SHAMAN_FLAMETONGUE_WEAPON_AURA, ObjectGuid::Empty, offhandItemGuid))
            AddPct(pctMod, GetSpell()->CalculateDamage(GetEffectInfo(EFFECT_1), victim));
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_sha_lava_lash::AddBonusFlametongueDamage);
    }
};

// 77756 - Lava Surge
class spell_sha_lava_surge : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_LAVA_SURGE });
    }

    // Igneous Potential (a Lava Burst-crit proc-chance bonus to this same roll) doesn't exist
    // under any id in this build - forward drift from a later expansion's Elemental rework.
    bool CheckProcChance(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/) const
    {
        return roll_chance_i(aurEff->GetAmount());
    }

    void HandleEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& /*eventInfo*/)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_LAVA_SURGE, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_sha_lava_surge::CheckProcChance, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_sha_lava_surge::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 77762 - Lava Surge
class spell_sha_lava_surge_proc : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_LAVA_BURST });
    }

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void ResetCooldown() const
    {
        GetCaster()->GetSpellHistory()->RestoreCharge(sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_LAVA_BURST, GetCastDifficulty())->ChargeCategoryId);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_sha_lava_surge_proc::ResetCooldown);
    }
};

// 188196 - Lightning Bolt
class spell_sha_lightning_bolt : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_LIGHTNING_BOLT_ENERGIZE })
            && ValidateSpellEffect({ { SPELL_SHAMAN_MAELSTROM_CONTROLLER, EFFECT_0 } });
    }

    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        if (AuraEffect const* energizeAmount = GetCaster()->GetAuraEffect(SPELL_SHAMAN_MAELSTROM_CONTROLLER, EFFECT_0))
            GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_LIGHTNING_BOLT_ENERGIZE, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                .TriggeringAura = energizeAmount,
                .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, energizeAmount->GetAmount() } }
            });
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_sha_lightning_bolt::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 45284 - Lightning Bolt Overload
class spell_sha_lightning_bolt_overload : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_ENERGIZE })
            && ValidateSpellEffect({ { SPELL_SHAMAN_MAELSTROM_CONTROLLER, EFFECT_1 } });
    }

    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        if (AuraEffect const* energizeAmount = GetCaster()->GetAuraEffect(SPELL_SHAMAN_MAELSTROM_CONTROLLER, EFFECT_1))
            GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_ENERGIZE, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                .TriggeringAura = energizeAmount,
                .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, energizeAmount->GetAmount() } }
            });
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_sha_lightning_bolt_overload::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 192223 - Liquid Magma Totem (erupting hit spell)
class spell_sha_liquid_magma_totem : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_LIQUID_MAGMA_HIT });
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/) const
    {
        if (Unit* hitUnit = GetHitUnit())
            GetCaster()->CastSpell(hitUnit, SPELL_SHAMAN_LIQUID_MAGMA_HIT, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    static void HandleTargetSelect(SpellScript const&, std::list<WorldObject*>& targets)
    {
        // choose one random target from targets
        if (targets.size() > 1)
        {
            WorldObject* selected = Trinity::Containers::SelectRandomContainerElement(targets);
            targets.clear();
            targets.push_back(selected);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_liquid_magma_totem::HandleTargetSelect, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_sha_liquid_magma_totem::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 187880 - Maelstrom Weapon
class spell_sha_maelstrom_weapon : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_MAELSTROM_WEAPON_ENERGIZE });
    }

    bool CheckProc(ProcEventInfo const& eventInfo) const
    {
        if (DamageInfo const* damageInfo = eventInfo.GetDamageInfo())
            if (damageInfo->GetAttackType() == BASE_ATTACK || damageInfo->GetAttackType() == OFF_ATTACK)
                return true;

        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id == SPELL_SHAMAN_WINDFURY_ATTACK;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& /*eventInfo*/) const
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_MAELSTROM_WEAPON_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR
        });
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_sha_maelstrom_weapon::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_sha_maelstrom_weapon::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 168534 - Mastery: Elemental Overload (passive)
class spell_sha_mastery_elemental_overload : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_SHAMAN_LIGHTNING_BOLT,
            SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD,
            SPELL_SHAMAN_ELEMENTAL_BLAST,
            SPELL_SHAMAN_ELEMENTAL_BLAST_OVERLOAD,
            SPELL_SHAMAN_ICEFURY,
            SPELL_SHAMAN_ICEFURY_OVERLOAD,
            SPELL_SHAMAN_LAVA_BURST,
            SPELL_SHAMAN_LAVA_BURST_OVERLOAD,
            SPELL_SHAMAN_CHAIN_LIGHTNING,
            SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD,
            SPELL_SHAMAN_STORMKEEPER
        });
    }

    static bool CheckProc(AuraScript const&, AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo || !eventInfo.GetProcSpell())
            return false;

        if (!GetTriggeredSpellId(spellInfo->Id))
            return false;

        float chance = aurEff->GetAmount();   // Mastery % amount

        if (spellInfo->Id == SPELL_SHAMAN_CHAIN_LIGHTNING)
            chance /= 3.0f;

        if (Aura* stormkeeper = eventInfo.GetActor()->GetAura(SPELL_SHAMAN_STORMKEEPER))
            if (eventInfo.GetProcSpell()->m_appliedMods.contains(stormkeeper))
                chance = 100.0f;

        return roll_chance_f(chance);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& procInfo)
    {
        PreventDefaultAction();

        Unit* caster = procInfo.GetActor();

        caster->m_Events.AddEventAtOffset([caster,
            targets = CastSpellTargetArg(procInfo.GetProcTarget()),
            overloadSpellId = GetTriggeredSpellId(procInfo.GetSpellInfo()->Id),
            originalCastId = procInfo.GetProcSpell()->m_castId]() mutable
        {
            if (!targets.Targets)
                return;

            targets.Targets->Update(caster);

            CastSpellExtraArgs args;
            args.OriginalCastId = originalCastId;
            caster->CastSpell(targets, overloadSpellId, args);
        }, 400ms);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_sha_mastery_elemental_overload::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_sha_mastery_elemental_overload::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

    static uint32 GetTriggeredSpellId(uint32 triggeringSpellId)
    {
        switch (triggeringSpellId)
        {
            case SPELL_SHAMAN_LIGHTNING_BOLT: return SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD;
            case SPELL_SHAMAN_ELEMENTAL_BLAST: return SPELL_SHAMAN_ELEMENTAL_BLAST_OVERLOAD;
            case SPELL_SHAMAN_ICEFURY: return SPELL_SHAMAN_ICEFURY_OVERLOAD;
            case SPELL_SHAMAN_LAVA_BURST: return SPELL_SHAMAN_LAVA_BURST_OVERLOAD;
            case SPELL_SHAMAN_CHAIN_LIGHTNING: return SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD;
            default:
                break;
        }
        return 0;
    }
};

// 45284 - Lightning Bolt Overload
// 45297 - Chain Lightning Overload
// 114738 - Lava Beam Overload
// 120588 - Elemental Blast Overload
// 219271 - Icefury Overload
// 285466 - Lava Burst Overload
class spell_sha_mastery_elemental_overload_proc : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_MASTERY_ELEMENTAL_OVERLOAD });
    }

    void ApplyDamageModifier(SpellEffIndex /*effIndex*/)
    {
        if (AuraEffect const* elementalOverload = GetCaster()->GetAuraEffect(SPELL_SHAMAN_MASTERY_ELEMENTAL_OVERLOAD, EFFECT_1))
            SetHitDamage(CalculatePct(GetHitDamage(), elementalOverload->GetAmount()));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_mastery_elemental_overload_proc::ApplyDamageModifier, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 334033 - Molten Assault (60103 - Lava Lash)
class spell_sha_molten_assault : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_FLAME_SHOCK });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_SHAMAN_MOLTEN_ASSAULT);
    }

    void TriggerFlameShocks(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit const* lavaLashTarget = GetHitUnit();
        if (!lavaLashTarget->HasAura(SPELL_SHAMAN_FLAME_SHOCK, caster->GetGUID()))
            return;

        float range = 10.0f;
        std::vector<WorldObject*> targets;
        Trinity::WorldObjectSpellAreaTargetCheck check(range, lavaLashTarget, caster, caster, sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_FLAME_SHOCK, DIFFICULTY_NONE),
            TARGET_CHECK_ENEMY, nullptr, TARGET_OBJECT_TYPE_UNIT, Trinity::WorldObjectSpellAreaTargetSearchReason::Area);
        Trinity::WorldObjectListSearcher searcher(caster, targets, check, GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER);
        Cell::VisitAllObjects(lavaLashTarget, searcher, range + EXTRA_CELL_SEARCH_RADIUS);

        auto withoutFlameShockItr = std::partition(targets.begin(), targets.end(), Trinity::UnitAuraCheck(true, SPELL_SHAMAN_FLAME_SHOCK, GetCaster()->GetGUID()));
        std::size_t flameShocksMissing = GetEffectValue() + 1 - std::ranges::distance(targets.begin(), withoutFlameShockItr);

        if (flameShocksMissing)
            Trinity::Containers::RandomShuffle(withoutFlameShockItr, targets.end());

        CastSpellExtraArgs args;
        args.SetTriggerFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_POWER_COST | TRIGGERED_IGNORE_CAST_IN_PROGRESS);
        args.SetTriggeringSpell(GetSpell());

        // targets that already have flame shock are first in the list (and need to refresh it)
        for (std::size_t i = 0; i < std::min<std::size_t>(targets.size(), GetEffectValue() + 1); ++i)
            caster->CastSpell(targets[i], SPELL_SHAMAN_FLAME_SHOCK, args);
    }

    void Register() override
    {
        // real Lava Lash (60103) only has 2 effects, both weapon-damage flavored (EFFECT_0
        // WEAPON_PERCENT_DAMAGE, EFFECT_1 NORMALIZED_WEAPON_DMG) - no EFFECT_2/DUMMY at all;
        // rebound to the main weapon-damage effect per wago.tools SpellEffect data
        OnEffectHitTarget += SpellEffectFn(spell_sha_molten_assault::TriggerFlameShocks, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// 210621 - Path of Flames Spread
class spell_sha_path_of_flames_spread : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_FLAME_SHOCK });
    }

    void FilterTargets(std::list<WorldObject*>& targets) const
    {
        targets.remove(GetExplTargetUnit());
        Trinity::Containers::RandomResize(targets, Trinity::UnitAuraCheck(false, SPELL_SHAMAN_FLAME_SHOCK, GetCaster()->GetGUID()), 1);
    }

    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        if (Unit* mainTarget = GetExplTargetUnit())
        {
            if (Aura* flameShock = mainTarget->GetAura(SPELL_SHAMAN_FLAME_SHOCK, GetCaster()->GetGUID()))
            {
                if (Aura* newAura = GetCaster()->AddAura(SPELL_SHAMAN_FLAME_SHOCK, GetHitUnit()))
                {
                    newAura->SetDuration(flameShock->GetDuration());
                    newAura->SetMaxDuration(flameShock->GetDuration());
                }
            }
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sha_path_of_flames_spread::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_sha_path_of_flames_spread::HandleScript, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// 114083 - Restorative Mists
// 294020 - Restorative Mists
class spell_sha_restorative_mists : public SpellScript
{
    void HandleHeal(SpellEffIndex effIndex)
    {
        SetHitHeal(GetHitHeal() / GetUnitTargetCountForEffect(effIndex));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_restorative_mists::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

class StormflurryEvent : public BasicEvent
{
public:
    struct Data
    {
        int32 DamagePercent = 0;
    };

    explicit StormflurryEvent(Unit* caster, Unit* target, ObjectGuid const& originalCastId, int32 damagePercent,
        uint32 mainHandDamageSpellId, uint32 offHandDamageSpellId, int32 procChance)
        : _caster(caster), _target(target), _originalCastId(originalCastId), _damagePercent(damagePercent),
            _mainHandDamageSpellId(mainHandDamageSpellId), _offHandDamageSpellId(offHandDamageSpellId), _procChance(procChance)
    {
    }

    bool Execute(uint64 time, uint32 /*diff*/) override
    {
        if (!_target.Targets)
            return true;

        _target.Targets->Update(_caster);

        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR;
        args.OriginalCastId = _originalCastId;
        args.CustomArg.emplace<Data>().DamagePercent = _damagePercent;

        _caster->CastSpell(_target, _mainHandDamageSpellId, args);
        _caster->CastSpell(_target, _offHandDamageSpellId, args);

        if (!roll_chance_i(_procChance))
            return true;

        _caster->m_Events.AddEvent(this, Milliseconds(time) + 200ms);
        return false;
    }

private:
    Unit* _caster;
    CastSpellTargetArg _target;
    ObjectGuid _originalCastId;
    int32 _damagePercent;
    uint32 _mainHandDamageSpellId;
    uint32 _offHandDamageSpellId;
    int32 _procChance;
};

// 198367 - Stormflurry (Doomhammer artifact trait)
class spell_sha_stormflurry : public SpellScript
{
public:
    explicit spell_sha_stormflurry(uint32 stormflurrySpellId, uint32 mainHandDamageSpellId, uint32 offHandDamageSpellId)
        : _stormflurrySpellId(stormflurrySpellId), _mainHandDamageSpellId(mainHandDamageSpellId), _offHandDamageSpellId(offHandDamageSpellId) { }

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ _stormflurrySpellId, _mainHandDamageSpellId, _offHandDamageSpellId })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } })
            && spellInfo->GetEffect(EFFECT_0).IsEffect(SPELL_EFFECT_TRIGGER_SPELL)
            && spellInfo->GetEffect(EFFECT_1).IsEffect(SPELL_EFFECT_TRIGGER_SPELL);
    }

    bool Load() override
    {
        return GetCaster()->HasAura(_stormflurrySpellId);
    }

    void HandleProc(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Aura const* stormflurry = caster->GetAura(_stormflurrySpellId);
        if (!stormflurry)
            return;

        AuraEffect const* chanceEffect = stormflurry->GetEffect(EFFECT_0);
        AuraEffect const* damageEffect = stormflurry->GetEffect(EFFECT_1);
        if (!chanceEffect || !damageEffect)
            return;

        int32 procChance = chanceEffect->GetAmount();
        if (!roll_chance_i(procChance))
            return;

        caster->m_Events.AddEventAtOffset(new StormflurryEvent(caster, GetHitUnit(), GetSpell()->m_castId, damageEffect->GetAmount(),
            _mainHandDamageSpellId, _offHandDamageSpellId, procChance), 200ms);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_stormflurry::HandleProc, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
    }

private:
    uint32 _stormflurrySpellId;
    uint32 _mainHandDamageSpellId;
    uint32 _offHandDamageSpellId;
};

// 32175 - Stormstrike
// 32176 - Stormstrike Off-Hand
class spell_sha_stormflurry_damage : public SpellScript
{
    bool Load() override
    {
        return GetSpell()->m_customArg.type() == typeid(StormflurryEvent::Data);
    }

    void ApplyModifier(SpellEffectInfo const& /*effectInfo*/, Unit* /*victim*/, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        ApplyPct(pctMod, std::any_cast<StormflurryEvent::Data>(&GetSpell()->m_customArg)->DamagePercent);
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_sha_stormflurry_damage::ApplyModifier);
    }
};

// 195255 - Stormlash (Doomhammer artifact trait): while weapons are enhanced, attacks have a
// chance to grant Stormlash to up to 2 party/raid members (the target-selection is the buff's
// own DB2 data, not scripted). Confirmed genuine Legion content via web search - a distinct
// trait from the removed WoD-era "Stormlash Totem," the same reused-name-across-eras pattern
// already confirmed this session for Doom Winds/Stormflurry/Death's Embrace. Confirmed via
// two independent reference sources (identical implementations).
class spell_sha_stormlash : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_STORMLASH_BUFF });
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_SHAMAN_STORMLASH_BUFF, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_stormlash::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 195222 - Stormlash Buff: whoever holds this buff deals extra Nature damage via
// SPELL_SHAMAN_STORMLASH_DAMAGE whenever their own attacks/casts land.
class spell_sha_stormlash_buff : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_STORMLASH_DAMAGE });
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActionTarget(), SPELL_SHAMAN_STORMLASH_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_stormlash_buff::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 201845 - Stormsurge
class spell_sha_stormsurge : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_STORMSURGE_PROC });
    }

    static void HandleProc(AuraScript const&, AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_SHAMAN_STORMSURGE_PROC, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_stormsurge::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 51564 - Tidal Waves
class spell_sha_tidal_waves : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_TIDAL_WAVES });
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/)
    {
        PreventDefaultAction();

        GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_TIDAL_WAVES, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff,
            .SpellValueOverrides =
            {
                { SPELLVALUE_BASE_POINT0, -aurEff->GetAmount() },
                { SPELLVALUE_BASE_POINT1, aurEff->GetAmount() }
            }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_tidal_waves::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 28823 - Totemic Power
class spell_sha_t3_6p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_SHAMAN_TOTEMIC_POWER_ARMOR,
            SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER,
            SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER,
            SPELL_SHAMAN_TOTEMIC_POWER_MP5
        });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        uint32 spellId;
        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        switch (target->GetClass())
        {
            case CLASS_PALADIN:
            case CLASS_PRIEST:
            case CLASS_SHAMAN:
            case CLASS_DRUID:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_MP5;
                break;
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER;
                break;
            case CLASS_HUNTER:
            case CLASS_ROGUE:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER;
                break;
            case CLASS_WARRIOR:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_ARMOR;
                break;
            default:
                return;
        }

        caster->CastSpell(target, spellId, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_t3_6p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 28820 - Lightning Shield
class spell_sha_t3_8p_bonus : public AuraScript
{
    void PeriodicTick(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        // Need remove self if Lightning Shield not active
        if (!GetTarget()->GetAuraEffect(SPELL_AURA_PROC_TRIGGER_SPELL, SPELLFAMILY_SHAMAN, flag128(0x400), GetCaster()->GetGUID()))
            Remove();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_t3_8p_bonus::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 64928 - Item - Shaman T8 Elemental 4P Bonus
class spell_sha_t8_elemental_4p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_ELECTRIFIED });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_ELECTRIFIED, GetCastDifficulty());
        int32 amount = CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount());

        ASSERT(spellInfo->GetMaxTicks() > 0);
        amount /= spellInfo->GetMaxTicks();

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        caster->CastSpell(target, SPELL_SHAMAN_ELECTRIFIED, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, amount } }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_t8_elemental_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 67228 - Item - Shaman T9 Elemental 4P Bonus (Lava Burst)
class spell_sha_t9_elemental_4p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_LAVA_BURST_BONUS_DAMAGE });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_LAVA_BURST_BONUS_DAMAGE, GetCastDifficulty());
        int32 amount = CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount());

        ASSERT(spellInfo->GetMaxTicks() > 0);
        amount /= spellInfo->GetMaxTicks();

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        caster->CastSpell(target, SPELL_SHAMAN_LAVA_BURST_BONUS_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, amount } }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_t9_elemental_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 70817 - Item - Shaman T10 Elemental 4P Bonus
class spell_sha_t10_elemental_4p_bonus : public AuraScript
{
    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        // try to find spell Flame Shock on the target
        AuraEffect* flameShock = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_SHAMAN, flag128(0x10000000), caster->GetGUID());
        if (!flameShock)
            return;

        Aura* flameShockAura = flameShock->GetBase();

        int32 maxDuration = flameShockAura->GetMaxDuration();
        int32 newDuration = flameShockAura->GetDuration() + aurEff->GetAmount() * IN_MILLISECONDS;

        flameShockAura->SetDuration(newDuration);
        // is it blizzlike to change max duration for FS?
        if (newDuration > maxDuration)
            flameShockAura->SetMaxDuration(newDuration);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_t10_elemental_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 70808 - Item - Shaman T10 Restoration 4P Bonus
class spell_sha_t10_restoration_4p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_CHAINED_HEAL });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_CHAINED_HEAL, GetCastDifficulty());
        int32 amount = CalculatePct(static_cast<int32>(healInfo->GetHeal()), aurEff->GetAmount());

        ASSERT(spellInfo->GetMaxTicks() > 0);
        amount /= spellInfo->GetMaxTicks();

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        caster->CastSpell(target, SPELL_SHAMAN_CHAINED_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, amount } }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_t10_restoration_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 200071 - Undulation
class spell_sha_undulation_passive : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_UNDULATION_PROC });
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& /*eventInfo*/)
    {
        if (++_castCounter == 3)
        {
            GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_UNDULATION_PROC, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            _castCounter = 0;
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_undulation_passive::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

    uint8 _castCounter = 1; // first proc happens after two casts, then one every 3 casts
};

// 33757 - Windfury Weapon: removed spell_sha_windfury_weapon entirely. Its whole premise - cast
// onto the weapon item, PreventHitDefaultEffect, substitute a separate "enchantment" spell
// (334302, confirmed absent) - was wrong. Real 33757's own EFFECT_0 targets self, not the weapon
// item, and is a plain APPLY_AURA/DUMMY that the engine already applies natively on cast; no
// script is needed for the cast side at all. See spell_sha_windfury_weapon_proc below for the
// resulting aura's proc behavior (also bound to 33757, since it's the same self-referential id).

bool WindfuryProcEvent::Execute(uint64 time, uint32 /*diff*/)
{
    if (!_target.Targets)
        return true;

    _target.Targets->Update(_shaman);
    if (!_target.Targets->GetUnitTarget())
        return true;

    CastSpellExtraArgs args;
    args.TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR;
    args.TriggeringAura = _shaman->GetAuraEffect(SPELL_SHAMAN_WINDFURY_AURA, EFFECT_0); // prevent proc from itself

    _shaman->CastSpell(_target, SPELL_SHAMAN_WINDFURY_ATTACK, args);

    if (++_itr == _end)
        return true;

    _shaman->m_Events.AddEvent(this, Milliseconds(time) + _itr->Delay);
    return false;
}

void WindfuryProcEvent::Trigger(Unit* shaman, Unit* target)
{
    std::ptrdiff_t attacks = 2;
    shaman->m_Events.AddEventAtOffset(new WindfuryProcEvent(shaman, target, attacks), Sequence.front().Delay);
}

// 33757 - Windfury Weapon (proc, same id as the outer cast - see comment above)
class spell_sha_windfury_weapon_proc : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_WINDFURY_ATTACK });
    }

    void HandleEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        WindfuryProcEvent::Trigger(eventInfo.GetActor(), eventInfo.GetActionTarget());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_sha_windfury_weapon_proc::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 192078 - Wind Rush Totem (Spell)
// 12676 - AreaTriggerId
struct areatrigger_sha_wind_rush_totem : AreaTriggerAI
{
    static constexpr uint32 REFRESH_TIME = 4500;

    using AreaTriggerAI::AreaTriggerAI;

    void OnUpdate(uint32 diff) override
    {
        _refreshTimer -= diff;
        if (_refreshTimer <= 0)
        {
            if (Unit* caster = at->GetCaster())
                for (ObjectGuid const& guid : at->GetInsideUnits())
                    if (Unit* unit = ObjectAccessor::GetUnit(*caster, guid))
                        CastSpeedBuff(caster, unit);

            _refreshTimer += REFRESH_TIME;
        }
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
            CastSpeedBuff(caster, unit);
    }

    static void CastSpeedBuff(Unit* caster, Unit* unit)
    {
        if (!caster->IsValidAssistTarget(unit))
            return;

        caster->CastSpell(unit, SPELL_SHAMAN_WIND_RUSH, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

private:
    int32 _refreshTimer = REFRESH_TIME;
};

// 51533 - Feral Spirit
// The spell itself does nothing useful — it needs a script to actually summon
// the spirit wolves using the correct summon spell (228562). Without this the
// cast fires and the spirit wolves never appear.
class spell_sha_feral_spirit : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SHAMAN_FERAL_SPIRIT_SUMMON });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetHitUnit(), SPELL_SHAMAN_FERAL_SPIRIT_SUMMON, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_feral_spirit::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// NPC 29264 - Spirit Wolf
// Each hit from a spirit wolf generates Maelstrom for the shaman owner.
// The energize dummy aura (231723) on the shaman acts as a "switch" — if the
// shaman has it, the wolf triggers the energize cast (190185) on every hit.
// DB note: creature_template.ScriptName must be 'npc_sha_feral_spirit' for entry 29264.
struct npc_sha_feral_spirit : public ScriptedAI
{
    npc_sha_feral_spirit(Creature* creature) : ScriptedAI(creature) { }

    void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) override
    {
        if (Unit* owner = me->GetOwner())
            if (owner->HasAura(SPELL_SHAMAN_FERAL_SPIRIT_ENERGIZE_DUMMY))
                me->CastSpell(owner, SPELL_SHAMAN_FERAL_SPIRIT_ENERGIZE, true);
    }
};

// 188070 - Healing Surge
// When the shaman has enough Maelstrom (checked via EFFECT_2 base points) the
// cast time is instantly set to zero and the Maelstrom cost is recorded. The
// energize effect (EFFECT_1) then drains exactly that amount so the UI shows
// the correct resource change without a separate aura or timer.
class spell_sha_healing_surge : public SpellScript
{
    int32 CalcCastTime(int32 castTime) override
    {
        int32 requiredMaelstrom = GetEffectInfo(EFFECT_2).BasePoints;
        if (GetCaster()->GetPower(POWER_MAELSTROM) >= requiredMaelstrom)
        {
            castTime = 0;
            _takenPower = requiredMaelstrom;
        }

        return castTime;
    }

    void HandleEnergize(SpellEffIndex /*effIndex*/)
    {
        SetEffectValue(-_takenPower);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sha_healing_surge::HandleEnergize, EFFECT_1, SPELL_EFFECT_ENERGIZE);
    }

private:
    int32 _takenPower = 0;
};

void AddSC_shaman_spell_scripts()
{
    RegisterSpellScript(spell_sha_aftershock);
    RegisterSpellScript(spell_sha_ancestral_guidance);
    RegisterSpellScript(spell_sha_ancestral_guidance_heal);
    RegisterSpellScript(spell_sha_artifact_gathering_storms);
    RegisterSpellScript(spell_sha_ascendance_restoration);
    RegisterSpellScript(spell_sha_feral_lunge);
    RegisterSpellScript(spell_sha_spirit_link);
    RegisterSpellScript(spell_sha_chain_lightning_crash_lightning);
    RegisterSpellScript(spell_sha_chain_lightning_energize);
    RegisterSpellScript(spell_sha_chain_lightning_overload);
    RegisterSpellScript(spell_sha_cloudburst);
    RegisterSpellScript(spell_sha_cloudburst_effect);
    RegisterSpellScript(spell_sha_resonance_effect);
    RegisterSpellScript(spell_sha_earthgrab);
    RegisterSpellScript(spell_sha_crash_lightning);
    RegisterSpellScript(spell_sha_fury_of_air);
    RegisterSpellScript(spell_sha_earthen_shield_absorb);
    RegisterSpellScript(spell_sha_deluge);
    RegisterSpellScript(spell_sha_deluge_healing_rain);
    RegisterSpellScript(spell_sha_downpour);
    RegisterSpellScript(spell_sha_earth_elemental);
    RegisterSpellScript(spell_sha_fire_elemental);
    RegisterSpellScript(spell_sha_earth_shield);
    RegisterSpellScript(spell_sha_earthen_rage_passive);
    RegisterSpellScript(spell_sha_earthen_rage_proc_aura);
    RegisterAreaTriggerAI(areatrigger_sha_earthquake);
    RegisterSpellScript(spell_sha_earthquake_tick);
    RegisterSpellScript(spell_sha_elemental_blast);
    RegisterSpellScript(spell_sha_feral_spirit);
    RegisterCreatureAI(npc_sha_feral_spirit);
    RegisterSpellScript(spell_sha_flametongue_weapon);
    RegisterSpellScript(spell_sha_flametongue_weapon_aura);
    RegisterSpellScript(spell_sha_hailstorm);
    RegisterSpellAndAuraScriptPair(spell_sha_healing_rain, spell_sha_healing_rain_aura);
    RegisterSpellScript(spell_sha_healing_rain_target_limit);
    RegisterSpellScript(spell_sha_healing_stream_totem_heal);
    RegisterSpellScript(spell_sha_healing_surge);
    RegisterSpellScript(spell_sha_hot_hand);
    RegisterSpellScript(spell_sha_item_lightning_shield);
    RegisterSpellScript(spell_sha_item_lightning_shield_trigger);
    RegisterSpellScript(spell_sha_item_mana_surge);
    RegisterSpellScript(spell_sha_item_t6_trinket);
    RegisterSpellScript(spell_sha_item_t10_elemental_2p_bonus);
    RegisterSpellScript(spell_sha_item_t18_elemental_4p_bonus);
    RegisterSpellScript(spell_sha_lava_burst);
    RegisterSpellScript(spell_sha_lava_crit_chance);
    RegisterSpellScript(spell_sha_lava_lash);
    RegisterSpellScript(spell_sha_lava_surge);
    RegisterSpellScript(spell_sha_lava_surge_proc);
    RegisterSpellScript(spell_sha_lightning_bolt);
    RegisterSpellScript(spell_sha_lightning_bolt_overload);
    RegisterSpellScript(spell_sha_liquid_magma_totem);
    RegisterSpellScript(spell_sha_maelstrom_weapon);
    RegisterSpellScript(spell_sha_mastery_elemental_overload);
    RegisterSpellScript(spell_sha_mastery_elemental_overload_proc);
    RegisterSpellScript(spell_sha_molten_assault);
    RegisterSpellScript(spell_sha_path_of_flames_spread);
    RegisterSpellScript(spell_sha_restorative_mists);
    RegisterSpellScriptWithArgs(spell_sha_stormflurry, "spell_sha_artifact_stormflurry_stormstrike",
        SPELL_SHAMAN_STORMFLURRY_ARTIFACT, SPELL_SHAMAN_STORMSTRIKE_DAMAGE_MAIN_HAND, SPELL_SHAMAN_STORMSTRIKE_DAMAGE_OFF_HAND);
    RegisterSpellScriptWithArgs(spell_sha_stormflurry, "spell_sha_artifact_stormflurry_windstrike",
        SPELL_SHAMAN_STORMFLURRY_ARTIFACT, SPELL_SHAMAN_WINDSTRIKE_DAMAGE_MAIN_HAND, SPELL_SHAMAN_WINDSTRIKE_DAMAGE_OFF_HAND);
    RegisterSpellScript(spell_sha_stormflurry_damage);
    RegisterSpellScript(spell_sha_stormlash);
    RegisterSpellScript(spell_sha_stormlash_buff);
    RegisterSpellScript(spell_sha_stormsurge);
    RegisterSpellScriptWithArgs(spell_sha_delayed_stormstrike_mod_charge_drop_proc, "spell_sha_stormsurge_proc");
    RegisterSpellScript(spell_sha_tidal_waves);
    RegisterSpellScript(spell_sha_t3_6p_bonus);
    RegisterSpellScript(spell_sha_t3_8p_bonus);
    RegisterSpellScript(spell_sha_t8_elemental_4p_bonus);
    RegisterSpellScript(spell_sha_t9_elemental_4p_bonus);
    RegisterSpellScript(spell_sha_t10_elemental_4p_bonus);
    RegisterSpellScript(spell_sha_t10_restoration_4p_bonus);
    RegisterSpellScript(spell_sha_undulation_passive);
    RegisterSpellScript(spell_sha_windfury_weapon_proc);
    RegisterAreaTriggerAI(areatrigger_sha_wind_rush_totem);
}
