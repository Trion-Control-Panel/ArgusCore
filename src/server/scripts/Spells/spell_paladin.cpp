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
 * Scripts for spells with SPELLFAMILY_PALADIN and SPELLFAMILY_GENERIC spells used by paladin players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pal_".
 */

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "CommonPredicates.h"
#include "Containers.h"
#include "DB2Stores.h"
#include "Group.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum PaladinSpells
{
    SPELL_PALADIN_ARDENT_DEFENDER_HEAL           = 66235,
    SPELL_PALADIN_ART_OF_WAR_TRIGGERED           = 231843,
    SPELL_PALADIN_AURA_OF_SACRIFICE               = 183416,
    SPELL_PALADIN_AURA_OF_SACRIFICE_ALLY          = 210372,
    SPELL_PALADIN_AURA_OF_SACRIFICE_DAMAGE        = 210380,
    SPELL_PALADIN_AVENGERS_SHIELD                = 31935,
    SPELL_PALADIN_AVENGING_WRATH                 = 31884,
    SPELL_PALADIN_BEACON_OF_FAITH                = 156910,
    SPELL_PALADIN_BEACON_OF_FAITH_PROC_AURA      = 177173,
    SPELL_PALADIN_BEACON_OF_LIGHT                = 53563,
    SPELL_PALADIN_BEACON_OF_LIGHT_HEAL           = 53652,
    SPELL_PALADIN_BEACON_OF_LIGHT_PROC_AURA      = 53651,
    SPELL_PALADIN_BEACON_OF_VIRTUE               = 200025,
    SPELL_PALADIN_BLADE_OF_JUSTICE               = 184575,
    SPELL_PALADIN_BLESSING_OF_FREEDOM            = 1044,
    SPELL_PALADIN_BLINDING_LIGHT_EFFECT          = 105421,
    SPELL_PALADIN_CONCENTRACTION_AURA            = 19746,
    SPELL_PALADIN_CONSECRATED_GROUND_PASSIVE     = 204054,
    SPELL_PALADIN_CONSECRATED_GROUND_SLOW        = 204242,
    SPELL_PALADIN_CONSECRATION                   = 26573,
    SPELL_PALADIN_CONSECRATION_DAMAGE            = 81297,
    SPELL_PALADIN_CONSECRATION_PROTECTION_AURA   = 188370,
    SPELL_PALADIN_CRUSADER_STRIKE                = 35395,
    SPELL_PALADIN_DIVINE_HAMMER                  = 198034,
    SPELL_PALADIN_DIVINE_INTERVENTION_HEAL       = 184250,
    SPELL_PALADIN_DIVINE_PURPOSE_TRIGGERED       = 223819,
    SPELL_PALADIN_DIVINE_SHIELD                  = 642,
    SPELL_PALADIN_DIVINE_STEED_HUMAN             = 221883,
    SPELL_PALADIN_DIVINE_STEED_DWARF             = 276111,
    SPELL_PALADIN_DIVINE_STEED_DRAENEI           = 221887,
    SPELL_PALADIN_DIVINE_STEED_BLOODELF          = 221886,
    SPELL_PALADIN_DIVINE_STEED_TAUREN            = 221885,
    SPELL_PALADIN_DIVINE_STORM_DAMAGE            = 224239,
    SPELL_PALADIN_ENDURING_LIGHT                 = 40471,
    SPELL_PALADIN_ENDURING_JUDGEMENT             = 40472,
    SPELL_PALADIN_EYE_FOR_AN_EYE_TRIGGERED       = 205202,
    SPELL_PALADIN_FERVENT_MARTYR_BUFF            = 223316,
    SPELL_PALADIN_FINAL_STAND                    = 204077,
    SPELL_PALADIN_FLASH_OF_LIGHT                 = 19750,
    SPELL_PALADIN_FINAL_STAND_EFFECT             = 204079,
    SPELL_PALADIN_FINAL_VERDICT                  = 383329,
    SPELL_PALADIN_FIRST_AVENGER                  = 203776,
    SPELL_PALADIN_FORBEARANCE                    = 25771,
    SPELL_PALADIN_GREATER_BLESSING_OF_KINGS      = 203538,
    SPELL_PALADIN_GUARDIAN_OF_ANCIENT_KINGS      = 86659,
    SPELL_PALADIN_HAMMER_OF_JUSTICE              = 853,
    SPELL_PALADIN_HAMMER_OF_THE_RIGHTEOUS_AOE    = 88263,
    SPELL_PALADIN_HAND_OF_SACRIFICE              = 6940,
    SPELL_PALADIN_HAND_OF_THE_PROTECTOR          = 213652,
    SPELL_PALADIN_HOLY_MENDING                   = 64891,
    SPELL_PALADIN_HOLY_POWER_ARMOR               = 28790,
    SPELL_PALADIN_HOLY_POWER_ATTACK_POWER        = 28791,
    SPELL_PALADIN_HOLY_POWER_SPELL_POWER         = 28793,
    SPELL_PALADIN_HOLY_POWER_MP5                 = 28795,
    SPELL_PALADIN_HOLY_PRISM_AREA_BEAM_VISUAL    = 121551,
    SPELL_PALADIN_HOLY_PRISM_TARGET_ALLY         = 114871,
    SPELL_PALADIN_HOLY_PRISM_TARGET_ENEMY        = 114852,
    SPELL_PALADIN_HOLY_PRISM_TARGET_BEAM_VISUAL  = 114862,
    SPELL_PALADIN_HOLY_SHOCK                     = 20473,
    SPELL_PALADIN_HOLY_SHOCK_DAMAGE              = 25912,
    SPELL_PALADIN_HOLY_SHOCK_HEALING             = 25914,
    SPELL_PALADIN_HOLY_LIGHT                     = 82326,
    SPELL_PALADIN_INFUSION_OF_LIGHT_AURA         = 54149,
    SPELL_PALADIN_INFUSION_OF_LIGHT_ENERGIZE     = 356717,
    SPELL_PALADIN_IMMUNE_SHIELD_MARKER           = 61988, // Serverside
    SPELL_PALADIN_ITEM_HEALING_TRANCE            = 37706,
    SPELL_PALADIN_JUDGMENT                       = 20271,
    SPELL_PALADIN_JUDGMENT_GAIN_HOLY_POWER       = 220637,
    SPELL_PALADIN_JUDGMENT_HOLY_R3               = 231644,
    SPELL_PALADIN_JUDGMENT_HOLY_R3_DEBUFF        = 214222,
    SPELL_PALADIN_JUDGMENT_OF_LIGHT_HEAL         = 183811,
    SPELL_PALADIN_JUDGMENT_PROT_RET_R3           = 231663, // real id "Judgment, Rank 3" (315867 is a later-expansion remake id)
    SPELL_PALADIN_LIGHT_OF_DAWN                  = 85222,
    SPELL_PALADIN_LIGHT_OF_DAWN_TRIGGER          = 185984,
    SPELL_PALADIN_LIGHT_HAMMER_COSMETIC          = 122257,
    SPELL_PALADIN_LIGHT_OF_THE_MARTYR_DAMAGE     = 196917,
    SPELL_PALADIN_LIGHT_OF_THE_PROTECTOR         = 184092,
    SPELL_PALADIN_LIGHT_HAMMER_DAMAGE            = 114919,
    SPELL_PALADIN_LIGHT_HAMMER_HEALING           = 119952,
    SPELL_PALADIN_LIGHT_HAMMER_PERIODIC          = 114918,
    SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT        = 31790,
    SPELL_PALADIN_RIGHTEOUS_VERDICT_AURA         = 267611,
    SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS          = 25742,
    SPELL_PALADIN_SERAPHIM                       = 152262,
    SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS        = 53600,
    SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS_ARMOR  = 132403,
    SPELL_PALADIN_SHIELD_OF_VENGEANCE_DAMAGE     = 184689,
    SPELL_PALADIN_TEMPLAR_VERDICT_DAMAGE         = 224266,
    SPELL_PALADIN_WORD_OF_GLORY                  = 210191,
    SPELL_PALADIN_ZEAL_AURA                      = 269571
};

