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
 * Scripts for spells with SPELLFAMILY_WARLOCK and SPELLFAMILY_GENERIC spells used by warlock players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_warl_".
 */

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "Containers.h"
#include "Creature.h"
#include "GameObject.h"
#include "GridNotifiersImpl.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "Random.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum WarlockSpells
{
    SPELL_WARLOCK_ABSOLUTE_CORRUPTION               = 196103,
    SPELL_WARLOCK_AGONY                             = 980,
    SPELL_WARLOCK_BACKDRAFT                         = 196406,
    SPELL_WARLOCK_BACKDRAFT_PROC                    = 117828,
    SPELL_WARLOCK_CALL_DREADSTALKERS_SUMMON         = 193331,
    SPELL_WARLOCK_CHANNEL_DEMONFIRE_DAMAGE          = 196448,
    SPELL_WARLOCK_CONFLAGRATE_DEBUFF                = 265931,
    SPELL_WARLOCK_CONFLAGRATE_ENERGIZE              = 245330,
    SPELL_WARLOCK_CORRUPTION_DAMAGE                 = 146739,
    SPELL_WARLOCK_CREATE_HEALTHSTONE                = 23517,
    SPELL_WARLOCK_DEATHS_EMBRACE                    = 453189,
    SPELL_WARLOCK_DEMONBOLT_ENERGIZE                = 280127,
    SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST         = 62388,
    SPELL_WARLOCK_DEMONIC_CIRCLE_SUMMON             = 48018,
    SPELL_WARLOCK_DEMONBOLT                         = 157695,
    SPELL_WARLOCK_DEMONIC_CALLING                   = 205145,
    SPELL_WARLOCK_DEMONIC_CALLING_TRIGGER           = 205146,
    SPELL_WARLOCK_DEMONIC_CIRCLE_TELEPORT           = 48020,
    // Demonic Gateway (111771, confirmed via independent web sources - WowDB/Wowhead both
    // agree - since neither reference core declares the base cast id explicitly, only its
    // component pieces below).
    SPELL_WARLOCK_DEMONIC_GATEWAY                    = 111771,
    SPELL_WARLOCK_DEMONIC_GATEWAY_SUMMON_GREEN       = 113886,
    SPELL_WARLOCK_GATEWAY_PORTAL_VISUAL               = 113900,
    SPELL_WARLOCK_GATEWAY_INTERACT                    = 113902,
    SPELL_WARLOCK_GATEWAY_COOLDOWN_MARKER             = 113942,
    SPELL_WARLOCK_GATEWAY_TELEPORT_VISUAL_GREEN       = 236762,
    SPELL_WARLOCK_GATEWAY_TELEPORT_VISUAL_PURPLE      = 236671,
    NPC_WARLOCK_GATEWAY_GREEN                          = 59262,
    NPC_WARLOCK_GATEWAY_PURPLE                         = 59271,
    SPELL_WARLOCK_DEVOUR_MAGIC_HEAL                 = 19658,
    SPELL_WARLOCK_DOOM                              = 603,
    SPELL_WARLOCK_DOOM_ENERGIZE                     = 193318,
    SPELL_WARLOCK_DRAIN_SOUL_ENERGIZE               = 205292,
    SPELL_WARLOCK_ERADICATION                       = 196412,
    SPELL_WARLOCK_EYE_LASER                         = 205231,
    SPELL_WARLOCK_ERADICATION_DEBUFF                = 196414,
    SPELL_WARLOCK_FLAMESHADOW                       = 37379,
    SPELL_WARLOCK_GLYPH_OF_DEMON_TRAINING           = 56249,
    SPELL_WARLOCK_GLYPH_OF_SOUL_SWAP                = 56226,
    SPELL_WARLOCK_GLYPH_OF_SUCCUBUS                 = 56250,
    SPELL_WARLOCK_GRIMOIRE_OF_SYNERGY_BUFF          = 171982,
    SPELL_WARLOCK_HAND_OF_DOOM                      = 196283,
    SPELL_WARLOCK_HAND_OF_GULDAN_DAMAGE             = 86040,
    SPELL_WARLOCK_HAND_OF_GULDAN_SUMMON             = 196282,
    SPELL_WARLOCK_IMMOLATE_PERIODIC                 = 157736,
    SPELL_WARLOCK_IMPLOSION_DAMAGE                  = 196278,
    SPELL_WARLOCK_IMPLOSION_JUMP                    = 205205,
    // Wild Imp pet creature entry, needed by Implosion below to find the caster's currently
    // active imps via Unit::m_Controlled - confirmed independently via Wowhead, since neither
    // reference core actually defines this id anywhere (their own Implosion logic is dead,
    // commented-out code referencing an undefined PET_ENTRY_WILD_IMP that never compiled).
    NPC_WARLOCK_WILD_IMP                            = 55659,
    SPELL_WARLOCK_IMPROVED_DREADSTALKERS            = 196272,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1    = 60955,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2    = 60956,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R1         = 18703,
    SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R2         = 18704,
    SPELL_WARLOCK_RAIN_OF_FIRE                      = 5740,
    SPELL_WARLOCK_RAIN_OF_FIRE_DAMAGE               = 42223,
    SPELL_WARLOCK_ROARING_BLAZE                     = 205184,
    SPELL_WARLOCK_SEED_OF_CORRUPTION_DAMAGE         = 27285,
    SPELL_WARLOCK_SEED_OF_CORRUPTION_GENERIC        = 32865,
    SPELL_WARLOCK_SHADOWBURN_ENERGIZE               = 245731,
    SPELL_WARLOCK_SHADOW_BOLT                       = 686,
    SPELL_WARLOCK_SHADOW_BOLT_ENERGIZE              = 194192,
    SPELL_WARLOCK_SHADOWFLAME                       = 37378,
    SPELL_WARLOCK_SOUL_CONDUIT_REFUND                = 215942,
    SPELL_WARLOCK_DEMONSKIN                         = 219272,
    SPELL_WARLOCK_SOUL_LEECH                        = 228974,
    SPELL_WARLOCK_SOUL_LEECH_ABSORB                 = 108366,
    SPELL_WARLOCK_SOUL_FIRE_ENERGIZE                = 281490,
    SPELL_WARLOCK_SOUL_SWAP_CD_MARKER               = 94229,
    SPELL_WARLOCK_SOUL_SWAP_DOT_MARKER              = 92795,
    SPELL_WARLOCK_SOUL_SWAP_MOD_COST                = 92794,
    SPELL_WARLOCK_SOUL_SWAP_OVERRIDE                = 86211,
    SPELL_WARLOCK_SOULSHATTER_EFFECT                = 32835,
    SPELL_WARLOCK_UNSTABLE_AFFLICTION_DAMAGE        = 196364,
    SPELL_WARLOCK_UNSTABLE_AFFLICTION_ENERGIZE      = 31117,
};

enum MiscSpells
{
    SPELL_GEN_REPLENISHMENT                         = 57669,
    SPELL_PRIEST_SHADOW_WORD_DEATH                  = 32409
};

// 146739 - Corruption
class spell_warl_absolute_corruption : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellEffect({ { SPELL_WARLOCK_ABSOLUTE_CORRUPTION, EFFECT_0 } });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_WARLOCK_ABSOLUTE_CORRUPTION);
    }

    void HandleApply(SpellEffIndex /*effIndex*/) const
    {
        if (Aura const* absoluteCorruption = GetCaster()->GetAura(SPELL_WARLOCK_ABSOLUTE_CORRUPTION))
        {
            Milliseconds duration = GetHitUnit()->IsPvP()
                ? Seconds(absoluteCorruption->GetSpellInfo()->GetEffect(EFFECT_0).CalcValue())
                : Milliseconds(-1);

            GetHitAura()->SetMaxDuration(duration.count());
            GetHitAura()->SetDuration(duration.count());
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_absolute_corruption::HandleApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// Called by 17962 - Conflagrate
class spell_warl_backdraft : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_WARLOCK_BACKDRAFT, SPELL_WARLOCK_BACKDRAFT_PROC });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_WARLOCK_BACKDRAFT);
    }

    void HandleAfterCast() const
    {
        Unit* caster = GetCaster();
        caster->CastSpell(caster, SPELL_WARLOCK_BACKDRAFT_PROC, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_warl_backdraft::HandleAfterCast);
    }
};

// 710 - Banish
class spell_warl_banish : public SpellScript
{
public:
    spell_warl_banish() {}

private:
    void HandleBanish(SpellMissInfo missInfo)
    {
        if (missInfo != SPELL_MISS_IMMUNE)
            return;

        if (Unit* target = GetHitUnit())
        {
            // Casting Banish on a banished target will remove applied aura
            if (Aura * banishAura = target->GetAura(GetSpellInfo()->Id, GetCaster()->GetGUID()))
                banishAura->Remove();
        }
    }

    void Register() override
    {
        BeforeHit += BeforeSpellHitFn(spell_warl_banish::HandleBanish);
    }
};