enum PaladinSpellVisualKit
{
    PALADIN_VISUAL_KIT_DIVINE_STORM              = 73892
};

enum PaladinSpellVisual
{
    PALADIN_VISUAL_SPELL_HOLY_SHOCK_DAMAGE       = 83731,
    PALADIN_VISUAL_SPELL_HOLY_SHOCK_DAMAGE_CRIT  = 83881,
    PALADIN_VISUAL_SPELL_HOLY_SHOCK_HEAL         = 83732,
    PALADIN_VISUAL_SPELL_HOLY_SHOCK_HEAL_CRIT    = 83880
};

// 31850 - Ardent Defender
class spell_pal_ardent_defender : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_ARDENT_DEFENDER_HEAL })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void HandleAbsorb(AuraEffect const* aurEff, DamageInfo const& dmgInfo, uint32& absorbAmount)
    {
        PreventDefaultAction();

        int32 targetHealthPercent = GetEffectInfo(EFFECT_1).CalcValue(GetTarget());
        uint64 targetHealth = int32(GetTarget()->CountPctFromMaxHealth(targetHealthPercent));
        if (GetTarget()->HealthBelowPct(targetHealthPercent))
        {
            // we are currently below desired health
            // absorb everything and heal up
            absorbAmount = dmgInfo.GetDamage();

            GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_ARDENT_DEFENDER_HEAL,
                CastSpellExtraArgs(aurEff)
                .AddSpellMod(SPELLVALUE_BASE_POINT0, int32(targetHealth - GetTarget()->GetHealth())));
        }
        else
        {
            // we are currently above desired health
            // just absorb enough to reach that percentage
            absorbAmount = dmgInfo.GetDamage() - int32(GetTarget()->GetHealth() - targetHealth);
        }

        Remove();
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_ardent_defender::HandleAbsorb, EFFECT_0);
    }
};

// 267344 - Art of War
class spell_pal_art_of_war : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_ART_OF_WAR_TRIGGERED, SPELL_PALADIN_BLADE_OF_JUSTICE });
    }

    bool CheckProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        return roll_chance_i(aurEff->GetAmount());
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        GetTarget()->GetSpellHistory()->ResetCooldown(SPELL_PALADIN_BLADE_OF_JUSTICE, true);
        GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_ART_OF_WAR_TRIGGERED, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_art_of_war::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pal_art_of_war::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 31935 - Avenger's Shield
// First Avenger (203776) boosts the damage dealt to the primary (selected) target.
class spell_pal_avengers_shield : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_FIRST_AVENGER });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        if (!caster)
            return;

        AuraEffect const* firstAvenger = caster->GetAuraEffect(SPELL_PALADIN_FIRST_AVENGER, EFFECT_0);
        if (!firstAvenger || caster->GetSelectedUnit() != GetHitUnit())
            return;

        int32 damage = GetHitDamage();
        AddPct(damage, firstAvenger->GetAmount());
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_avengers_shield::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 248033 - Awakening
class spell_pal_awakening : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_AVENGING_WRATH })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    bool CheckProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        return roll_chance_i(aurEff->GetAmount());
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Milliseconds extraDuration = 0ms;
        if (AuraEffect const* durationEffect = GetEffect(EFFECT_1))
            extraDuration = Seconds(durationEffect->GetAmount());

        if (Aura* avengingWrath = GetTarget()->GetAura(SPELL_PALADIN_AVENGING_WRATH))
        {
            avengingWrath->SetDuration(avengingWrath->GetDuration() + extraDuration.count());
            avengingWrath->SetMaxDuration(avengingWrath->GetMaxDuration() + extraDuration.count());
        }
        else
            GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_AVENGING_WRATH,
                CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD)
                    .SetTriggeringSpell(eventInfo.GetProcSpell())
                    .AddSpellMod(SPELLVALUE_DURATION, extraDuration.count()));
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_awakening::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pal_awakening::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 1022 - Blessing of Protection
// 204018 - Blessing of Spellwarding
class spell_pal_blessing_of_protection : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo(
        {
            SPELL_PALADIN_FORBEARANCE,
            SPELL_PALADIN_IMMUNE_SHIELD_MARKER
        }) && spellInfo->ExcludeTargetAuraSpell == SPELL_PALADIN_IMMUNE_SHIELD_MARKER;
    }

    void TriggerForbearance() const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_FORBEARANCE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_pal_blessing_of_protection::TriggerForbearance);
    }
};

// 115750 - Blinding Light
class spell_pal_blinding_light : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_BLINDING_LIGHT_EFFECT });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_BLINDING_LIGHT_EFFECT, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_blinding_light::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 26573 - Consecration
class spell_pal_consecration : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PALADIN_CONSECRATION_DAMAGE,
            // validate for areatrigger_pal_consecration
            SPELL_PALADIN_CONSECRATION_PROTECTION_AURA,
            SPELL_PALADIN_CONSECRATED_GROUND_PASSIVE,
            SPELL_PALADIN_CONSECRATED_GROUND_SLOW
        });
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        if (AreaTrigger* at = GetTarget()->GetAreaTrigger(SPELL_PALADIN_CONSECRATION))
            GetTarget()->CastSpell(at->GetPosition(), SPELL_PALADIN_CONSECRATION_DAMAGE);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_consecration::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 26573 - Consecration
//  9228 - AreaTriggerId
struct areatrigger_pal_consecration : AreaTriggerAI
{
    areatrigger_pal_consecration(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            // 243597 is also being cast as protection, but CreateObject is not sent, either serverside areatrigger for this aura or unused - also no visual is seen
            if (unit == caster && caster->IsPlayer() && caster->ToPlayer()->GetPrimarySpecialization() == ChrSpecialization::PaladinProtection)
                caster->CastSpell(caster, SPELL_PALADIN_CONSECRATION_PROTECTION_AURA);

            if (caster->IsValidAttackTarget(unit))
                if (caster->HasAura(SPELL_PALADIN_CONSECRATED_GROUND_PASSIVE))
                    caster->CastSpell(unit, SPELL_PALADIN_CONSECRATED_GROUND_SLOW);
        }
    }

    void OnUnitExit(Unit* unit) override
    {
        if (at->GetCasterGuid() == unit->GetGUID())
            unit->RemoveAurasDueToSpell(SPELL_PALADIN_CONSECRATION_PROTECTION_AURA, at->GetCasterGuid());

        unit->RemoveAurasDueToSpell(SPELL_PALADIN_CONSECRATED_GROUND_SLOW, at->GetCasterGuid());
    }
};

// 231895 - Crusade
// Retribution's Avenging Wrath replacement: starts at a small damage/haste bonus and grows by
// 1 stack per Holy Power spent while active. DB2 stores the per-effect amount at 10x scale so it
// can be stacked in fine increments; both modified effects are rescaled down to their real value.
class spell_pal_crusade : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount /= 10;
    }

    void HandleEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Spell const* procSpell = eventInfo.GetProcSpell();
        if (!procSpell)
            return;

        if (Optional<int32> holyPowerCost = procSpell->GetPowerTypeCostAmount(POWER_HOLY_POWER))
            GetAura()->ModStackAmount(*holyPowerCost, AURA_REMOVE_BY_DEFAULT, false);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_crusade::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_crusade::CalculateAmount, EFFECT_2, SPELL_AURA_MELEE_SLOW);
        OnEffectProc += AuraEffectProcFn(spell_pal_crusade::HandleEffectProc, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    }
};

// 196926 - Crusader Might
class spell_pal_crusader_might : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_HOLY_SHOCK });
    }

    void HandleEffectProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_PALADIN_HOLY_SHOCK, Milliseconds(aurEff->GetAmount()));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_crusader_might::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 209785 - The Fires of Justice
// Gates its own (DB2-driven) proc-chance/cost-reduction effect to only trigger off Crusader
// Strike, matching this file's established DB2-carries-the-numeric-effect pattern (see
// spell_pal_judgement_of_the_pure, spell_pal_judgment_of_light below). The Crusader Strike
// cooldown-reduction half of this talent is presumed to be a flat DB2 spell-mod on the
// talent's own passive and needs no script.
class spell_pal_the_fires_of_justice : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_CRUSADER_STRIKE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_PALADIN_CRUSADER_STRIKE;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_the_fires_of_justice::CheckProc);
    }
};

// 223817 - Divine Purpose
class spell_pal_divine_purpose : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_DIVINE_PURPOSE_TRIGGERED });
    }

    bool CheckProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Spell const* procSpell = eventInfo.GetProcSpell();
        if (!procSpell)
            return false;

        if (!procSpell->HasPowerTypeCost(POWER_HOLY_POWER))
            return false;

        return roll_chance_i(aurEff->GetAmount());
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_PALADIN_DIVINE_PURPOSE_TRIGGERED,
            CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetTriggeringSpell(eventInfo.GetProcSpell()));
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_divine_purpose::CheckProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pal_divine_purpose::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 216860 - Judgement of the Pure
// Gates its own (DB2-driven) effect to only trigger off Judgment.
class spell_pal_judgement_of_the_pure : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_PALADIN_JUDGMENT;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_judgement_of_the_pure::CheckProc);
    }
};

// 152261 - Holy Shield
class spell_pal_holy_shield : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return (eventInfo.GetHitMask() & PROC_HIT_BLOCK) != 0;
    }

    // The spell's own DB2 data carries an unwanted absorb component on this effect - disable it.
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = 0;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_holy_shield::CheckProc);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_holy_shield::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 204035 - Bastion of Light
class spell_pal_bastion_of_light : public SpellScript
{
    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            caster->GetSpellHistory()->ResetCharges(sSpellMgr->AssertSpellInfo(SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS, DIFFICULTY_NONE)->ChargeCategoryId);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_bastion_of_light::HandleOnHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 213313 - Divine Intervention
// Auto-triggers Divine Shield plus a heal when about to take fatal damage, gated by Forbearance
// and Divine Shield's own cooldown so it can't be exploited to bypass either.
class spell_pal_divine_intervention : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_DIVINE_INTERVENTION_HEAL, SPELL_PALADIN_FORBEARANCE, SPELL_PALADIN_DIVINE_SHIELD })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
    {
        Unit* target = GetTarget();
        if (dmgInfo.GetDamage() < target->GetHealth())
            return;

        if (target->HasAura(SPELL_PALADIN_FORBEARANCE) || target->GetSpellHistory()->HasCooldown(SPELL_PALADIN_DIVINE_SHIELD))
            return;

        int32 healAmount = int32(target->CountPctFromMaxHealth(GetEffectInfo(EFFECT_1).CalcValue()));

        target->CastSpell(target, SPELL_PALADIN_DIVINE_SHIELD, true);
        target->CastSpell(target, SPELL_PALADIN_DIVINE_INTERVENTION_HEAL, CastSpellExtraArgs(aurEff).AddSpellMod(SPELLVALUE_BASE_POINT0, healAmount));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_divine_intervention::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_divine_intervention::Absorb, EFFECT_0);
    }
};

// 231832 - Blade of Wrath (proc)
class spell_pal_blade_of_wrath_proc : public AuraScript
{
    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        if (Unit* caster = GetCaster())
            caster->GetSpellHistory()->ResetCooldown(SPELL_PALADIN_BLADE_OF_JUSTICE, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_blade_of_wrath_proc::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 152262 - Seraphim
// Consumes all available Shield of the Righteous charges to extend Seraphim's own buff duration
// by one full duration per charge spent.
class spell_pal_seraphim : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_SERAPHIM, SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS });
    }

    SpellCastResult CheckCast()
    {
        uint32 chargeCategoryId = sSpellMgr->AssertSpellInfo(SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS, DIFFICULTY_NONE)->ChargeCategoryId;
        if (!GetCaster()->GetSpellHistory()->HasCharge(chargeCategoryId))
            return SPELL_FAILED_NO_POWER;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        uint32 chargeCategoryId = sSpellMgr->AssertSpellInfo(SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS, DIFFICULTY_NONE)->ChargeCategoryId;
        SpellHistory* spellHistory = GetCaster()->GetSpellHistory();

        int32 maxCharges = int32(GetEffectInfo(effIndex).CalcValue());
        int32 usedCharges = 0;
        while (usedCharges < maxCharges && spellHistory->HasCharge(chargeCategoryId))
        {
            spellHistory->ConsumeCharge(chargeCategoryId);
            ++usedCharges;
        }

        if (Aura* seraphimAura = GetCaster()->GetAura(SPELL_PALADIN_SERAPHIM))
            seraphimAura->SetDuration(GetSpellInfo()->GetMaxDuration() * usedCharges);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pal_seraphim::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_pal_seraphim::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// 203538 - Greater Blessing of Kings
// A periodically-refilling absorb shield, capped at 2.7x the caster's spell power.
class spell_pal_greater_blessing_of_kings : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_GREATER_BLESSING_OF_KINGS });
    }

    bool Load() override
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return false;

        maxAbsorbAmount = uint32(2.7f * caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL));
        leftAbsorbAmount = maxAbsorbAmount;
        return true;
    }

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        leftAbsorbAmount = maxAbsorbAmount;
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        absorbAmount = std::min(dmgInfo.GetDamage(), leftAbsorbAmount);
        leftAbsorbAmount -= absorbAmount;
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_greater_blessing_of_kings::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_greater_blessing_of_kings::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_greater_blessing_of_kings::Absorb, EFFECT_1);
    }

private:
    uint32 leftAbsorbAmount = 0;
    uint32 maxAbsorbAmount = 0;
};

// 183998 - Light of the Martyr
// Heals the target, then deals self-damage equal to 50% of the healing done.
class spell_pal_light_of_the_martyr : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_LIGHT_OF_THE_MARTYR_DAMAGE });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 dmg = int32(GetHitHeal() / 2);
        caster->CastSpell(caster, SPELL_PALADIN_LIGHT_OF_THE_MARTYR_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, dmg));

        if (caster->HasAura(SPELL_PALADIN_FERVENT_MARTYR_BUFF))
            caster->RemoveAurasDueToSpell(SPELL_PALADIN_FERVENT_MARTYR_BUFF);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_light_of_the_martyr::HandleOnHit, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// 196923 - Fervent Martyr
// Grants the stacking buff (223316) that spell_pal_light_of_the_martyr above already consumes -
// this file previously had the consuming half but never the granting half. Gates its own
// (DB2-driven, stacking) buff application to only trigger off Holy Light/Flash of Light casts,
// matching this file's established DB2-carries-the-numeric-effect pattern.
class spell_pal_fervent_martyr : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_HOLY_LIGHT, SPELL_PALADIN_FLASH_OF_LIGHT, SPELL_PALADIN_FERVENT_MARTYR_BUFF });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && (spellInfo->Id == SPELL_PALADIN_HOLY_LIGHT || spellInfo->Id == SPELL_PALADIN_FLASH_OF_LIGHT);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_PALADIN_FERVENT_MARTYR_BUFF, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_fervent_martyr::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pal_fervent_martyr::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 184092 - Light of the Protector, 213652 - Hand of the Protector
// Self-heal scaling with the caster's own missing health, read from each spell's own EFFECT_0.
class spell_pal_light_of_the_protector : public SpellScript
{
    void HandleLaunchTarget(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();

        int32 healthMissing = int32(caster->GetMaxHealth() - caster->GetHealth());
        if (healthMissing > 0)
            SetEffectValue(CalculatePct(healthMissing, GetEffectInfo(effIndex).CalcValue(caster)));
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_pal_light_of_the_protector::HandleLaunchTarget, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// 85222 - Light of Dawn
class spell_pal_light_of_dawn : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_LIGHT_OF_DAWN_TRIGGER });
    }

    void HandleOnCast()
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_PALADIN_LIGHT_OF_DAWN_TRIGGER, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_pal_light_of_dawn::HandleOnCast);
    }
};