// 111400 - Burning Rush
class spell_warl_burning_rush : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    SpellCastResult CheckApplyAura()
    {
        Unit* caster = GetCaster();

        if (caster->GetHealthPct() <= float(GetEffectInfo(EFFECT_1).CalcValue(caster)))
        {
            SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_YOU_DONT_HAVE_ENOUGH_HEALTH);
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_burning_rush::CheckApplyAura);
    }
};

// 111400 - Burning Rush
class spell_warl_burning_rush_aura : public AuraScript
{
    void PeriodicTick(AuraEffect const* aurEff)
    {
        if (GetTarget()->GetHealthPct() <= float(aurEff->GetAmount()))
        {
            PreventDefaultAction();
            Remove();
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_burning_rush_aura::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
    }
};

// 104316 - Call Dreadstalkers
class spell_warl_call_dreadstalkers : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_CALL_DREADSTALKERS_SUMMON, SPELL_WARLOCK_HAND_OF_GULDAN_SUMMON });
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_WARLOCK_CALL_DREADSTALKERS_SUMMON, true);
        caster->CastSpell(caster, SPELL_WARLOCK_CALL_DREADSTALKERS_SUMMON + 1, true);

        if (AuraEffect const* improvedDreadstalkers = caster->GetAuraEffect(SPELL_WARLOCK_IMPROVED_DREADSTALKERS, EFFECT_0))
            for (int32 i = 0; i < improvedDreadstalkers->GetAmount(); ++i)
                caster->CastSpell(caster, SPELL_WARLOCK_HAND_OF_GULDAN_SUMMON, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_call_dreadstalkers::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 205145 - Demonic Calling (Demonology talent): Shadow Bolt/Demonbolt hits have a 20% chance
// to grant a buff making the next Call Dreadstalkers free/instant. Confirmed via DestinyCore/
// AshamaneCore (identical implementations) - both also carry an apparently-erroneous second
// copy of this exact check inside their unrelated Demonwrath class, which was not ported here
// since it reads as leftover/copy-paste drift rather than a genuine second trigger (Demonic
// Calling's real tooltip is specifically about Shadow Bolt/Demonbolt). Only grants the buff -
// nothing in either reference ever scripts consuming it either, meaning the "next Call
// Dreadstalkers is free" effect is entirely the buff's own DB2 spell-modifier data
// (SpellClassMask-restricted, auto-consumed by the generic engine spell-mod pipeline when Call
// Dreadstalkers is cast), needing no script on the consuming side at all.
class spell_warl_demonic_calling : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_DEMONIC_CALLING_TRIGGER });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (caster && eventInfo.GetSpellInfo() &&
            (eventInfo.GetSpellInfo()->Id == SPELL_WARLOCK_DEMONBOLT || eventInfo.GetSpellInfo()->Id == SPELL_WARLOCK_SHADOW_BOLT) &&
            roll_chance_i(20))
        {
            caster->CastSpell(caster, SPELL_WARLOCK_DEMONIC_CALLING_TRIGGER, true);
        }

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_warl_demonic_calling::CheckProc);
    }
};

// 111771 - Demonic Gateway: summons a green gateway at the target destination (despawning any
// pre-existing gateway pair the caster already owns first). The matching purple gateway is
// summoned automatically by this spell's own second effect at the caster's position - not
// scripted, since it needs no destination resolution. Confirmed via DestinyCore/AshamaneCore
// (identical implementations). Translated the raw CastSpell(x, y, z, spellId, ...) float-triple
// overload (this project's own CLAUDE.md already documents this doesn't exist in ArgusCore) to
// CastSpell(Position const&, ...), and Creature::GetOwner() (unreliable for temp summons that
// aren't guardian-type) to TempSummon::GetSummonerUnit(), the dedicated accessor for exactly
// this need.
class spell_warl_demonic_gateway : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_DEMONIC_GATEWAY_SUMMON_GREEN });
    }

    void HandleSummon()
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        WorldLocation const* dest = GetExplTargetDest();
        if (!caster || !dest)
            return;

        for (uint32 entry : { uint32(NPC_WARLOCK_GATEWAY_GREEN), uint32(NPC_WARLOCK_GATEWAY_PURPLE) })
        {
            std::list<Creature*> gates;
            caster->GetCreatureListWithEntryInGrid(gates, entry, 500.0f);

            for (Creature* gate : gates)
                if (gate->IsSummon() && gate->ToTempSummon()->GetSummonerUnit() == caster)
                    gate->DespawnOrUnsummon();
        }

        caster->CastSpell(*dest, SPELL_WARLOCK_DEMONIC_GATEWAY_SUMMON_GREEN, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_warl_demonic_gateway::HandleSummon);
    }
};

// 113902 - Gateway Interact: triggered by clicking either gateway NPC (bound via
// npc_spellclick_spells with cast_flags = NPC_CLICK_CAST_TARGET_CLICKER only, so the clicked
// gateway itself is the caster and the clicking player is the target - letting this script
// know both which gate was clicked and who to teleport). Finds the nearest matching
// opposite-color gateway belonging to the same owner and jumps the player there. Confirmed via
// DestinyCore/AshamaneCore, but substantially redesigned: the references drive the whole
// interaction through a legacy raw UNIT_FIELD_INTERACT_SPELLID field plus a manual
// CreatureAI::OnSpellClick override with a `bool&` result parameter - ArgusCore's actual
// CreatureAI::OnSpellClick takes `bool spellClickHandled` by value (an already-resolved
// in-parameter, not an out-parameter the AI can veto), and click-to-cast wiring is DB-driven
// via npc_spellclick_spells (SpellClickInfo), including built-in SPELL_CLICK_USER_PARTY
// group-membership validation that replaces the reference's manual Group-comparison code
// entirely. The group-membership/fear/cooldown checks and "am I friendly to whoever's already
// interacting" logic are handled by that DB config plus this spell's own CheckCast, so no
// CreatureAI::OnSpellClick override is needed at all - see npc_warl_demonic_gateway below for
// the (much smaller) remaining creature AI piece.
class spell_warl_demonic_gateway_interact : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_GATEWAY_COOLDOWN_MARKER, SPELL_WARLOCK_GATEWAY_TELEPORT_VISUAL_GREEN, SPELL_WARLOCK_GATEWAY_TELEPORT_VISUAL_PURPLE });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        Unit* player = GetExplTargetUnit();
        if (!caster || !player)
            return SPELL_FAILED_DONT_REPORT;

        if (player->HasAura(SPELL_WARLOCK_GATEWAY_COOLDOWN_MARKER) || player->IsFeared())
            return SPELL_FAILED_DONT_REPORT;

        return SPELL_CAST_OK;
    }

    void HandleTeleport()
    {
        Creature* clickedGate = GetCaster() ? GetCaster()->ToCreature() : nullptr;
        Unit* player = GetExplTargetUnit();
        if (!clickedGate || !player || !clickedGate->IsSummon())
            return;

        Unit* owner = clickedGate->ToTempSummon()->GetSummonerUnit();
        if (!owner)
            return;

        bool clickedGreen = clickedGate->GetEntry() == NPC_WARLOCK_GATEWAY_GREEN;
        uint32 targetEntry = clickedGreen ? NPC_WARLOCK_GATEWAY_PURPLE : NPC_WARLOCK_GATEWAY_GREEN;

        std::list<Creature*> candidates;
        clickedGate->GetCreatureListWithEntryInGrid(candidates, targetEntry, 500.0f);
        candidates.remove_if([owner](Creature* gate)
        {
            return !gate->IsSummon() || gate->ToTempSummon()->GetSummonerUnit() != owner;
        });

        if (candidates.empty())
            return;

        candidates.sort([clickedGate](Creature const* a, Creature const* b)
        {
            return a->GetDistance(clickedGate) < b->GetDistance(clickedGate);
        });

        Creature* targetGate = candidates.front();

        player->CastSpell(player, SPELL_WARLOCK_GATEWAY_COOLDOWN_MARKER, true);
        player->CastSpell(player, clickedGreen ? SPELL_WARLOCK_GATEWAY_TELEPORT_VISUAL_GREEN : SPELL_WARLOCK_GATEWAY_TELEPORT_VISUAL_PURPLE, true);

        float speedZ = 5.0f;
        float speedXY = player->GetExactDist2d(targetGate) * 10.0f / speedZ;
        player->GetMotionMaster()->MoveJump(targetGate->GetPosition(), speedXY, speedZ);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_demonic_gateway_interact::CheckCast);
        OnCast += SpellCastFn(spell_warl_demonic_gateway_interact::HandleTeleport);
    }
};

// 59262 (green) / 59271 (purple) - Demonic Gateway portals. Shared by both colors - just plays
// the portal visual on spawn. Everything else (click-to-cast wiring, non-attackable/no-client-
// control flags, passive react state) is DB-driven via creature_template/npc_spellclick_spells
// rather than runtime AI code, matching this project's established preference (e.g. the
// existing Gunship Turret spell-click NPC configuration) over the reference's legacy
// UNIT_FIELD_INTERACT_SPELLID-poking approach.
struct npc_warl_demonic_gateway : public ScriptedAI
{
    npc_warl_demonic_gateway(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->CastSpell(me, SPELL_WARLOCK_GATEWAY_PORTAL_VISUAL, true);
    }
};