// 185984 - Light of Dawn (AOE heal trigger)
// Cone target selection (15 yards, 90 degrees) capped to 5 targets, sorted by lowest health
// percentage first - matching this file's health-pct-sort idiom used elsewhere for similar
// Holy Power heals (e.g. spell_pal_holy_prism_selector) rather than the older random-pick
// selection. The heal amount itself is read from this spell's own DB2 effect data, not a
// hardcoded attack-power multiplier.
class spell_pal_light_of_dawn_trigger : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();

        targets.remove_if([caster](WorldObject* target)
        {
            return !(caster->IsWithinDist2d(target, 15.0f) && caster->isInFront(target, float(M_PI / 3)));
        });

        uint32 const maxTargets = 5;
        if (targets.size() > maxTargets)
        {
            targets.sort(Trinity::Predicates::HealthPctOrderPred());
            targets.resize(maxTargets);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_light_of_dawn_trigger::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 198034 - Divine Hammer
class spell_pal_divine_hammer : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_DIVINE_HAMMER });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            caster->ModifyPower(POWER_HOLY_POWER, 2);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_divine_hammer::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// 642 - Divine Shield
class spell_pal_divine_shield : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo(
        {
            SPELL_PALADIN_FINAL_STAND,
            SPELL_PALADIN_FINAL_STAND_EFFECT,
            SPELL_PALADIN_FORBEARANCE,
            SPELL_PALADIN_IMMUNE_SHIELD_MARKER
        }) && spellInfo->ExcludeCasterAuraSpell == SPELL_PALADIN_IMMUNE_SHIELD_MARKER;
    }

    void HandleFinalStand()
    {
        if (GetCaster()->HasAura(SPELL_PALADIN_FINAL_STAND))
            GetCaster()->CastSpell(nullptr, SPELL_PALADIN_FINAL_STAND_EFFECT, true);
    }

    void TriggerForbearance() const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_FORBEARANCE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_pal_divine_shield::HandleFinalStand);
        AfterHit += SpellHitFn(spell_pal_divine_shield::TriggerForbearance);
    }
};

// 190784 - Divine Steed
class spell_pal_divine_steed : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_PALADIN_DIVINE_STEED_HUMAN,
            SPELL_PALADIN_DIVINE_STEED_DWARF,
            SPELL_PALADIN_DIVINE_STEED_DRAENEI,
            SPELL_PALADIN_DIVINE_STEED_BLOODELF,
            SPELL_PALADIN_DIVINE_STEED_TAUREN
        });
    }

    void HandleOnCast()
    {
        Unit* caster = GetCaster();

        uint32 spellId = SPELL_PALADIN_DIVINE_STEED_HUMAN;
        switch (caster->GetRace())
        {
            case RACE_HUMAN:
                spellId = SPELL_PALADIN_DIVINE_STEED_HUMAN;
                break;
            case RACE_DWARF:
                spellId = SPELL_PALADIN_DIVINE_STEED_DWARF;
                break;
            case RACE_DRAENEI:
            case RACE_LIGHTFORGED_DRAENEI:
                spellId = SPELL_PALADIN_DIVINE_STEED_DRAENEI;
                break;
            case RACE_BLOODELF:
                spellId = SPELL_PALADIN_DIVINE_STEED_BLOODELF;
                break;
            case RACE_TAUREN:
                spellId = SPELL_PALADIN_DIVINE_STEED_TAUREN;
                break;
            default:
                break;
        }

        caster->CastSpell(caster, spellId, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_pal_divine_steed::HandleOnCast);
    }
};

// 53385 - Divine Storm
class spell_pal_divine_storm : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return sSpellVisualKitStore.HasRecord(PALADIN_VISUAL_KIT_DIVINE_STORM);
    }

    void HandleOnCast()
    {
        GetCaster()->SendPlaySpellVisualKit(PALADIN_VISUAL_KIT_DIVINE_STORM, 0, 0);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_pal_divine_storm::HandleOnCast);
    }
};

// 205191 - Eye for an Eye
class spell_pal_eye_for_an_eye : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_EYE_FOR_AN_EYE_TRIGGERED });
    }

    void HandleEffectProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        GetTarget()->CastSpell(eventInfo.GetActor(), SPELL_PALADIN_EYE_FOR_AN_EYE_TRIGGERED, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_eye_for_an_eye::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 383328 - Final Verdict
class spell_pal_final_verdict : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_FINAL_VERDICT });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        if (!roll_chance_i(GetEffectValue()))
            return;

        Unit* caster = GetCaster();
        caster->CastSpell(caster, SPELL_PALADIN_FINAL_VERDICT, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_final_verdict::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// 234299 - Fist of Justice
class spell_pal_fist_of_justice : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_HAMMER_OF_JUSTICE });
    }

    bool CheckEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Spell const* procSpell = eventInfo.GetProcSpell())
            return procSpell->HasPowerTypeCost(POWER_HOLY_POWER);

        return false;
    }

    void HandleEffectProc(AuraEffect* aurEff, ProcEventInfo& /*procInfo*/)
    {
        int32 value = aurEff->GetAmount() / 10;

        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_PALADIN_HAMMER_OF_JUSTICE, Seconds(-value));
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_fist_of_justice::CheckEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pal_fist_of_justice::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// -85043 - Grand Crusader
class spell_pal_grand_crusader : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_AVENGERS_SHIELD });
    }

    bool CheckProc(ProcEventInfo& /*eventInfo*/)
    {
        return GetTarget()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleEffectProc(AuraEffect* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        GetTarget()->GetSpellHistory()->ResetCooldown(SPELL_PALADIN_AVENGERS_SHIELD, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_grand_crusader::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pal_grand_crusader::HandleEffectProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 54968 - Glyph of Holy Light
class spell_pal_glyph_of_holy_light : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        uint32 const maxTargets = GetSpellInfo()->MaxAffectedTargets;

        if (targets.size() > maxTargets)
        {
            targets.sort(Trinity::Predicates::HealthPctOrderPred());
            targets.resize(maxTargets);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_glyph_of_holy_light::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 53595 - Hammer of the Righteous
struct spell_pal_hammer_of_the_righteous : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PALADIN_CONSECRATION_PROTECTION_AURA,
            SPELL_PALADIN_HAMMER_OF_THE_RIGHTEOUS_AOE
        });
    }

    void HandleAoEHit(SpellEffIndex /*effIndex*/)
    {
        if (GetCaster()->HasAura(SPELL_PALADIN_CONSECRATION_PROTECTION_AURA))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_HAMMER_OF_THE_RIGHTEOUS_AOE);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_hammer_of_the_righteous::HandleAoEHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 6940 - Hand of Sacrifice
class spell_pal_hand_of_sacrifice : public AuraScript
{
public:
    spell_pal_hand_of_sacrifice()
    {
        remainingAmount = 0;
    }

private:
    int32 remainingAmount;

    bool Load() override
    {
        if (Unit* caster = GetCaster())
        {
            remainingAmount = caster->GetMaxHealth();
            return true;
        }
        return false;
    }

    void Split(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, uint32 & splitAmount)
    {
        remainingAmount -= splitAmount;

        if (remainingAmount <= 0)
        {
            GetTarget()->RemoveAura(SPELL_PALADIN_HAND_OF_SACRIFICE);
        }
    }

    void Register() override
    {
        OnEffectSplit += AuraEffectSplitFn(spell_pal_hand_of_sacrifice::Split, EFFECT_0);
    }
};

// 54149 - Infusion of Light
class spell_pal_infusion_of_light : public AuraScript
{
    static constexpr flag128 HolyLightSpellClassMask = { 0, 0, 0x400 };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_INFUSION_OF_LIGHT_ENERGIZE });
    }

    bool CheckFlashOfLightProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcSpell() && eventInfo.GetProcSpell()->m_appliedMods.find(GetAura()) != eventInfo.GetProcSpell()->m_appliedMods.end();
    }

    bool CheckHolyLightProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->IsAffected(SPELLFAMILY_PALADIN, HolyLightSpellClassMask);
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_PALADIN_INFUSION_OF_LIGHT_ENERGIZE,
            CastSpellExtraArgs(TRIGGERED_FULL_MASK).SetTriggeringSpell(eventInfo.GetProcSpell()));
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_infusion_of_light::CheckFlashOfLightProc, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER);
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_infusion_of_light::CheckFlashOfLightProc, EFFECT_2, SPELL_AURA_ADD_FLAT_MODIFIER);

        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_infusion_of_light::CheckHolyLightProc, EFFECT_1, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pal_infusion_of_light::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 327193 - Moment of Glory