// 152108 - Cataclysm
class spell_warl_cataclysm : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_IMMOLATE_PERIODIC });
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_WARLOCK_IMMOLATE_PERIODIC, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_cataclysm::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 196447 - Channel Demonfire
// Periodically deals damage to a random nearby enemy carrying the caster's own Immolate.
class spell_warl_channel_demonfire : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_IMMOLATE_PERIODIC, SPELL_WARLOCK_CHANNEL_DEMONFIRE_DAMAGE });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<Unit*> enemies;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck checker(caster, caster, 100.0f);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(caster, enemies, checker);
        Cell::VisitAllObjects(caster, searcher, 100.0f);

        enemies.remove_if(Trinity::UnitAuraCheck(false, SPELL_WARLOCK_IMMOLATE_PERIODIC, caster->GetGUID()));
        if (enemies.empty())
            return;

        caster->CastSpell(Trinity::Containers::SelectRandomContainerElement(enemies), SPELL_WARLOCK_CHANNEL_DEMONFIRE_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_channel_demonfire::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 116858 - Chaos Bolt
class spell_warl_chaos_bolt : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_ERADICATION_DEBUFF });
    }

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    // 196412/196414 - Eradication (Destruction talent): Chaos Bolt applies a damage-taken
    // debuff to the target if the caster has the talent. Confirmed via DestinyCore/
    // AshamaneCore; the references' own standalone "Eradication" AuraScript does nothing
    // (its CheckProc always returns false) - the actual debuff application already lives on
    // Chaos Bolt's own hit handler in both references, matched here.
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), GetCaster()->ToPlayer()->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1)));

        if (Unit* caster = GetCaster())
            if (Unit* target = GetHitUnit())
                if (caster->HasAura(SPELL_WARLOCK_ERADICATION))
                    caster->CastSpell(target, SPELL_WARLOCK_ERADICATION_DEBUFF, true);
    }

    void CalcCritChance(Unit const* /*victim*/, float& critChance)
    {
        critChance = 100.0f;
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_chaos_bolt::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_warl_chaos_bolt::CalcCritChance);
    }
};

// 77220 - Mastery: Chaotic Energies
class spell_warl_chaotic_energies : public AuraScript
{
    void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        AuraEffect const* effect1 = GetEffect(EFFECT_1);
        if (!effect1 || !GetTargetApplication()->HasEffect(EFFECT_1))
        {
            PreventDefaultAction();
            return;
        }

        // You take ${$s2/3}% reduced damage
        float damageReductionPct = float(effect1->GetAmount()) / 3;
        // plus a random amount of up to ${$s2/3}% additional reduced damage
        damageReductionPct += frand(0.0f, damageReductionPct);

        absorbAmount = CalculatePct(dmgInfo.GetDamage(), damageReductionPct);
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_warl_chaotic_energies::HandleAbsorb, EFFECT_2);
    }
};

// 17962 - Conflagrate
class spell_warl_conflagrate : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_WARLOCK_CONFLAGRATE_ENERGIZE });
    }

    void HandleAfterCast(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_CONFLAGRATE_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_conflagrate::HandleAfterCast, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 6201 - Create Healthstone
class spell_warl_create_healthstone : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_CREATE_HEALTHSTONE });
    }

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_CREATE_HEALTHSTONE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_create_healthstone::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 34130 - Create Healthstone (Soulwell)
// Grants a healthstone to whoever clicks the Soulwell object, same as the player's own personal
// Create Healthstone (6201, spell_warl_create_healthstone above).
class spell_warl_create_healthstone_soulwell : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_CREATE_HEALTHSTONE });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_CREATE_HEALTHSTONE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_create_healthstone_soulwell::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 108416 - Dark Pact
class spell_warl_dark_pact : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 }, { spellInfo->Id, EFFECT_2 } });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = false;
        if (Unit* caster = GetCaster())
        {
            float extraAmount = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * 2.5f;
            int32 absorb = caster->CountPctFromCurHealth(GetEffectInfo(EFFECT_1).CalcValue(caster));
            caster->SetHealth(caster->GetHealth() - absorb);
            amount = CalculatePct(absorb, GetEffectInfo(EFFECT_2).CalcValue(caster)) + extraAmount;
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_dark_pact::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

struct spell_warl_deaths_embrace_impl
{
    static void HandleDamageOrHealingCalculation(Unit const* caster, Unit const* target, float& pctMod, SpellEffIndex inreaseEffect, SpellEffIndex healthLimitEffect)
    {
        Aura const* deathsEmbrace = caster->GetAura(SPELL_WARLOCK_DEATHS_EMBRACE, ObjectGuid::Empty, ObjectGuid::Empty, 1 << inreaseEffect | 1 << healthLimitEffect);
        if (!deathsEmbrace)
            return;

        if (!target->HealthBelowPct(deathsEmbrace->GetEffect(healthLimitEffect)->GetAmount()))
            return;

        AddPct(pctMod, deathsEmbrace->GetEffect(inreaseEffect)->GetAmount());
    }
};

// Called by 980 - Agony, 146739 - Corruption and 316099 - Unstable Affliction
class spell_warl_deaths_embrace_dots : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellEffect({ { SPELL_WARLOCK_DEATHS_EMBRACE, EFFECT_3 } });
    }

    void CalculateDamage(AuraEffect const* /*aurEff*/, Unit const* victim, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        if (Unit const* caster = GetCaster())
            spell_warl_deaths_embrace_impl::HandleDamageOrHealingCalculation(caster, victim, pctMod, EFFECT_2, EFFECT_3);
    }

    void Register() override
    {
        DoEffectCalcDamageAndHealing += AuraEffectCalcDamageFn(spell_warl_deaths_embrace_dots::CalculateDamage, EFFECT_ALL, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 234153 - Drain Life
class spell_warl_deaths_embrace_drain_life : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellEffect({ { SPELL_WARLOCK_DEATHS_EMBRACE, EFFECT_1 } });
    }

    void CalculateHeal(AuraEffect const* /*aurEff*/, Unit const* victim, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        Unit const* caster = GetCaster();
        if (!caster)
            return;

        if (caster != victim)   // check who is being targeted, this hook is called for both damage and healing of PERIODIC_LEECH
            return;

        spell_warl_deaths_embrace_impl::HandleDamageOrHealingCalculation(caster, caster, pctMod, EFFECT_0, EFFECT_1);
    }

    void Register() override
    {
        DoEffectCalcDamageAndHealing += AuraEffectCalcHealingFn(spell_warl_deaths_embrace_drain_life::CalculateHeal, EFFECT_0, SPELL_AURA_PERIODIC_LEECH);
    }
};

// 264178 - Demonbolt
class spell_warl_demonbolt : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_WARLOCK_DEMONBOLT_ENERGIZE });
    }

    void HandleAfterCast() const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_DEMONBOLT_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_warl_demonbolt::HandleAfterCast);
    }
};

// 48018 - Demonic Circle: Summon
class spell_warl_demonic_circle_summon : public AuraScript
{
    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        // If effect is removed by expire remove the summoned demonic circle too.
        if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
            GetTarget()->RemoveGameObject(GetId(), true);

        GetTarget()->RemoveAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST);
    }

    void HandleDummyTick(AuraEffect const* /*aurEff*/)
    {
        if (GameObject* circle = GetTarget()->GetGameObject(GetId()))
        {
            // Here we check if player is in demonic circle teleport range, if so add
            // WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST; allowing him to cast the WARLOCK_DEMONIC_CIRCLE_TELEPORT.
            // If not in range remove the WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST.

            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_WARLOCK_DEMONIC_CIRCLE_TELEPORT, GetCastDifficulty());

            if (GetTarget()->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
            {
                if (!GetTarget()->HasAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST))
                    GetTarget()->CastSpell(GetTarget(), SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST, true);
            }
            else
                GetTarget()->RemoveAura(SPELL_WARLOCK_DEMONIC_CIRCLE_ALLOW_CAST);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectApplyFn(spell_warl_demonic_circle_summon::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_demonic_circle_summon::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 48020 - Demonic Circle: Teleport
class spell_warl_demonic_circle_teleport : public AuraScript
{
    void HandleTeleport(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
        {
            if (GameObject* circle = player->GetGameObject(SPELL_WARLOCK_DEMONIC_CIRCLE_SUMMON))
            {
                player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                player->RemoveMovementImpairingAuras(false);
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_warl_demonic_circle_teleport::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 67518, 19505 - Devour Magic
class spell_warl_devour_magic : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_GLYPH_OF_DEMON_TRAINING, SPELL_WARLOCK_DEVOUR_MAGIC_HEAL })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void OnSuccessfulDispel(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        args.AddSpellBP0(GetEffectInfo(EFFECT_1).CalcValue(caster));

        caster->CastSpell(caster, SPELL_WARLOCK_DEVOUR_MAGIC_HEAL, args);

        // Glyph of Felhunter
        if (Unit* owner = caster->GetOwner())
            if (owner->GetAura(SPELL_WARLOCK_GLYPH_OF_DEMON_TRAINING))
                owner->CastSpell(owner, SPELL_WARLOCK_DEVOUR_MAGIC_HEAL, args);
    }

    void Register() override
    {
        OnEffectSuccessfulDispel += SpellEffectFn(spell_warl_devour_magic::OnSuccessfulDispel, EFFECT_0, SPELL_EFFECT_DISPEL);
    }
};

// 603 - Doom
class spell_warl_doom : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_WARLOCK_DOOM_ENERGIZE });
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_WARLOCK_DOOM_ENERGIZE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_doom::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 198590 - Drain Soul
class spell_warl_drain_soul : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo ({ SPELL_WARLOCK_DRAIN_SOUL_ENERGIZE })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            return;

        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_WARLOCK_DRAIN_SOUL_ENERGIZE, true);
    }

    void CalculateDamage(AuraEffect const* /*aurEff*/, Unit const* victim, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        if (victim->HealthBelowPct(GetEffectInfo(EFFECT_2).CalcValue(GetCaster())))
            AddPct(pctMod, GetEffectInfo(EFFECT_1).CalcValue(GetCaster()));
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectApplyFn(spell_warl_drain_soul::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        DoEffectCalcDamageAndHealing += AuraEffectCalcDamageFn(spell_warl_drain_soul::CalculateDamage, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 171975 - Grimoire of Synergy
// Applies its own passive aura to both the caster and their guardian pet; whichever one procs
// (damages or heals) has a chance to buff the other.
class spell_warl_grimoire_of_synergy : public SpellScript
{
    void HandleCast() const
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        player->AddAura(GetSpellInfo()->Id, player);
        if (Guardian* pet = player->GetGuardianPet())
            player->AddAura(GetSpellInfo()->Id, pet);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_warl_grimoire_of_synergy::HandleCast);
    }
};

// 171975 - Grimoire of Synergy (proc half)
class spell_warl_grimoire_of_synergy_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_GRIMOIRE_OF_SYNERGY_BUFF });
    }

    bool CheckProc(ProcEventInfo& eventInfo) const
    {
        Unit* actor = eventInfo.GetActor();
        if (!actor)
            return false;

        if (actor->IsPet() || actor->IsGuardian())
        {
            Unit* owner = actor->GetOwner();
            if (!owner)
                return false;

            if (roll_chance_i(10))
                owner->CastSpell(owner, SPELL_WARLOCK_GRIMOIRE_OF_SYNERGY_BUFF, true);
            return true;
        }

        if (Player* player = actor->ToPlayer())
        {
            Guardian* guardian = player->GetGuardianPet();
            if (!guardian)
                return false;

            if (roll_chance_i(10))
                player->CastSpell(guardian, SPELL_WARLOCK_GRIMOIRE_OF_SYNERGY_BUFF, true);
            return true;
        }

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_warl_grimoire_of_synergy_aura::CheckProc);
    }
};

// 105174 - Hand of Gul'dan
// Summons Wild Imps (one extra per shard spent, capped) and deals damage scaled the same way,
// via 86040; Hand of Doom additionally applies Doom to the target. Deliberately merged into a
// single script rather than the reference's separate "damage" script: the damage spell (86040)
// has no Soul Shard cost of its own, so a script bound to it can't independently re-derive how
// many shards the original cast spent - doing the scaling here, where that information is still
// available, avoids that dependency entirely.
class spell_warl_hand_of_guldan : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellEffect({ { SPELL_WARLOCK_HAND_OF_GULDAN_DAMAGE, EFFECT_0 } })
            && ValidateSpellInfo({ SPELL_WARLOCK_HAND_OF_GULDAN_SUMMON, SPELL_WARLOCK_HAND_OF_DOOM, SPELL_WARLOCK_DOOM });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        int32 shardsSpent = GetSpell()->GetPowerTypeCostAmount(POWER_SOUL_SHARDS).value_or(0);

        static float const offsetX[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
        static float const offsetY[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

        for (int32 i = 1; i < std::min(shardsSpent + 1, 4); ++i)
        {
            Position dest(target->GetPositionX() + offsetX[i], target->GetPositionY() + offsetY[i], target->GetPositionZ());
            caster->CastSpell(dest, SPELL_WARLOCK_HAND_OF_GULDAN_SUMMON, true);
        }

        int32 baseDamage = sSpellMgr->AssertSpellInfo(SPELL_WARLOCK_HAND_OF_GULDAN_DAMAGE, GetCastDifficulty())->GetEffect(EFFECT_0).CalcValue(caster);
        caster->CastSpell(target, SPELL_WARLOCK_HAND_OF_GULDAN_DAMAGE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_FULL_MASK,
            .SpellValueOverrides = { { SPELLVALUE_BASE_POINT0, baseDamage * std::min(shardsSpent + 1, 3) } }
        });

        if (caster->HasAura(SPELL_WARLOCK_HAND_OF_DOOM))
            caster->CastSpell(target, SPELL_WARLOCK_DOOM, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_hand_of_guldan::HandleOnHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 196277 - Implosion (Demonology): commands all active Wild Imp pets to fly to the target and
// explode, dealing damage. Both DestinyCore and AshamaneCore have this ability's real logic
// entirely commented out (dead code referencing an undefined PET_ENTRY_WILD_IMP that never
// compiled even in the reference) - the intended design is clear from the comment though, and
// was used to inform this implementation: iterate Unit::m_Controlled (the established idiom
// for "my current pets/guardians," e.g. spell_dk.cpp's Dancing Rune Weapon lookup), filtered
// to the Wild Imp creature entry, guarded the same safe way DK's lookup is (Validate() checks
// sObjectMgr->GetCreatureTemplate() first, so the script simply no-ops if the entry somehow
// isn't in this server's DB rather than assuming it blindly). Uses the
// Unit::m_Events/AddEventAtOffset lambda idiom for the ~1 sec travel delay before each imp
// detonates, matching this project's own established pattern for staggered delayed casts.
class spell_warl_implosion : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sObjectMgr->GetCreatureTemplate(NPC_WARLOCK_WILD_IMP))
            return false;
        return ValidateSpellInfo({ SPELL_WARLOCK_IMPLOSION_JUMP, SPELL_WARLOCK_IMPLOSION_DAMAGE });
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        ObjectGuid targetGuid = target->GetGUID();

        for (Unit* controlled : caster->m_Controlled)
        {
            Creature* imp = controlled->GetEntry() == NPC_WARLOCK_WILD_IMP ? controlled->ToCreature() : nullptr;
            if (!imp)
                continue;

            imp->CastStop();
            imp->CastSpell(target, SPELL_WARLOCK_IMPLOSION_JUMP, true);

            ObjectGuid impGuid = imp->GetGUID();
            caster->m_Events.AddEventAtOffset([caster, impGuid, targetGuid]()
            {
                Creature* imp = ObjectAccessor::GetCreature(*caster, impGuid);
                if (!imp)
                    return;

                if (Unit* target = ObjectAccessor::GetUnit(*caster, targetGuid))
                    imp->CastSpell(target, SPELL_WARLOCK_IMPLOSION_DAMAGE, true);

                imp->DisappearAndDie();
            }, 1000ms);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_implosion::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 48181 - Haunt
class spell_warl_haunt : public AuraScript
{
    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
            if (Unit* caster = GetCaster())
                caster->GetSpellHistory()->ResetCooldown(GetId(), true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectApplyFn(spell_warl_haunt::HandleRemove, EFFECT_1, SPELL_AURA_MOD_SCHOOL_MASK_DAMAGE_FROM_CASTER, AURA_EFFECT_HANDLE_REAL);
    }
};

// 80240 - Havoc
// Applied to a secondary target; whenever the caster damages a different target while it's up,
// copies that damage onto the Havoc'd target too.
class spell_warl_havoc : public AuraScript
{
    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& procInfo) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* victim = procInfo.GetActionTarget();
        Unit* target = procInfo.GetProcTarget();
        if (!victim || !target || victim == target)
            return;

        DamageInfo const* damageInfo = procInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        SpellNonMeleeDamage copy(caster, target, aurEff->GetSpellInfo(), aurEff->GetBase()->GetSpellVisual(), aurEff->GetSpellInfo()->SchoolMask);
        copy.damage = damageInfo->GetDamage();
        copy.cleanDamage = copy.damage;
        caster->DealSpellDamage(&copy, false);
        caster->SendSpellNonMeleeDamageLog(&copy);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_warl_havoc::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 755 - Health Funnel
class spell_warl_health_funnel : public AuraScript
{
    void ApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* target = GetTarget();
        if (caster->HasAura(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R2))
            target->CastSpell(target, SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2, true);
        else if (caster->HasAura(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_R1))
            target->CastSpell(target, SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1, true);
    }

    void RemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->RemoveAurasDueToSpell(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R1);
        target->RemoveAurasDueToSpell(SPELL_WARLOCK_IMPROVED_HEALTH_FUNNEL_BUFF_R2);
    }

    void OnPeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        //! HACK for self damage, is not blizz :/
        uint32 damage = caster->CountPctFromMaxHealth(aurEff->GetBaseAmount());

        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetSpellInfo(), SpellModOp::PowerCost0, damage);

        SpellNonMeleeDamage damageInfo(caster, caster, GetSpellInfo(), GetAura()->GetSpellVisual(), GetSpellInfo()->SchoolMask, GetAura()->GetCastId());
        damageInfo.periodicLog = true;
        damageInfo.damage = damage;
        caster->DealSpellDamage(&damageInfo, false);
        caster->SendSpellNonMeleeDamageLog(&damageInfo);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_warl_health_funnel::ApplyEffect, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_warl_health_funnel::RemoveEffect, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_health_funnel::OnPeriodic, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH);
    }
};