class spell_pal_moment_of_glory : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_AVENGERS_SHIELD });
    }

    void HandleOnHit()
    {
        GetCaster()->GetSpellHistory()->ResetCooldown(SPELL_PALADIN_AVENGERS_SHIELD);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_pal_moment_of_glory::HandleOnHit);
    }
};

// 20271/275779/275773 - Judgement (Retribution/Protection/Holy)
class spell_pal_judgment : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PALADIN_JUDGMENT_PROT_RET_R3,
            SPELL_PALADIN_JUDGMENT_GAIN_HOLY_POWER,
            SPELL_PALADIN_JUDGMENT_HOLY_R3,
            SPELL_PALADIN_JUDGMENT_HOLY_R3_DEBUFF
        });
    }

    void HandleOnHit()
    {
        Unit* caster = GetCaster();

        if (caster->HasSpell(SPELL_PALADIN_JUDGMENT_PROT_RET_R3))
            caster->CastSpell(caster, SPELL_PALADIN_JUDGMENT_GAIN_HOLY_POWER, GetSpell());

        if (caster->HasSpell(SPELL_PALADIN_JUDGMENT_HOLY_R3))
            caster->CastSpell(GetHitUnit(), SPELL_PALADIN_JUDGMENT_HOLY_R3_DEBUFF, GetSpell());
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_pal_judgment::HandleOnHit);
    }
};

// 183778 - Judgment of Light
// Gates its own (DB2-driven) triggered effect to only apply the 196941 debuff off Judgment casts.
class spell_pal_judgment_of_light : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_PALADIN_JUDGMENT;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_judgment_of_light::CheckProc);
    }
};

// 196941 - Judgment of Light (debuff applied to the judged target)
// Heals the paladin whenever they personally land damage on a target carrying this debuff.
class spell_pal_judgment_of_light_proc : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_JUDGMENT_OF_LIGHT_HEAL });
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* attacker = eventInfo.GetActor();
        if (!caster || !attacker || attacker->GetGUID() != caster->GetGUID())
            return;

        caster->CastSpell(attacker, SPELL_PALADIN_JUDGMENT_OF_LIGHT_HEAL, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_judgment_of_light_proc::HandleEffectProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 215661 - Justicar's Vengeance
class spell_pal_justicars_vengeance : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void HandleDamage(SpellEffectInfo const& /*spellEffectInfo*/, Unit const* victim, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        if (victim->HasUnitState(UNIT_STATE_STUNNED))
            AddPct(pctMod, GetEffectInfo(EFFECT_1).CalcValue(GetCaster()));
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_pal_justicars_vengeance::HandleDamage);
    }
};

// 114165 - Holy Prism
class spell_pal_holy_prism : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_PALADIN_HOLY_PRISM_TARGET_ALLY, SPELL_PALADIN_HOLY_PRISM_TARGET_ENEMY, SPELL_PALADIN_HOLY_PRISM_TARGET_BEAM_VISUAL });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (GetCaster()->IsFriendlyTo(GetHitUnit()))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_HOLY_PRISM_TARGET_ALLY, true);
        else
            GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_HOLY_PRISM_TARGET_ENEMY , true);

        GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_HOLY_PRISM_TARGET_BEAM_VISUAL, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_holy_prism::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 114852 - Holy Prism (Damage)
// 114871 - Holy Prism (Heal)
class spell_pal_holy_prism_selector : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo ({ SPELL_PALADIN_HOLY_PRISM_TARGET_ALLY, SPELL_PALADIN_HOLY_PRISM_AREA_BEAM_VISUAL });
    }

    void SaveTargetGuid(SpellEffIndex /*effIndex*/)
    {
        _targetGUID = GetHitUnit()->GetGUID();
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        uint8 const maxTargets = 5;

        if (targets.size() > maxTargets)
        {
            if (GetSpellInfo()->Id == SPELL_PALADIN_HOLY_PRISM_TARGET_ALLY)
            {
                targets.sort(Trinity::Predicates::HealthPctOrderPred());
                targets.resize(maxTargets);
            }
            else
                Trinity::Containers::RandomResize(targets, maxTargets);
        }

        _sharedTargets = targets;
    }

    void ShareTargets(std::list<WorldObject*>& targets)
    {
        targets = _sharedTargets;
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* initialTarget = ObjectAccessor::GetUnit(*GetCaster(), _targetGUID))
            initialTarget->CastSpell(GetHitUnit(), SPELL_PALADIN_HOLY_PRISM_AREA_BEAM_VISUAL, true);
    }

    void Register() override
    {
        if (m_scriptSpellId == SPELL_PALADIN_HOLY_PRISM_TARGET_ENEMY)
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_prism_selector::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
        else if (m_scriptSpellId == SPELL_PALADIN_HOLY_PRISM_TARGET_ALLY)
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_prism_selector::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);

        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_prism_selector::ShareTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENTRY);

        OnEffectHitTarget += SpellEffectFn(spell_pal_holy_prism_selector::SaveTargetGuid, EFFECT_0, SPELL_EFFECT_ANY);
        OnEffectHitTarget += SpellEffectFn(spell_pal_holy_prism_selector::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
    }

private:
    std::list<WorldObject*> _sharedTargets;
    ObjectGuid _targetGUID;
};

// 20473 - Holy Shock
class spell_pal_holy_shock : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PALADIN_HOLY_SHOCK,
            SPELL_PALADIN_HOLY_SHOCK_HEALING,
            SPELL_PALADIN_HOLY_SHOCK_DAMAGE
        });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();

        if (Unit* target = GetExplTargetUnit())
        {
            if (!caster->IsFriendlyTo(target))
            {
                if (!caster->IsValidAttackTarget(target))
                    return SPELL_FAILED_BAD_TARGETS;

                if (!caster->isInFront(target))
                    return SPELL_FAILED_UNIT_NOT_INFRONT;
            }
        }
        else
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();

        if (Unit* unitTarget = GetHitUnit())
        {
            if (caster->IsFriendlyTo(unitTarget))
                caster->CastSpell(unitTarget, SPELL_PALADIN_HOLY_SHOCK_HEALING, GetSpell());
            else
                caster->CastSpell(unitTarget, SPELL_PALADIN_HOLY_SHOCK_DAMAGE, GetSpell());
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pal_holy_shock::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_pal_holy_shock::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 25912 - Holy Shock
class spell_pal_holy_shock_damage_visual : public SpellScript
{
    bool Validate(SpellInfo const*) override
    {
        return sSpellVisualStore.HasRecord(PALADIN_VISUAL_SPELL_HOLY_SHOCK_DAMAGE)
            && sSpellVisualStore.HasRecord(PALADIN_VISUAL_SPELL_HOLY_SHOCK_DAMAGE_CRIT);
    }

    void PlayVisual()
    {
        GetCaster()->SendPlaySpellVisual(GetHitUnit(), IsHitCrit() ? PALADIN_VISUAL_SPELL_HOLY_SHOCK_DAMAGE_CRIT : PALADIN_VISUAL_SPELL_HOLY_SHOCK_DAMAGE, 0, 0, 0.0f, false);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_pal_holy_shock_damage_visual::PlayVisual);
    }
};