// 6262 - Healthstone
class spell_warl_healthstone_heal : public SpellScript
{
    void HandleOnHit()
    {
        int32 heal = int32(CalculatePct(GetCaster()->GetCreateHealth(), GetHitHeal()));
        SetHitHeal(heal);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_warl_healthstone_heal::HandleOnHit);
    }
};

// 348 - Immolate
class spell_warl_immolate : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_IMMOLATE_PERIODIC});
    }

    void HandleOnEffectHit(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_WARLOCK_IMMOLATE_PERIODIC, GetSpell());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_immolate::HandleOnEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 5740 - Rain of Fire
/// Updated 7.1.5
class spell_warl_rain_of_fire : public AuraScript
{
    void HandleDummyTick(AuraEffect const* /*aurEff*/)
    {
        std::vector<AreaTrigger*> rainOfFireAreaTriggers = GetTarget()->GetAreaTriggers(SPELL_WARLOCK_RAIN_OF_FIRE);
        GuidUnorderedSet targetsInRainOfFire;

        for (AreaTrigger* rainOfFireAreaTrigger : rainOfFireAreaTriggers)
        {
            GuidUnorderedSet const& insideTargets = rainOfFireAreaTrigger->GetInsideUnits();
            targetsInRainOfFire.insert(insideTargets.begin(), insideTargets.end());
        }

        for (ObjectGuid insideTargetGuid : targetsInRainOfFire)
            if (Unit* insideTarget = ObjectAccessor::GetUnit(*GetTarget(), insideTargetGuid))
                if (!GetTarget()->IsFriendlyTo(insideTarget))
                    GetTarget()->CastSpell(insideTarget, SPELL_WARLOCK_RAIN_OF_FIRE_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_rain_of_fire::HandleDummyTick, EFFECT_3, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Called by 17962 - Conflagrate
class spell_warl_roaring_blaze : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_WARLOCK_ROARING_BLAZE, SPELL_WARLOCK_CONFLAGRATE_DEBUFF });
    }

    bool Load() override
    {
        return GetCaster()->HasAura(SPELL_WARLOCK_ROARING_BLAZE);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_WARLOCK_CONFLAGRATE_DEBUFF, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_roaring_blaze::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 6358 - Seduction (Special Ability)
class spell_warl_seduction : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_GLYPH_OF_SUCCUBUS, SPELL_PRIEST_SHADOW_WORD_DEATH });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (Unit* target = GetHitUnit())
        {
            if (caster->GetOwner() && caster->GetOwner()->HasAura(SPELL_WARLOCK_GLYPH_OF_SUCCUBUS))
            {
                target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, ObjectGuid::Empty, target->GetAura(SPELL_PRIEST_SHADOW_WORD_DEATH)); // SW:D shall not be removed.
                target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_seduction::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 27285 - Seed of Corruption (damage)
class spell_warl_seed_of_corruption : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_CORRUPTION_DAMAGE });
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_WARLOCK_CORRUPTION_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_seed_of_corruption::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_warl_seed_of_corruption_dummy : public SpellScript
{
    void RemoveVisualMissile(WorldObject*& target) const
    {
        target = nullptr;
    }

    void SelectTarget(std::list<WorldObject*>& targets) const
    {
        if (targets.size() < 2)
            return;

        if (!GetExplTargetUnit()->HasAura(GetSpellInfo()->Id, GetCaster()->GetGUID()))
        {
            // primary target doesn't have seed, keep it
            targets.clear();
            targets.push_back(GetExplTargetUnit());
        }
        else
        {
            // primary target has seed, select random other target with no seed
            targets.remove_if(Trinity::UnitAuraCheck(true, GetSpellInfo()->Id, GetCaster()->GetGUID()));
            if (!targets.empty())
                Trinity::Containers::RandomResize(targets, 1);
            else
                targets.push_back(GetExplTargetUnit());
        }
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_warl_seed_of_corruption_dummy::RemoveVisualMissile, EFFECT_0, TARGET_UNIT_TARGET_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_seed_of_corruption_dummy::SelectTarget, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_seed_of_corruption_dummy::SelectTarget, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// 27243 - Seed of Corruption
class spell_warl_seed_of_corruption_dummy_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SEED_OF_CORRUPTION_DAMAGE });
    }

    void OnPeriodic(AuraEffect const* aurEff) const
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget(), SPELL_WARLOCK_SEED_OF_CORRUPTION_DAMAGE, aurEff);
    }

    void CalculateBuffer(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        amount = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * GetEffectInfo(EFFECT_0).CalcValue(caster) / 100;
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        DamageInfo const* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (!damageInfo->GetAttacker() || damageInfo->GetAttacker() != caster)
            return;

        // other seed explosions detonate this instantly, no matter what damage amount is
        if (!damageInfo->GetSpellInfo() || damageInfo->GetSpellInfo()->Id != SPELL_WARLOCK_SEED_OF_CORRUPTION_DAMAGE)
        {
            int32 amount = aurEff->GetAmount() - damageInfo->GetDamage();
            if (amount > 0)
            {
                aurEff->SetAmount(amount);
                if (!GetTarget()->HealthBelowPctDamaged(1, damageInfo->GetDamage()))
                    return;
            }
        }

        Remove();

        caster->CastSpell(eventInfo.GetActionTarget(), SPELL_WARLOCK_SEED_OF_CORRUPTION_DAMAGE, aurEff);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_seed_of_corruption_dummy_aura::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_seed_of_corruption_dummy_aura::CalculateBuffer, EFFECT_2, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_warl_seed_of_corruption_dummy_aura::HandleProc, EFFECT_2, SPELL_AURA_DUMMY);
    }
};

// 32863 - Seed of Corruption
// 36123 - Seed of Corruption
// 38252 - Seed of Corruption
// 39367 - Seed of Corruption
// 44141 - Seed of Corruption
// 70388 - Seed of Corruption
// Monster spells, triggered only on amount drop (not on death)
class spell_warl_seed_of_corruption_generic : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SEED_OF_CORRUPTION_GENERIC });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        int32 amount = aurEff->GetAmount() - damageInfo->GetDamage();
        if (amount > 0)
        {
            const_cast<AuraEffect*>(aurEff)->SetAmount(amount);
            return;
        }

        Remove();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(eventInfo.GetActionTarget(), SPELL_WARLOCK_SEED_OF_CORRUPTION_GENERIC, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_warl_seed_of_corruption_generic::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 17877 - Shadowburn
class spell_warl_shadowburn : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SHADOWBURN_ENERGIZE })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_3 } });
    }

    void HandleEnergize() const
    {
        if (GetHitUnit()->IsAlive())
            return;

        // killing target with current spell doesn't apply the aura (apply/remove scripts don't execute)
        // but we can use the fact that it still gets created and immediately marked as removed to detect that case
        Aura* hitAura = GetHitAura(false, true);
        if (!hitAura || !hitAura->IsRemoved())
            return;

        TryEnergize(Object::ToPlayer(GetCaster()), GetHitUnit(), GetSpellInfo(), GetSpell(), nullptr);
    }

    void CalcCritChance(Unit const* victim, float& critChance) const
    {
        if (victim->HealthBelowPct(GetEffectInfo(EFFECT_3).CalcValue(GetCaster())))
            critChance += GetEffectInfo(EFFECT_2).CalcValue(GetCaster());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_warl_shadowburn::HandleEnergize);
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_warl_shadowburn::CalcCritChance);
    }

public:
    static void TryEnergize(Player* caster, Unit const* target, SpellInfo const* spellInfo,
        Spell const* triggeringSpell, AuraEffect const* triggeringAura)
    {
        if (!caster)
            return;

        if (caster->isHonorOrXPTarget(target))
        {
            caster->CastSpell(caster, SPELL_WARLOCK_SHADOWBURN_ENERGIZE, CastSpellExtraArgsInit{
                .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
                .TriggeringSpell = triggeringSpell,
                .TriggeringAura = triggeringAura
            });

            caster->GetSpellHistory()->RestoreCharge(spellInfo->ChargeCategoryId);
        }
    }
};

class spell_warl_shadowburn_aura : public AuraScript
{
    void RemoveEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/) const
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            return;

        spell_warl_shadowburn::TryEnergize(Object::ToPlayer(GetCaster()), GetTarget(), GetSpellInfo(), nullptr, aurEff);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_warl_shadowburn_aura::RemoveEffect, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 686 - Shadow Bolt
class spell_warl_shadow_bolt : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_WARLOCK_SHADOW_BOLT_ENERGIZE });
    }

    void HandleAfterCast()
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_SHADOW_BOLT_ENERGIZE, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_warl_shadow_bolt::HandleAfterCast);
    }
};

enum WarlockPetSpells
{
    SPELL_SUCCUBUS_WHIPLASH        = 6360,
    SPELL_VOIDWALKER_SUFFERING     = 17735,
    SPELL_FELHUNTER_SPELL_LOCK     = 19647,
    SPELL_IMP_CAUTERIZE_MASTER     = 119899,
    SPELL_FELGUARD_FELSTORM        = 89751,
    SPELL_INFERNAL_METEOR_STRIKE   = 171017,
    SPELL_DOOMGUARD_SHADOW_LOCK    = 171138
};

// 171140 - Shadow Lock
// Commands a Doomguard guardian pet to interrupt/silence the target, mirroring the pet ability's
// cooldown onto the player's own command spell.
class spell_warl_shadow_lock : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DOOMGUARD_SHADOW_LOCK });
    }

    SpellCastResult CheckCast() const
    {
        Guardian* pet = GetCaster()->GetGuardianPet();
        if (!pet)
            return SPELL_FAILED_DONT_REPORT;

        if (pet->GetSpellHistory()->HasCooldown(SPELL_DOOMGUARD_SHADOW_LOCK))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Guardian* pet = caster->GetGuardianPet();
        if (!pet || !target)
            return;

        pet->CastSpell(target, SPELL_DOOMGUARD_SHADOW_LOCK, true);
        caster->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, 24s);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_shadow_lock::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_warl_shadow_lock::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 6353 - Soul Fire
class spell_warl_soul_fire : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_WARLOCK_SOUL_FIRE_ENERGIZE,
            SPELL_WARLOCK_IMMOLATE_PERIODIC
        });
    }

    void HandleTriggers(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();

        caster->CastSpell(caster, SPELL_WARLOCK_SOUL_FIRE_ENERGIZE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });

        caster->CastSpell(GetHitUnit(), SPELL_WARLOCK_IMMOLATE_PERIODIC, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_warl_soul_fire::HandleTriggers, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 196098 - Soul Harvest
// Extends its own duration based on how many nearby enemies carry the caster's spec DoT (Agony
// for Affliction, Doom for Demonology, Immolate for Destruction), capped at 15 targets and at the
// spell's own max duration (both read from its own DB2 effects rather than hardcoded).
class spell_warl_soul_harvest : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 }, { spellInfo->Id, EFFECT_2 } });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster)
            return;

        uint32 dotSpellId = SPELL_WARLOCK_AGONY;
        switch (caster->GetPrimarySpecialization())
        {
            case ChrSpecialization::WarlockDemonology:
                dotSpellId = SPELL_WARLOCK_DOOM;
                break;
            case ChrSpecialization::WarlockDestruction:
                dotSpellId = SPELL_WARLOCK_IMMOLATE_PERIODIC;
                break;
            default:
                break;
        }

        std::list<Unit*> enemies;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck checker(caster, caster, 100.0f);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(caster, enemies, checker);
        Cell::VisitAllObjects(caster, searcher, 100.0f);
        enemies.remove_if(Trinity::UnitAuraCheck(false, dotSpellId, caster->GetGUID()));

        int64 affectedCount = std::min<int64>(int64(enemies.size()), 15);
        Milliseconds perTarget = Seconds(GetEffectInfo(EFFECT_1).CalcValue());
        Milliseconds maxDuration = Seconds(GetEffectInfo(EFFECT_2).CalcValue());
        Milliseconds newDuration = std::min(Milliseconds(GetAura()->GetMaxDuration()) + perTarget * affectedCount, maxDuration);

        GetAura()->SetMaxDuration(newDuration.count());
        GetAura()->SetDuration(newDuration.count());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_warl_soul_harvest::HandleApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 86121 - Soul Swap
class spell_warl_soul_swap : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_WARLOCK_GLYPH_OF_SOUL_SWAP,
            SPELL_WARLOCK_SOUL_SWAP_CD_MARKER,
            SPELL_WARLOCK_SOUL_SWAP_OVERRIDE
        });
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_SOUL_SWAP_OVERRIDE, true);
        GetHitUnit()->CastSpell(GetCaster(), SPELL_WARLOCK_SOUL_SWAP_DOT_MARKER, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_soul_swap::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 86211 - Soul Swap Override - Also acts as a dot container
class spell_warl_soul_swap_override : public AuraScript
{
    //! Forced to, pure virtual functions must have a body when linking
    void Register() override { }

public:
    void AddDot(uint32 id)                          { _dotList.push_back(id); }
    std::list<uint32> const GetDotList() const      { return _dotList; }
    Unit* GetOriginalSwapSource() const             { return _swapCaster; }
    void SetOriginalSwapSource(Unit* victim)        { _swapCaster = victim; }

private:
    std::list<uint32> _dotList;
    Unit* _swapCaster = nullptr;
};

//! Soul Swap Copy Spells - 92795 - Simply copies spell IDs.
class spell_warl_soul_swap_dot_marker : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* swapVictim = GetCaster();
        Unit* warlock    = GetHitUnit();
        if (!warlock || !swapVictim)
            return;

        Unit::AuraApplicationMap const& appliedAuras = swapVictim->GetAppliedAuras();
        spell_warl_soul_swap_override* swapSpellScript = nullptr;
        if (Aura* swapOverrideAura = warlock->GetAura(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
            swapSpellScript = swapOverrideAura->GetScript<spell_warl_soul_swap_override>();

        if (!swapSpellScript)
            return;

        flag128 classMask = GetEffectInfo().SpellClassMask;

        for (Unit::AuraApplicationMap::const_iterator itr = appliedAuras.begin(); itr != appliedAuras.end(); ++itr)
        {
            SpellInfo const* spellProto = itr->second->GetBase()->GetSpellInfo();
            if (itr->second->GetBase()->GetCaster() == warlock)
                if (spellProto->SpellFamilyName == SPELLFAMILY_WARLOCK && (spellProto->SpellFamilyFlags & classMask))
                    swapSpellScript->AddDot(itr->first);
        }

        swapSpellScript->SetOriginalSwapSource(swapVictim);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_soul_swap_dot_marker::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 86213 - Soul Swap Exhale
class spell_warl_soul_swap_exhale : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SOUL_SWAP_MOD_COST, SPELL_WARLOCK_SOUL_SWAP_OVERRIDE });
    }

    SpellCastResult CheckCast()
    {
        Unit* currentTarget = GetExplTargetUnit();
        Unit* swapTarget = nullptr;
        if (Aura const* swapOverride = GetCaster()->GetAura(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
            if (spell_warl_soul_swap_override* swapScript = swapOverride->GetScript<spell_warl_soul_swap_override>())
                swapTarget = swapScript->GetOriginalSwapSource();

        // Soul Swap Exhale can't be cast on the same target than Soul Swap
        if (swapTarget && currentTarget && swapTarget == currentTarget)
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void OnEffectHit(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_SOUL_SWAP_MOD_COST, true);
        bool hasGlyph = GetCaster()->HasAura(SPELL_WARLOCK_GLYPH_OF_SOUL_SWAP);

        std::list<uint32> dotList;
        Unit* swapSource = nullptr;
        if (Aura const* swapOverride = GetCaster()->GetAura(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE))
        {
            spell_warl_soul_swap_override* swapScript = swapOverride->GetScript<spell_warl_soul_swap_override>();
            if (!swapScript)
                return;
            dotList = swapScript->GetDotList();
            swapSource = swapScript->GetOriginalSwapSource();
        }

        if (dotList.empty())
            return;

        for (std::list<uint32>::const_iterator itr = dotList.begin(); itr != dotList.end(); ++itr)
        {
            GetCaster()->AddAura(*itr, GetHitUnit());
            if (!hasGlyph && swapSource)
                swapSource->RemoveAurasDueToSpell(*itr);
        }

        // Remove Soul Swap Exhale buff
        GetCaster()->RemoveAurasDueToSpell(SPELL_WARLOCK_SOUL_SWAP_OVERRIDE);

        if (hasGlyph) // Add a cooldown on Soul Swap if caster has the glyph
            GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_SOUL_SWAP_CD_MARKER, false);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_soul_swap_exhale::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_warl_soul_swap_exhale::OnEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 228974 - Soul Leech (baseline passive, all specs): dealing damage grants an absorb shield
// scaled by the damage dealt, capped at 15% max health. Confirmed via DestinyCore/AshamaneCore
// (identical implementations), but simplified: the reference also raises the cap to 20% if the
// caster has "Demonskin" (a separate talent) - not ported, since Demonskin itself isn't
// implemented in ArgusCore yet either (left as a documented follow-up rather than guessed at).
// The reference's own separate "Soul Leach appliers" system (137046/137044/137043, cast on pet
// summon) is unrelated pre-Legion drift under a similar name - modern Soul Leech is a simple
// always-active baseline passive, not something that needs applying via a pet summon at all,
// so that piece was not ported either. Translated `CastCustomSpell` (doesn't exist in
// ArgusCore) to `CastSpellExtraArgs`/`AddSpellMod`, and the raw
// `GetSpellInfo()->GetEffect(EFFECT_N)->BasePoints` pointer access (`SpellEffectInfo` has no
// `operator->`) to `GetEffectInfo(EFFECT_N).CalcValue()`.
class spell_warl_soul_leech : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SOUL_LEECH_ABSORB });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return false;

        DamageInfo const* damageInfo = eventInfo.GetDamageInfo();
        int32 absorb = damageInfo ? CalculatePct(int32(damageInfo->GetDamage()), GetEffectInfo(EFFECT_0).CalcValue(caster)) : 0;

        // Add the remaining amount if the shield is already up
        if (Aura const* existing = caster->GetAura(SPELL_WARLOCK_SOUL_LEECH_ABSORB))
            if (AuraEffect const* existingEffect = existing->GetEffect(EFFECT_0))
                absorb += existingEffect->GetAmount();

        int32 threshold = CalculatePct(int32(caster->GetMaxHealth()), GetEffectInfo(EFFECT_1).CalcValue(caster));
        absorb = std::min(absorb, threshold);

        caster->CastSpell(caster, SPELL_WARLOCK_SOUL_LEECH_ABSORB, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, absorb));
        return true;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_warl_soul_leech::CheckProc);
    }
};