// 25914 - Holy Shock
class spell_pal_holy_shock_heal_visual : public SpellScript
{
    bool Validate(SpellInfo const*) override
    {
        return sSpellVisualStore.HasRecord(PALADIN_VISUAL_SPELL_HOLY_SHOCK_HEAL)
            && sSpellVisualStore.HasRecord(PALADIN_VISUAL_SPELL_HOLY_SHOCK_HEAL_CRIT);
    }

    void PlayVisual()
    {
        GetCaster()->SendPlaySpellVisual(GetHitUnit(), IsHitCrit() ? PALADIN_VISUAL_SPELL_HOLY_SHOCK_HEAL_CRIT : PALADIN_VISUAL_SPELL_HOLY_SHOCK_HEAL, 0, 0, 0.0f, false);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_pal_holy_shock_heal_visual::PlayVisual);
    }
};

// 210220 - Holy Wrath
// Damage scales with the caster's own missing health; the spell's own DB2 data carries a
// separate (lower) multiplier for player targets at EFFECT_3 vs. non-players at EFFECT_2.
class spell_pal_holy_wrath : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 }, { spellInfo->Id, EFFECT_3 } });
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!target)
            return;

        int32 healthMissing = int32(caster->GetMaxHealth() - caster->GetHealth());
        if (healthMissing <= 0)
            return;

        SpellEffIndex pctEffect = target->GetTypeId() == TYPEID_PLAYER ? EFFECT_3 : EFFECT_2;
        SetHitDamage(CalculatePct(healthMissing, GetEffectInfo(pctEffect).CalcValue(caster)));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_holy_wrath::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 37705 - Healing Discount
class spell_pal_item_healing_discount : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_ITEM_HEALING_TRANCE });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_ITEM_HEALING_TRANCE, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_item_healing_discount::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 40470 - Paladin Tier 6 Trinket
class spell_pal_item_t6_trinket : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_PALADIN_ENDURING_LIGHT,
            SPELL_PALADIN_ENDURING_JUDGEMENT
        });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return;

        uint32 spellId;
        int32 chance;

        // Holy Light & Flash of Light
        if (spellInfo->SpellFamilyFlags[0] & 0xC0000000)
        {
            spellId = SPELL_PALADIN_ENDURING_LIGHT;
            chance = 15;
        }
        // Judgements
        else if (spellInfo->SpellFamilyFlags[0] & 0x00800000)
        {
            spellId = SPELL_PALADIN_ENDURING_JUDGEMENT;
            chance = 50;
        }
        else
            return;

        if (roll_chance_i(chance))
            eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), spellId, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_item_t6_trinket::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 633 - Lay on Hands
// 471195 - Lay on Hands (from 387791 - Empyreal Ward)
class spell_pal_lay_on_hands : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_FORBEARANCE })
            && spellInfo->ExcludeTargetAuraSpell == SPELL_PALADIN_IMMUNE_SHIELD_MARKER;
    }

    void TriggerForbearance() const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_FORBEARANCE, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_pal_lay_on_hands::TriggerForbearance);
    }
};

// 53651 - Light's Beacon - Beacon of Light
class spell_pal_light_s_beacon : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_BEACON_OF_LIGHT, SPELL_PALADIN_BEACON_OF_LIGHT_HEAL });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (!eventInfo.GetActionTarget())
            return false;
        if (eventInfo.GetActionTarget()->HasAura(SPELL_PALADIN_BEACON_OF_LIGHT, eventInfo.GetActor()->GetGUID()))
            return false;
        return true;
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        uint32 heal = CalculatePct(healInfo->GetHeal(), aurEff->GetAmount());

        Unit::AuraList const& auras = GetCaster()->GetSingleCastAuras();
        for (Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        {
            if ((*itr)->GetId() == SPELL_PALADIN_BEACON_OF_LIGHT)
            {
                std::vector<AuraApplication*> applications;
                (*itr)->GetApplicationVector(applications);
                if (!applications.empty())
                {
                    CastSpellExtraArgs args(aurEff);
                    args.AddSpellMod(SPELLVALUE_BASE_POINT0, heal);
                    eventInfo.GetActor()->CastSpell(applications.front()->GetTarget(), SPELL_PALADIN_BEACON_OF_LIGHT_HEAL, args);
                }
                return;
            }
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_light_s_beacon::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pal_light_s_beacon::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 177173 - Beacon of Faith (heal-echo half)
// Same heal-echo mechanic as Light's Beacon above, but for the Beacon of Faith talent, and at half
// the healing - the trade-off for being usable alongside Beacon of Light on a second target.
class spell_pal_beacon_of_faith_proc : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_BEACON_OF_FAITH, SPELL_PALADIN_BEACON_OF_LIGHT_HEAL });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (!eventInfo.GetActionTarget())
            return false;
        if (eventInfo.GetActionTarget()->HasAura(SPELL_PALADIN_BEACON_OF_FAITH, eventInfo.GetActor()->GetGUID()))
            return false;
        return true;
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        uint32 heal = CalculatePct(healInfo->GetHeal(), aurEff->GetAmount()) / 2;

        Unit::AuraList const& auras = GetCaster()->GetSingleCastAuras();
        for (Aura* aura : auras)
        {
            if (aura->GetId() == SPELL_PALADIN_BEACON_OF_FAITH)
            {
                std::vector<AuraApplication*> applications;
                aura->GetApplicationVector(applications);
                if (!applications.empty())
                {
                    CastSpellExtraArgs args(aurEff);
                    args.AddSpellMod(SPELLVALUE_BASE_POINT0, heal);
                    eventInfo.GetActor()->CastSpell(applications.front()->GetTarget(), SPELL_PALADIN_BEACON_OF_LIGHT_HEAL, args);
                }
                return;
            }
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pal_beacon_of_faith_proc::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pal_beacon_of_faith_proc::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 53563 - Beacon of Light, 156910 - Beacon of Faith
// Prevents applying Beacon of Light and Beacon of Faith to the same target at once.
class spell_pal_beacon_of_light : public SpellScript
{
    SpellCastResult CheckCast()
    {
        Unit* target = GetExplTargetUnit();
        if (!target)
            return SPELL_FAILED_DONT_REPORT;

        if (target->HasAura(SPELL_PALADIN_BEACON_OF_FAITH) || target->HasAura(SPELL_PALADIN_BEACON_OF_LIGHT))
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pal_beacon_of_light::CheckCast);
    }
};

// 53563 - Beacon of Light, 156910 - Beacon of Faith, 200025 - Beacon of Virtue
// Applies/removes the correct heal-echo proc aura depending on which Beacon spell was cast.
class spell_pal_beacon_of_light_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_BEACON_OF_LIGHT_PROC_AURA, SPELL_PALADIN_BEACON_OF_FAITH_PROC_AURA });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        if (GetSpellInfo()->Id == SPELL_PALADIN_BEACON_OF_LIGHT || GetSpellInfo()->Id == SPELL_PALADIN_BEACON_OF_VIRTUE)
            caster->CastSpell(target, SPELL_PALADIN_BEACON_OF_LIGHT_PROC_AURA, true);
        else
            caster->CastSpell(target, SPELL_PALADIN_BEACON_OF_FAITH_PROC_AURA, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (GetSpellInfo()->Id == SPELL_PALADIN_BEACON_OF_LIGHT || GetSpellInfo()->Id == SPELL_PALADIN_BEACON_OF_VIRTUE)
            caster->RemoveAura(SPELL_PALADIN_BEACON_OF_LIGHT_PROC_AURA);
        else
            caster->RemoveAura(SPELL_PALADIN_BEACON_OF_FAITH_PROC_AURA);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_pal_beacon_of_light_aura::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_pal_beacon_of_light_aura::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 122773 - Light's Hammer
class spell_pal_light_hammer_init_summon : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PALADIN_LIGHT_HAMMER_COSMETIC,
            SPELL_PALADIN_LIGHT_HAMMER_PERIODIC
        });
    }

    void InitSummon()
    {
         for (SpellLogEffectGenericVictimParams const& summonedObject : GetSpell()->GetExecuteLogEffectTargets(SPELL_EFFECT_SUMMON, &SpellLogEffect::GenericVictimTargets))
         {
             if (Unit* hammer = ObjectAccessor::GetUnit(*GetCaster(), summonedObject.Victim))
             {
                 hammer->CastSpell(hammer, SPELL_PALADIN_LIGHT_HAMMER_COSMETIC,
                     CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetTriggeringSpell(GetSpell()));
                 hammer->CastSpell(hammer, SPELL_PALADIN_LIGHT_HAMMER_PERIODIC,
                     CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetTriggeringSpell(GetSpell()));
             }
         }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_pal_light_hammer_init_summon::InitSummon);
    }
};

// 114918 - Light's Hammer (Periodic)
class spell_pal_light_hammer_periodic : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PALADIN_LIGHT_HAMMER_HEALING,
            SPELL_PALADIN_LIGHT_HAMMER_DAMAGE
        });
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* lightHammer = GetTarget();
        if (Unit* originalCaster = lightHammer->GetOwner())
        {
            originalCaster->CastSpell(lightHammer->GetPosition(), SPELL_PALADIN_LIGHT_HAMMER_DAMAGE, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
            originalCaster->CastSpell(lightHammer->GetPosition(), SPELL_PALADIN_LIGHT_HAMMER_HEALING, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_light_hammer_periodic::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 204074 - Righteous Protector
class spell_pal_righteous_protector : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_AVENGING_WRATH, SPELL_PALADIN_GUARDIAN_OF_ANCIENT_KINGS });
    }

    bool CheckEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (SpellInfo const* procSpell = eventInfo.GetSpellInfo())
            _baseHolyPowerCost = procSpell->CalcPowerCost(POWER_HOLY_POWER, false, eventInfo.GetActor(), eventInfo.GetSchoolMask());
        else
            _baseHolyPowerCost.reset();

        return _baseHolyPowerCost.has_value();
    }

    void HandleEffectProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        int32 value = aurEff->GetAmount() * 100 * _baseHolyPowerCost->Amount;

        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_PALADIN_AVENGING_WRATH, Milliseconds(-value));
        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_PALADIN_GUARDIAN_OF_ANCIENT_KINGS, Milliseconds(-value));
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_righteous_protector::CheckEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pal_righteous_protector::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

    Optional<SpellPowerCost> _baseHolyPowerCost;
};

// 267610 - Righteous Verdict
class spell_pal_righteous_verdict : public AuraScript
{
    bool Validate(SpellInfo const* /*spellEntry*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_RIGHTEOUS_VERDICT_AURA });
    }

    void HandleEffectProc(AuraEffect* /*aurEff*/, ProcEventInfo& procInfo)
    {
        procInfo.GetActor()->CastSpell(procInfo.GetActor(), SPELL_PALADIN_RIGHTEOUS_VERDICT_AURA, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_righteous_verdict::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 85804 - Selfless Healer
class spell_pal_selfless_healer : public AuraScript
{
    bool CheckEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Spell const* procSpell = eventInfo.GetProcSpell())
            return procSpell->HasPowerTypeCost(POWER_HOLY_POWER);

        return false;
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_selfless_healer::CheckEffectProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 53600 - Shield of the Righteous
class spell_pal_shield_of_the_righteous : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS_ARMOR });
    }

    void HandleArmor()
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_PALADIN_SHIELD_OF_THE_RIGHTEOUS_ARMOR, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_pal_shield_of_the_righteous::HandleArmor);
    }
};

// 184662 - Shield of Vengeance
class spell_pal_shield_of_vengeance : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_SHIELD_OF_VENGEANCE_DAMAGE }) && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = CalculatePct(GetUnitOwner()->GetMaxHealth(), GetEffectInfo(EFFECT_1).CalcValue());
        if (Player const* player = GetUnitOwner()->ToPlayer())
            AddPct(amount, player->GetRatingBonusValue(CR_VERSATILITY_DAMAGE_DONE) + player->GetTotalAuraModifier(SPELL_AURA_MOD_VERSATILITY));

        _initialAmount = amount;
    }

    void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_SHIELD_OF_VENGEANCE_DAMAGE,
            CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, _initialAmount - aurEff->GetAmount()));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_shield_of_vengeance::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectRemove += AuraEffectApplyFn(spell_pal_shield_of_vengeance::HandleRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }

    int32 _initialAmount = 0;
};

// 183416 - Aura of Sacrifice
// Periodically re-applies the ally redirect-absorb buff (210372, below) to nearby raid
// members instead of using a persistent AreaTrigger object like the reference cores do (their
// AreaTrigger id is explicitly a made-up "custom" one, not a real Blizzard id) - avoids
// inventing a new non-Blizzard AreaTrigger id/SQL row for a real spell. Uses the engine's own
// Trinity::AnyGroupedUnitInObjectRangeCheck raid-range helper, the same shape already used
// elsewhere in this codebase for similar radius-based grouped-unit scans.
class spell_pal_aura_of_sacrifice : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_AURA_OF_SACRIFICE_ALLY });
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetTarget();

        std::list<Unit*> targets;
        Trinity::AnyGroupedUnitInObjectRangeCheck check(caster, caster, 20.0f, true);
        Trinity::UnitListSearcher<Trinity::AnyGroupedUnitInObjectRangeCheck> searcher(caster, targets, check);
        Cell::VisitAllObjects(caster, searcher, 20.0f);

        for (Unit* target : targets)
            if (target != caster)
                caster->CastSpell(target, SPELL_PALADIN_AURA_OF_SACRIFICE_ALLY, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_aura_of_sacrifice::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 210372 - Aura of Sacrifice (ally redirect-absorb buff)
// Redirects a percentage of damage taken by the ally back onto the paladin as damage, gated
// on the paladin's own health being above the talent's own threshold - both percentages read
// from Aura of Sacrifice's (183416) own DB2 effect data rather than hardcoded.
class spell_pal_aura_of_sacrifice_ally : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_AURA_OF_SACRIFICE, SPELL_PALADIN_AURA_OF_SACRIFICE_DAMAGE });
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsAlive())
        {
            absorbAmount = 0;
            return;
        }

        SpellInfo const* auraOfSacrificeInfo = sSpellMgr->AssertSpellInfo(SPELL_PALADIN_AURA_OF_SACRIFICE, GetCastDifficulty());

        if (caster->GetHealthPct() < auraOfSacrificeInfo->GetEffect(EFFECT_2).CalcValue(caster))
        {
            absorbAmount = 0;
            return;
        }

        absorbAmount = CalculatePct(dmgInfo.GetDamage(), auraOfSacrificeInfo->GetEffect(EFFECT_0).CalcValue(caster));
        GetTarget()->CastSpell(caster, SPELL_PALADIN_AURA_OF_SACRIFICE_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, int32(absorbAmount)));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_aura_of_sacrifice_ally::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_aura_of_sacrifice_ally::Absorb, EFFECT_0);
    }
};