// 219272 - Demonskin (PvP Honor Talent, lower priority): periodically tops up the Soul Leech
// absorb shield by 1% max health per tick, capped at this talent's own (higher) threshold.
// Confirmed via DestinyCore/AshamaneCore (identical implementations). Translated
// `CastCustomSpell` (doesn't exist in ArgusCore) to `CastSpellExtraArgs`/`AddSpellMod`, and the
// raw `GetSpellInfo()->GetEffect(EFFECT_1)->BasePoints` pointer access (`SpellEffectInfo` has
// no `operator->`) to `GetEffectInfo(EFFECT_1).CalcValue()`.
class spell_warl_demonskin : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SOUL_LEECH_ABSORB });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 absorb = int32(caster->CountPctFromMaxHealth(1));

        if (Aura const* existing = caster->GetAura(SPELL_WARLOCK_SOUL_LEECH_ABSORB))
            if (AuraEffect const* existingEffect = existing->GetEffect(EFFECT_0))
                absorb += existingEffect->GetAmount();

        int32 threshold = int32(caster->CountPctFromMaxHealth(GetEffectInfo(EFFECT_1).CalcValue(caster)));
        absorb = std::min(absorb, threshold);

        caster->CastSpell(caster, SPELL_WARLOCK_SOUL_LEECH_ABSORB, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, absorb));
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_demonskin::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 205231 - Eye Laser (Darkglare pet ability, Affliction cooldown): overrides its own default
// target selection to hit every enemy within 100 yards currently afflicted by Doom, refreshing
// it on each. The Darkglare pet AI (npc_pet_warlock_darkglare in pet_warlock.cpp) only needs
// to find one valid Doom target to trigger this cast - this override does the real work of
// finding and hitting all of them. Confirmed via DestinyCore/AshamaneCore (identical
// implementations). Translated Trinity::AllWorldObjectsInRange/WorldObjectListSearcher (the
// latter doesn't exist in ArgusCore) to the established
// Trinity::AnyUnfriendlyUnitInObjectRangeCheck/UnitListSearcher idiom used elsewhere this
// session, filtering to Unit* results and re-checking the Doom aura directly rather than
// relying on a separate WorldObject-flavored searcher.
class spell_warl_eye_laser : public SpellScript
{
    void HandleTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetOriginalCaster();
        if (!caster)
            return;

        targets.clear();

        std::list<Unit*> nearby;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck checker(caster, caster, 100.0f);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(caster, nearby, checker);
        Cell::VisitAllObjects(caster, searcher, 100.0f);

        for (Unit* unit : nearby)
            if (unit->HasAura(SPELL_WARLOCK_DOOM, caster->GetGUID()))
                targets.push_back(unit);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_eye_laser::HandleTargets, EFFECT_0, TARGET_UNIT_TARGET_ENEMY);
    }
};

// 215941 - Soul Conduit (talent, all specs): each Soul Shard spent has a chance to be
// refunded. Confirmed via DestinyCore/AshamaneCore, but corrected a bug shared by both: they
// check `POWER_MANA` instead of `POWER_SOUL_SHARDS` when looking up how much was just spent -
// a mana-refund formula that was never adapted for what should be a Soul Shard refund.
// Translated `CastCustomSpell` (doesn't exist in ArgusCore) to
// `CastSpellExtraArgs`/`AddSpellMod`, and the raw
// `GetSpellInfo()->GetEffect(EFFECT_0)->BasePoints` pointer access (`SpellEffectInfo` has no
// `operator->`) to `aurEff->GetAmount()` (the same underlying DB2 value, already resolved by
// the aura-effect pipeline for the effect this hook is bound to).
class spell_warl_soul_conduit : public AuraScript
{
    int32 _refund = 0;

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SOUL_CONDUIT_REFUND });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (!caster || (eventInfo.GetActor() && eventInfo.GetActor() != caster))
            return false;

        Spell const* spell = eventInfo.GetProcSpell();
        if (!spell)
            return false;

        Optional<int32> cost = spell->GetPowerTypeCostAmount(POWER_SOUL_SHARDS);
        if (!cost || *cost <= 0)
            return false;

        _refund = *cost;
        return true;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (roll_chance_i(aurEff->GetAmount()))
            caster->CastSpell(caster, SPELL_WARLOCK_SOUL_CONDUIT_REFUND, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, _refund));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_warl_soul_conduit::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_warl_soul_conduit::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 29858 - Soulshatter
class spell_warl_soulshatter : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_SOULSHATTER_EFFECT });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (Unit* target = GetHitUnit())
            if (target->GetThreatManager().IsThreatenedBy(caster, true))
                caster->CastSpell(target, SPELL_WARLOCK_SOULSHATTER_EFFECT, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_soulshatter::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 119910 - Spell Lock
// Commands a Felhunter guardian pet to interrupt/silence the target, mirroring the pet ability's
// cooldown onto the player's own command spell.
class spell_warl_spell_lock : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FELHUNTER_SPELL_LOCK });
    }

    SpellCastResult CheckCast() const
    {
        Guardian* pet = GetCaster()->GetGuardianPet();
        if (!pet)
            return SPELL_FAILED_DONT_REPORT;

        if (pet->GetSpellHistory()->HasCooldown(SPELL_FELHUNTER_SPELL_LOCK))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Guardian* pet = caster->GetGuardianPet();
        if (!pet || !target)
            return;

        pet->CastSpell(target, SPELL_FELHUNTER_SPELL_LOCK, true);
        caster->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, 24s);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_spell_lock::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_warl_spell_lock::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 119907 - Suffering
// Commands a Voidwalker guardian pet to taunt/damage nearby enemies, mirroring the pet ability's
// cooldown onto the player's own command spell.
class spell_warl_suffering : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_VOIDWALKER_SUFFERING });
    }

    SpellCastResult CheckCast() const
    {
        Guardian* pet = GetCaster()->GetGuardianPet();
        if (!pet)
            return SPELL_FAILED_DONT_REPORT;

        if (pet->GetSpellHistory()->HasCooldown(SPELL_VOIDWALKER_SUFFERING))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Guardian* pet = caster->GetGuardianPet();
        if (!pet || !target)
            return;

        pet->CastSpell(target, SPELL_VOIDWALKER_SUFFERING, true);
        caster->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, 10s);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_suffering::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_warl_suffering::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 119914 - Felstorm
// Commands a Felguard guardian pet to attack; the pet's DB2 data already handles the actual
// mirrored cast, this script only needs to mirror its cooldown onto the player's command spell.
class spell_warl_felstorm : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FELGUARD_FELSTORM });
    }

    SpellCastResult CheckCast() const
    {
        Guardian* pet = GetCaster()->GetGuardianPet();
        if (!pet)
            return SPELL_FAILED_DONT_REPORT;

        if (pet->GetSpellHistory()->HasCooldown(SPELL_FELGUARD_FELSTORM))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleCast() const
    {
        GetCaster()->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, 45s);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_felstorm::CheckCast);
        AfterCast += SpellCastFn(spell_warl_felstorm::HandleCast);
    }
};