// 469304 - Steed of Liberty
class spell_pal_steed_of_liberty : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_BLESSING_OF_FREEDOM });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/) const
    {
        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_PALADIN_BLESSING_OF_FREEDOM, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringAura = aurEff,
            .SpellValueOverrides = { { SPELLVALUE_DURATION, aurEff->GetAmount() } }
        });
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_steed_of_liberty::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 85256 - Templar's Verdict
class spell_pal_templar_s_verdict : public SpellScript
{
    bool Validate(SpellInfo const* /*spellEntry*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_TEMPLAR_VERDICT_DAMAGE });
    }

    void HandleHitTarget(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_PALADIN_TEMPLAR_VERDICT_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pal_templar_s_verdict::HandleHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 210191 - Word of Glory
// Legion 7.3.5 tooltip (per Warcraft Wiki, patch 7.0.3): "Heal yourself and up to 5 friendly
// targets within 15 yards for (900% of Spell power)" - a single multi-target heal effect, not
// a separate self-cast bonus. Re-sorts the engine's own target selection by lowest-health-first
// (matching this file's health-pct-sort idiom, e.g. spell_pal_holy_prism_selector) rather than
// resizing/capping here - the actual target count cap is expected to already be data-driven via
// this spell's own DB2 effect data, same as every other Holy Power heal in this file.
// NOTE: ArgusCore has no caster-relative "ally" target type distinct from "party" - used
// TARGET_UNIT_CASTER_AREA_PARTY (the closest valid enum for a self-centered ally heal) since
// TARGET_UNIT_CASTER_AREA_ALLY does not exist anywhere in this engine. If this hook doesn't
// fire, that assumption is the first thing to check against Legion 7.3.5 DB2 data.
class spell_pal_word_of_glory : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (!targets.empty())
            targets.sort(Trinity::Predicates::HealthPctOrderPred());
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_word_of_glory::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_PARTY);
    }
};

// 28789 - Holy Power
class spell_pal_t3_6p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_PALADIN_HOLY_POWER_ARMOR,
            SPELL_PALADIN_HOLY_POWER_ATTACK_POWER,
            SPELL_PALADIN_HOLY_POWER_SPELL_POWER,
            SPELL_PALADIN_HOLY_POWER_MP5
        });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
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
                spellId = SPELL_PALADIN_HOLY_POWER_MP5;
                break;
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                spellId = SPELL_PALADIN_HOLY_POWER_SPELL_POWER;
                break;
            case CLASS_HUNTER:
            case CLASS_ROGUE:
                spellId = SPELL_PALADIN_HOLY_POWER_ATTACK_POWER;
                break;
            case CLASS_WARRIOR:
                spellId = SPELL_PALADIN_HOLY_POWER_ARMOR;
                break;
            default:
                return;
        }

        caster->CastSpell(target, spellId, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_t3_6p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 64890 - Item - Paladin T8 Holy 2P Bonus
class spell_pal_t8_2p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_HOLY_MENDING });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_PALADIN_HOLY_MENDING, GetCastDifficulty());
        int32 amount = CalculatePct(static_cast<int32>(healInfo->GetHeal()), aurEff->GetAmount());

        ASSERT(spellInfo->GetMaxTicks() > 0);
        amount /= spellInfo->GetMaxTicks();

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(amount);
        caster->CastSpell(target, SPELL_PALADIN_HOLY_MENDING, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_t8_2p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 269569 - Zeal
class spell_pal_zeal : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_ZEAL_AURA });
    }

    void HandleEffectProc(AuraEffect* aurEff, ProcEventInfo& /*procInfo*/)
    {
        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_PALADIN_ZEAL_AURA, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_AURA_STACK, aurEff->GetAmount()));

        PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_zeal::HandleEffectProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

void AddSC_paladin_spell_scripts()
{
    RegisterSpellScript(spell_pal_ardent_defender);
    RegisterSpellScript(spell_pal_art_of_war);
    RegisterSpellScript(spell_pal_avengers_shield);
    RegisterSpellScript(spell_pal_awakening);
    RegisterSpellScript(spell_pal_blessing_of_protection);
    RegisterSpellScript(spell_pal_blinding_light);
    RegisterSpellScript(spell_pal_crusade);
    RegisterSpellScript(spell_pal_crusader_might);
    RegisterSpellScript(spell_pal_the_fires_of_justice);
    RegisterSpellScript(spell_pal_consecration);
    RegisterAreaTriggerAI(areatrigger_pal_consecration);
    RegisterSpellScript(spell_pal_divine_purpose);
    RegisterSpellScript(spell_pal_judgement_of_the_pure);
    RegisterSpellScript(spell_pal_holy_shield);
    RegisterSpellScript(spell_pal_bastion_of_light);
    RegisterSpellScript(spell_pal_divine_intervention);
    RegisterSpellScript(spell_pal_blade_of_wrath_proc);
    RegisterSpellScript(spell_pal_seraphim);
    RegisterSpellScript(spell_pal_greater_blessing_of_kings);
    RegisterSpellScript(spell_pal_light_of_the_martyr);
    RegisterSpellScript(spell_pal_fervent_martyr);
    RegisterSpellScript(spell_pal_light_of_the_protector);
    RegisterSpellScriptWithArgs(spell_pal_light_of_the_protector, "spell_pal_hand_of_the_protector");
    RegisterSpellScript(spell_pal_light_of_dawn);
    RegisterSpellScript(spell_pal_light_of_dawn_trigger);
    RegisterSpellScript(spell_pal_divine_hammer);
    RegisterSpellScript(spell_pal_divine_shield);
    RegisterSpellScript(spell_pal_divine_steed);
    RegisterSpellScript(spell_pal_divine_storm);
    RegisterSpellScript(spell_pal_eye_for_an_eye);
    RegisterSpellScript(spell_pal_final_verdict);
    RegisterSpellScript(spell_pal_fist_of_justice);
    RegisterSpellScript(spell_pal_glyph_of_holy_light);
    RegisterSpellScript(spell_pal_grand_crusader);
    RegisterSpellScript(spell_pal_hammer_of_the_righteous);
    RegisterSpellScript(spell_pal_hand_of_sacrifice);
    RegisterSpellScript(spell_pal_infusion_of_light);
    RegisterSpellScript(spell_pal_moment_of_glory);
    RegisterSpellScript(spell_pal_judgment);
    RegisterSpellScript(spell_pal_judgment_of_light);
    RegisterSpellScript(spell_pal_judgment_of_light_proc);
    RegisterSpellScript(spell_pal_justicars_vengeance);
    RegisterSpellScript(spell_pal_holy_prism);
    RegisterSpellScript(spell_pal_holy_prism_selector);
    RegisterSpellScript(spell_pal_holy_shock);
    RegisterSpellScript(spell_pal_holy_shock_damage_visual);
    RegisterSpellScript(spell_pal_holy_shock_heal_visual);
    RegisterSpellScript(spell_pal_holy_wrath);
    RegisterSpellScript(spell_pal_item_healing_discount);
    RegisterSpellScript(spell_pal_item_t6_trinket);
    RegisterSpellScript(spell_pal_lay_on_hands);
    RegisterSpellScript(spell_pal_light_s_beacon);
    RegisterSpellScript(spell_pal_beacon_of_faith_proc);
    RegisterSpellScript(spell_pal_beacon_of_light);
    RegisterSpellScriptWithArgs(spell_pal_beacon_of_light, "spell_pal_beacon_of_faith_check_cast");
    RegisterSpellScript(spell_pal_beacon_of_light_aura);
    RegisterSpellScriptWithArgs(spell_pal_beacon_of_light_aura, "spell_pal_beacon_of_faith_aura");
    RegisterSpellScriptWithArgs(spell_pal_beacon_of_light_aura, "spell_pal_beacon_of_virtue_aura");
    RegisterSpellScript(spell_pal_light_hammer_init_summon);
    RegisterSpellScript(spell_pal_light_hammer_periodic);
    RegisterSpellScript(spell_pal_righteous_protector);
    RegisterSpellScript(spell_pal_righteous_verdict);
    RegisterSpellScript(spell_pal_selfless_healer);
    RegisterSpellScript(spell_pal_shield_of_the_righteous);
    RegisterSpellScript(spell_pal_shield_of_vengeance);
    RegisterSpellScript(spell_pal_aura_of_sacrifice);
    RegisterSpellScript(spell_pal_aura_of_sacrifice_ally);
    RegisterSpellScript(spell_pal_steed_of_liberty);
    RegisterSpellScript(spell_pal_templar_s_verdict);
    RegisterSpellScript(spell_pal_word_of_glory);
    RegisterSpellScript(spell_pal_t3_6p_bonus);
    RegisterSpellScript(spell_pal_t8_2p_bonus);
    RegisterSpellScript(spell_pal_zeal);
}