// 171152 - Meteor Strike
// Commands an Infernal guardian pet to slam the ground beneath itself.
class spell_warl_meteor_strike : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_INFERNAL_METEOR_STRIKE });
    }

    SpellCastResult CheckCast() const
    {
        Guardian* pet = GetCaster()->GetGuardianPet();
        if (!pet)
            return SPELL_FAILED_DONT_REPORT;

        if (pet->GetSpellHistory()->HasCooldown(SPELL_INFERNAL_METEOR_STRIKE))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleCast() const
    {
        Unit* caster = GetCaster();
        Guardian* pet = caster->GetGuardianPet();
        if (!pet)
            return;

        pet->CastSpell(pet, SPELL_INFERNAL_METEOR_STRIKE, true);
        caster->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, 60s);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_meteor_strike::CheckCast);
        AfterCast += SpellCastFn(spell_warl_meteor_strike::HandleCast);
    }
};

// 119905 - Cauterize Master
// Commands an Imp guardian pet to heal the caster.
class spell_warl_cauterize_master : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_IMP_CAUTERIZE_MASTER });
    }

    SpellCastResult CheckCast() const
    {
        Guardian* pet = GetCaster()->GetGuardianPet();
        if (!pet)
            return SPELL_FAILED_DONT_REPORT;

        if (pet->GetSpellHistory()->HasCooldown(SPELL_IMP_CAUTERIZE_MASTER))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleCast() const
    {
        Unit* caster = GetCaster();
        Guardian* pet = caster->GetGuardianPet();
        if (!pet)
            return;

        pet->CastSpell(caster, SPELL_IMP_CAUTERIZE_MASTER, true);
        caster->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, 30s);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_cauterize_master::CheckCast);
        AfterCast += SpellCastFn(spell_warl_cauterize_master::HandleCast);
    }
};

// 119909 - Whiplash
// Commands a Succubus guardian pet to fear enemies at the target destination.
class spell_warl_whiplash : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUCCUBUS_WHIPLASH });
    }

    SpellCastResult CheckCast() const
    {
        Guardian* pet = GetCaster()->GetGuardianPet();
        if (!pet)
            return SPELL_FAILED_DONT_REPORT;

        if (pet->GetSpellHistory()->HasCooldown(SPELL_SUCCUBUS_WHIPLASH))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        return SPELL_CAST_OK;
    }

    void HandleCast() const
    {
        Unit* caster = GetCaster();
        Guardian* pet = caster->GetGuardianPet();
        WorldLocation const* dest = GetExplTargetDest();
        if (!pet || !dest)
            return;

        pet->CastSpell(*dest, SPELL_SUCCUBUS_WHIPLASH, true);
        caster->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, 25s);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warl_whiplash::CheckCast);
        AfterCast += SpellCastFn(spell_warl_whiplash::HandleCast);
    }
};

// 37377 - Shadowflame
// 39437 - Shadowflame Hellfire and RoF
template <uint32 Trigger>
class spell_warl_t4_2p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ Trigger });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();
        caster->CastSpell(caster, Trigger, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_warl_t4_2p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 316099 - Unstable Affliction
class spell_warl_unstable_affliction : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WARLOCK_UNSTABLE_AFFLICTION_DAMAGE, SPELL_WARLOCK_UNSTABLE_AFFLICTION_ENERGIZE });
    }

    void HandleDispel(DispelInfo const* dispelInfo) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        AuraEffect const* removedEffect = GetEffect(EFFECT_1);
        if (!removedEffect)
            return;

        int32 damage = GetEffectInfo(EFFECT_0).CalcValue(caster, nullptr, GetUnitOwner()) / 100.0f * *removedEffect->CalculateEstimatedAmount(caster, removedEffect->GetAmount());
        caster->CastSpell(dispelInfo->GetDispeller(), SPELL_WARLOCK_UNSTABLE_AFFLICTION_DAMAGE, CastSpellExtraArgs()
            .AddSpellMod(SPELLVALUE_BASE_POINT0, damage)
            .SetTriggerFlags(TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR));
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            return;

        GetCaster()->CastSpell(GetCaster(), SPELL_WARLOCK_UNSTABLE_AFFLICTION_ENERGIZE, true);
    }

    void Register() override
    {
        AfterDispel += AuraDispelFn(spell_warl_unstable_affliction::HandleDispel);
        OnEffectRemove += AuraEffectRemoveFn(spell_warl_unstable_affliction::HandleRemove, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_warlock_spell_scripts()
{
    RegisterSpellScript(spell_warl_absolute_corruption);
    RegisterSpellScript(spell_warl_backdraft);
    RegisterSpellScript(spell_warl_banish);
    RegisterSpellAndAuraScriptPair(spell_warl_burning_rush, spell_warl_burning_rush_aura);
    RegisterSpellScript(spell_warl_call_dreadstalkers);
    RegisterSpellScript(spell_warl_demonic_calling);
    RegisterSpellScript(spell_warl_demonic_gateway);
    RegisterSpellScript(spell_warl_demonic_gateway_interact);
    RegisterCreatureAI(npc_warl_demonic_gateway);
    RegisterSpellScript(spell_warl_cataclysm);
    RegisterSpellScript(spell_warl_channel_demonfire);
    RegisterSpellScript(spell_warl_chaos_bolt);
    RegisterSpellScript(spell_warl_chaotic_energies);
    RegisterSpellScript(spell_warl_conflagrate);
    RegisterSpellScript(spell_warl_create_healthstone);
    RegisterSpellScript(spell_warl_create_healthstone_soulwell);
    RegisterSpellScript(spell_warl_dark_pact);
    RegisterSpellScript(spell_warl_deaths_embrace_dots);
    RegisterSpellScript(spell_warl_deaths_embrace_drain_life);
    RegisterSpellScript(spell_warl_demonbolt);
    RegisterSpellScript(spell_warl_demonic_circle_summon);
    RegisterSpellScript(spell_warl_demonic_circle_teleport);
    RegisterSpellScript(spell_warl_devour_magic);
    RegisterSpellScript(spell_warl_doom);
    RegisterSpellScript(spell_warl_drain_soul);
    RegisterSpellAndAuraScriptPair(spell_warl_grimoire_of_synergy, spell_warl_grimoire_of_synergy_aura);
    RegisterSpellScript(spell_warl_hand_of_guldan);
    RegisterSpellScript(spell_warl_implosion);
    RegisterSpellScript(spell_warl_haunt);
    RegisterSpellScript(spell_warl_havoc);
    RegisterSpellScript(spell_warl_health_funnel);
    RegisterSpellScript(spell_warl_healthstone_heal);
    RegisterSpellScript(spell_warl_immolate);
    RegisterSpellScript(spell_warl_rain_of_fire);
    RegisterSpellScript(spell_warl_roaring_blaze);
    RegisterSpellScript(spell_warl_seduction);
    RegisterSpellScript(spell_warl_seed_of_corruption);
    RegisterSpellAndAuraScriptPair(spell_warl_seed_of_corruption_dummy, spell_warl_seed_of_corruption_dummy_aura);
    RegisterSpellScript(spell_warl_seed_of_corruption_generic);
    RegisterSpellAndAuraScriptPair(spell_warl_shadowburn, spell_warl_shadowburn_aura);
    RegisterSpellScript(spell_warl_shadow_bolt);
    RegisterSpellScript(spell_warl_shadow_lock);
    RegisterSpellScript(spell_warl_soul_fire);
    RegisterSpellScript(spell_warl_soul_harvest);
    RegisterSpellScript(spell_warl_soul_swap);
    RegisterSpellScript(spell_warl_soul_swap_dot_marker);
    RegisterSpellScript(spell_warl_soul_swap_exhale);
    RegisterSpellScript(spell_warl_soul_swap_override);
    RegisterSpellScript(spell_warl_soul_conduit);
    RegisterSpellScript(spell_warl_soul_leech);
    RegisterSpellScript(spell_warl_demonskin);
    RegisterSpellScript(spell_warl_eye_laser);
    RegisterSpellScript(spell_warl_soulshatter);
    RegisterSpellScript(spell_warl_spell_lock);
    RegisterSpellScript(spell_warl_suffering);
    RegisterSpellScript(spell_warl_felstorm);
    RegisterSpellScript(spell_warl_meteor_strike);
    RegisterSpellScript(spell_warl_cauterize_master);
    RegisterSpellScript(spell_warl_whiplash);
    RegisterSpellScriptWithArgs(spell_warl_t4_2p_bonus<SPELL_WARLOCK_FLAMESHADOW>, "spell_warl_t4_2p_bonus_shadow");
    RegisterSpellScriptWithArgs(spell_warl_t4_2p_bonus<SPELL_WARLOCK_SHADOWFLAME>, "spell_warl_t4_2p_bonus_fire");
    RegisterSpellScript(spell_warl_unstable_affliction);
}
