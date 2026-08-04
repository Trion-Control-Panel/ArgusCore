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
 * Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pri_".
 */

#include "ScriptMgr.h"
#include "AreaTriggerAI.h"
#include "Containers.h"
#include "G3DPosition.hpp"
#include "GridNotifiers.h"
#include "ListUtils.h"
#include "Log.h"
#include "MoveSplineInitArgs.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

enum PriestSpells
{
    SPELL_PRIEST_ABYSSAL_REVERIE                    = 373054,
    SPELL_PRIEST_ANGELIC_FEATHER_AREATRIGGER        = 158624,
    SPELL_PRIEST_ANGELIC_FEATHER_AURA               = 121557,
    SPELL_PRIEST_ANSWERED_PRAYERS                   = 394289,
    SPELL_PRIEST_APOTHEOSIS                         = 200183,
    SPELL_PRIEST_ARMOR_OF_FAITH                     = 28810,
    SPELL_PRIEST_ASSURED_SAFETY                     = 440766,
    SPELL_PRIEST_ATONEMENT                          = 81749,
    SPELL_PRIEST_ATONEMENT_EFFECT                   = 194384,
    SPELL_PRIEST_ATONEMENT_HEAL                     = 81751,
    SPELL_PRIEST_BENEDICTION                        = 193157,
    SPELL_PRIEST_BLAZE_OF_LIGHT                     = 215768,
    SPELL_PRIEST_BLAZE_OF_LIGHT_INCREASE            = 355851,
    SPELL_PRIEST_BLAZE_OF_LIGHT_DECREASE            = 356084,
    SPELL_PRIEST_BLESSED_HEALING                    = 70772,
    SPELL_PRIEST_BLESSED_LIGHT                      = 196813,
    SPELL_PRIEST_BODY_AND_SOUL                      = 64129,
    SPELL_PRIEST_BODY_AND_SOUL_SPEED                = 65081,
    SPELL_PRIEST_CIRCLE_OF_HEALING                  = 204883,
    SPELL_PRIEST_CRYSTALLINE_REFLECTION             = 373457,
    SPELL_PRIEST_CRYSTALLINE_REFLECTION_HEAL        = 373462,
    SPELL_PRIEST_CRYSTALLINE_REFLECTION_REFLECT     = 373464,
    SPELL_PRIEST_DARK_INDULGENCE                    = 372972,
    SPELL_PRIEST_DARK_REPRIMAND                     = 400169,
    SPELL_PRIEST_DARK_REPRIMAND_CHANNEL_DAMAGE      = 373129,
    SPELL_PRIEST_DARK_REPRIMAND_CHANNEL_HEALING     = 400171,
    SPELL_PRIEST_DARK_REPRIMAND_DAMAGE              = 373130,
    SPELL_PRIEST_DARK_REPRIMAND_HEALING             = 400187,
    SPELL_PRIEST_DAZZLING_LIGHT                     = 196810,
    SPELL_PRIEST_DIVINE_BLESSING                    = 40440,
    SPELL_PRIEST_DIVINE_HYMN_HEAL                   = 64844,
    SPELL_PRIEST_DIVINE_IMAGE_SUMMON                = 392990,
    SPELL_PRIEST_DIVINE_IMAGE_EMPOWER               = 409387,
    SPELL_PRIEST_DIVINE_IMAGE_EMPOWER_STACK         = 405963,
    SPELL_PRIEST_DIVINE_SERVICE                     = 391233,
    SPELL_PRIEST_DIVINE_STAR_HOLY                   = 110744,
    SPELL_PRIEST_DIVINE_STAR_SHADOW                 = 122121,
    SPELL_PRIEST_DIVINE_STAR_HOLY_DAMAGE            = 122128,
    SPELL_PRIEST_DIVINE_STAR_HOLY_HEAL              = 110745,
    SPELL_PRIEST_DIVINE_STAR_SHADOW_DAMAGE          = 390845,
    SPELL_PRIEST_DIVINE_STAR_SHADOW_HEAL            = 390981,
    SPELL_PRIEST_DIVINE_WRATH                       = 40441,
    SPELL_PRIEST_EMPOWERED_RENEW_HEAL               = 391359,
    // SPELL_PRIEST_EPIPHANY / EPIPHANY_HIGHLIGHT removed - 414553/414556 is a Shadowlands-era
    // Discipline talent (Power Word: Radiance/Prayer of Mending free-cast proc), confirmed
    // forward drift: no "Epiphany" spell of any id in the real 7.3.5.26972 client data fits
    // this mechanic. See ARGUSCORE_FIXES.md.
    SPELL_PRIEST_ESSENCE_DEVOURER                   = 415479,
    SPELL_PRIEST_ESSENCE_DEVOURER_SHADOWFIEND_HEAL  = 415673,
    SPELL_PRIEST_ESSENCE_DEVOURER_MINDBENDER_HEAL   = 415676,
    SPELL_PRIEST_FLASH_HEAL                         = 2061,
    SPELL_PRIEST_FOCUSED_WILL_BUFF                  = 45242,
    SPELL_PRIEST_FROM_DARKNESS_COMES_LIGHT_AURA     = 390617,
    SPELL_PRIEST_GREATER_HEAL                       = 289666,
    SPELL_PRIEST_FOCUSED_MENDING                    = 372354,
    SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL               = 48153,
    SPELL_PRIEST_HALO_HOLY                          = 120517,
    SPELL_PRIEST_HALO_SHADOW                        = 120644,
    SPELL_PRIEST_HALO_HOLY_DAMAGE                   = 120696,
    SPELL_PRIEST_HALO_HOLY_HEAL                     = 120692,
    SPELL_PRIEST_HALO_SHADOW_DAMAGE                 = 390964,
    SPELL_PRIEST_HALO_SHADOW_HEAL                   = 390971,
    SPELL_PRIEST_HEAL                               = 2060,
    SPELL_PRIEST_HEALING_LIGHT                      = 196809,
    SPELL_PRIEST_HEAVENS_WRATH                      = 421558,
    SPELL_PRIEST_HOLY_FIRE                          = 14914,
    SPELL_PRIEST_HOLY_MENDING_HEAL                  = 391156,
    SPELL_PRIEST_HOLY_NOVA                          = 132157,
    SPELL_PRIEST_HOLY_WORD_CHASTISE                 = 88625,
    SPELL_PRIEST_HOLY_WORD_SALVATION                = 265202,
    SPELL_PRIEST_HOLY_WORD_SANCTIFY                 = 34861,
    SPELL_PRIEST_HOLY_WORD_SERENITY                 = 2050,
    SPELL_PRIEST_HOLY_10_1_CLASS_SET_2P_CHOOSER     = 411097,
    SPELL_PRIEST_HOLY_10_1_CLASS_SET_4P             = 405556,
    SPELL_PRIEST_HOLY_10_1_CLASS_SET_4P_EFFECT      = 409479,
    SPELL_PRIEST_ITEM_EFFICIENCY                    = 37595,
    SPELL_PRIEST_LEAP_OF_FAITH_EFFECT               = 92832,
    SPELL_PRIEST_LEVITATE_EFFECT                    = 111759,
    SPELL_PRIEST_LIGHT_ERUPTION                     = 196812,
    SPELL_PRIEST_LIGHTS_WRATH_VISUAL                = 215795,
    SPELL_PRIEST_LINGERING_INSANITY                 = 197937,
    SPELL_PRIEST_MASOCHISM_TALENT                   = 193063,
    SPELL_PRIEST_MASOCHISM_PERIODIC_HEAL            = 193065,
    SPELL_PRIEST_MIND_DEVOURER                      = 373202,
    SPELL_PRIEST_MIND_DEVOURER_AURA                 = 373204,
    SPELL_PRIEST_MINDBENDER_DISC                    = 123040,
    SPELL_PRIEST_MINDBENDER_SHADOW                  = 200174,
    SPELL_PRIEST_MINDGAMES                          = 375901,
    SPELL_PRIEST_MINDGAMES_VENTHYR                  = 323673,
    SPELL_PRIEST_MIND_BOMB_STUN                     = 226943,
    SPELL_PRIEST_MISERY                             = 238558,
    SPELL_PRIEST_ORACULAR_HEAL                      = 26170,
    SPELL_PRIEST_PAIN_TRANSFORMATION                = 372991,
    SPELL_PRIEST_PAIN_TRANSFORMATION_HEAL           = 372994,
    SPELL_PRIEST_PENANCE                            = 47540,
    SPELL_PRIEST_PENANCE_CHANNEL_DAMAGE             = 47758,
    SPELL_PRIEST_PENANCE_CHANNEL_HEALING            = 47757,
    SPELL_PRIEST_PENANCE_DAMAGE                     = 47666,
    SPELL_PRIEST_PENANCE_HEALING                    = 47750,
    SPELL_PRIEST_POWER_LEECH_MINDBENDER_MANA        = 123051,
    SPELL_PRIEST_POWER_LEECH_MINDBENDER_INSANITY    = 200010,
    SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_MANA       = 343727,
    SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_INSANITY   = 262485,
    SPELL_PRIEST_POWER_OF_THE_DARK_SIDE             = 198069,
    SPELL_PRIEST_POWER_OF_THE_DARK_SIDE_TINT        = 225795,
    SPELL_PRIEST_POWER_WORD_BARRIER_BUFF            = 81782,
    SPELL_PRIEST_POWER_WORD_LIFE                    = 373481,
    SPELL_PRIEST_POWER_WORD_RADIANCE                = 194509,
    SPELL_PRIEST_POWER_WORD_SHIELD                  = 17,
    SPELL_PRIEST_POWER_WORD_SOLACE_ENERGIZE         = 129253,
    SPELL_PRIEST_PRAYER_OF_HEALING                  = 596,
    SPELL_PRIEST_PRAYER_OF_MENDING                  = 33076,
    SPELL_PRIEST_PRAYER_OF_MENDING_AURA             = 41635,
    SPELL_PRIEST_PRAYER_OF_MENDING_HEAL             = 33110,
    SPELL_PRIEST_PRAYER_OF_MENDING_JUMP             = 155793,
    SPELL_PRIEST_PROTECTIVE_LIGHT_AURA              = 193065,
    SPELL_PRIEST_PURIFIED_RESOLVE                   = 196439,
    SPELL_PRIEST_PURIFIED_RESOLVE_BUFF              = 196440,
    SPELL_PRIEST_SPIRITUAL_CLEANSING                = 213654,
    SPELL_PRIEST_PURGE_THE_WICKED                   = 204197,
    SPELL_PRIEST_PURGE_THE_WICKED_DUMMY             = 204215,
    SPELL_PRIEST_PURGE_THE_WICKED_PERIODIC          = 204213,
    SPELL_PRIEST_RAPTURE                            = 47536,
    SPELL_PRIEST_RENEW                              = 139,
    SPELL_PRIEST_RENEWED_HOPE                       = 197469,
    SPELL_PRIEST_RENEWED_HOPE_EFFECT                = 197470,
    SPELL_PRIEST_REVEL_IN_PURITY                    = 373003,
    SPELL_PRIEST_SANCTUARY                          = 231682,
    SPELL_PRIEST_SANCTUARY_ABSORB                   = 208771,
    SPELL_PRIEST_SANCTUARY_AURA                     = 208772,
    SPELL_PRIEST_SHADOW_COVENANT                    = 314867,
    SPELL_PRIEST_SHADOW_COVENANT_EFFECT             = 322105,
    SPELL_PRIEST_RHAPSODY_PROC                      = 390636,
    SPELL_PRIEST_SAY_YOUR_PRAYERS                   = 196358, // real Legion Honor Talent id (391186 is a later-expansion remake id)
    SPELL_PRIEST_SCHISM                             = 214621, // Legion's real Schism has no separate "known" marker id in client data; script relies on this passive-copy pattern
    SPELL_PRIEST_SCHISM_AURA                        = 214621,
    SPELL_PRIEST_SEARING_LIGHT                      = 196811,
    SPELL_PRIEST_SHADOW_MEND_DAMAGE                 = 186439,
    SPELL_PRIEST_SHADOW_WORD_DEATH                  = 32379,
    SPELL_PRIEST_SHADOW_WORD_DEATH_DAMAGE           = 32409,
    SPELL_PRIEST_SHADOW_MEND_PERIODIC_DUMMY         = 187464,
    SPELL_PRIEST_SHADOW_WORD_PAIN                   = 589,
    SPELL_PRIEST_SHADOWFORM_STANCE                  = 232698,
    SPELL_PRIEST_SHADOWY_APPARITION_MISSILE         = 147193,
    SPELL_PRIEST_SHIELD_DISCIPLINE                  = 197045,
    SPELL_PRIEST_SHIELD_DISCIPLINE_EFFECT           = 47755,
    SPELL_PRIEST_SIN_AND_PUNISHMENT                 = 87204,
    SPELL_PRIEST_SINS_OF_THE_MANY                   = 198076, // real Legion id (198074 is the talent-tooltip parent; 198076 is the stacking aura this script casts/removes)
    SPELL_PRIEST_SMITE                              = 585,
    SPELL_PRIEST_SPIRIT_OF_REDEMPTION               = 27827,
    SPELL_PRIEST_STRENGTH_OF_SOUL                   = 197535,
    SPELL_PRIEST_STRENGTH_OF_SOUL_EFFECT            = 197548,
    SPELL_PRIEST_SURGE_OF_LIGHT                     = 109186,
    SPELL_PRIEST_SURGE_OF_LIGHT_EFFECT              = 114255,
    SPELL_PRIEST_TRANQUIL_LIGHT                     = 196816,
    SPELL_PRIEST_THE_PENITENT_AURA                  = 200347,
    SPELL_PRIEST_TRAIL_OF_LIGHT_HEAL                = 234946,
    SPELL_PRIEST_TRINITY                            = 214205,
    SPELL_PRIEST_SPIRIT_SHELL_ABSORPTION             = 114908,
    SPELL_PRIEST_SPIRIT_SHELL_AURA                   = 109964,
    SPELL_PRIEST_TRINITY_EFFECT                     = 214206,
    SPELL_PRIEST_ULTIMATE_PENITENCE                 = 421453,
    SPELL_PRIEST_ULTIMATE_PENITENCE_DAMAGE          = 421543,
    SPELL_PRIEST_ULTIMATE_PENITENCE_HEAL            = 421544,
    SPELL_PRIEST_UNFURLING_DARKNESS                 = 341273,
    SPELL_PRIEST_UNFURLING_DARKNESS_AURA            = 341282,
    SPELL_PRIEST_UNFURLING_DARKNESS_DEBUFF          = 341291,
    SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL              = 15290,
    SPELL_PRIEST_VAMPIRIC_TOUCH                     = 34914,
    SPELL_PRIEST_VOIDFORM_BUFFS                     = 194249,
    SPELL_PRIEST_VOIDFORM_TENTACLES                 = 210196,
    SPELL_PRIEST_VOID_BOLT_DURATION                 = 231688,
    SPELL_PRIEST_VOID_ERUPTION                      = 228260,
    SPELL_PRIEST_VOID_ERUPTION_DAMAGE               = 228360,
    SPELL_PRIEST_VOID_SHIELD                        = 199144,
    SPELL_PRIEST_VOID_SHIELD_EFFECT                 = 199145,
    SPELL_PRIEST_VOID_SHIFT                         = 108968,
    SPELL_PRIEST_VOID_TENDRILS_SUMMON                = 127665,
    SPELL_PRIEST_WEAKENED_SOUL                      = 6788,
    SPELL_PRIEST_WHISPERING_SHADOWS                 = 406777,
    SPELL_PRIEST_WHISPERING_SHADOWS_DUMMY           = 391286,
    SPELL_PVP_RULES_ENABLED_HARDCODED               = 134735
};

enum PriestSpellVisuals
{
    SPELL_VISUAL_PRIEST_POWER_WORD_RADIANCE         = 52872,
    SPELL_VISUAL_PRIEST_PRAYER_OF_MENDING           = 38945,
    SPELL_VISUAL_PRIEST_SHADOWY_APPARITION           = 33584
};

enum PriestSummons
{
    NPC_PRIEST_DIVINE_IMAGE                         = 198236,
    NPC_PRIEST_MINDBENDER                           = 62982,
    NPC_PRIEST_SHADOWFIEND                          = 19668
};

class RaidCheck
{
    public:
        explicit RaidCheck(Unit const* caster) : _caster(caster) { }

        bool operator()(WorldObject* obj) const
        {
            if (Unit* target = obj->ToUnit())
                return !_caster->IsInRaidWith(target);

            return true;
        }

    private:
        Unit const* _caster;
};

// 121536 - Angelic Feather talent
class spell_pri_angelic_feather_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_ANGELIC_FEATHER_AREATRIGGER });
    }

    void HandleEffectDummy(SpellEffIndex /*effIndex*/) const
    {
        Position destPos = GetHitDest()->GetPosition();
        float radius = GetEffectInfo().CalcRadius();

        // Caster is prioritary
        if (GetCaster()->IsWithinDist2d(&destPos, radius))
        {
            GetCaster()->CastSpell(GetCaster(), SPELL_PRIEST_ANGELIC_FEATHER_AURA, true);
        }
        else
        {
            CastSpellExtraArgs args;
            args.TriggerFlags = TRIGGERED_FULL_MASK;
            args.CastDifficulty = GetCastDifficulty();
            GetCaster()->CastSpell(destPos, SPELL_PRIEST_ANGELIC_FEATHER_AREATRIGGER, args);
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_pri_angelic_feather_trigger::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Angelic Feather areatrigger - created by SPELL_PRIEST_ANGELIC_FEATHER_AREATRIGGER
struct areatrigger_pri_angelic_feather : AreaTriggerAI
{
    areatrigger_pri_angelic_feather(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    // Called when the AreaTrigger has just been initialized, just before added to map
    void OnInitialize() override
    {
        if (Unit* caster = at->GetCaster())
        {
            std::vector<AreaTrigger*> areaTriggers = caster->GetAreaTriggers(SPELL_PRIEST_ANGELIC_FEATHER_AREATRIGGER);

            if (areaTriggers.size() >= 3)
                areaTriggers.front()->SetDuration(0);
        }
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (caster->IsFriendlyTo(unit))
            {
                // If target already has aura, increase duration to max 130% of initial duration
                caster->CastSpell(unit, SPELL_PRIEST_ANGELIC_FEATHER_AURA, true);
                at->SetDuration(0);
            }
        }
    }
};

// 26169 - Oracle Healing Bonus
class spell_pri_aq_3p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_ORACULAR_HEAL });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();
        if (caster == eventInfo.GetProcTarget())
            return;

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(CalculatePct(static_cast<int32>(healInfo->GetHeal()), 10));
        caster->CastSpell(caster, SPELL_PRIEST_ORACULAR_HEAL, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_aq_3p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 81749 - Atonement
class spell_pri_atonement : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        // real Sins of the Many (198076) only has EFFECT_0 in this build - UpdateSinsOfTheManyValue()
        // already only touches whichever of EFFECT_0/1/2 actually exists via GetAuraEffect(),
        // so requiring EFFECT_2 up front here was stricter than the code actually needs
        return ValidateSpellInfo({ SPELL_PRIEST_ATONEMENT_HEAL, SPELL_PRIEST_SINS_OF_THE_MANY })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetDamageInfo() != nullptr;
    }

    void HandleOnProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        TriggerAtonementHealOnTargets(aurEff, eventInfo);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pri_atonement::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pri_atonement::HandleOnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

    std::vector<ObjectGuid> _appliedAtonements;

public:
    void AddAtonementTarget(ObjectGuid const& target)
    {
        _appliedAtonements.push_back(target);

        UpdateSinsOfTheManyValue();
    }

    void RemoveAtonementTarget(ObjectGuid const& target)
    {
        std::erase(_appliedAtonements, target);

        UpdateSinsOfTheManyValue();
    }

    std::vector<ObjectGuid> const& GetAtonementTargets() const
    {
        return _appliedAtonements;
    }

    struct TriggerArgs
    {
        SpellInfo const* TriggeredBy = nullptr;
        SpellSchoolMask DamageSchoolMask = SPELL_SCHOOL_MASK_NONE;
    };

    void TriggerAtonementHealOnTargets(AuraEffect const* atonementEffect, ProcEventInfo const& eventInfo)
    {
        Unit* priest = GetUnitOwner();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        CastSpellExtraArgs args(TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR);

        // Note: atonementEffect holds the correct amount since we passed the effect in the AuraScript that calls this method.
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, CalculatePct(damageInfo->GetDamage(), atonementEffect->GetAmount()));

        args.SetCustomArg(TriggerArgs{
            .TriggeredBy = eventInfo.GetSpellInfo(),
            .DamageSchoolMask = eventInfo.GetDamageInfo()->GetSchoolMask() });

        float distanceLimit = GetEffectInfo(EFFECT_1).CalcValue();

        std::erase_if(_appliedAtonements, [priest, distanceLimit, &args](ObjectGuid const& targetGuid)
        {
            if (Unit* target = ObjectAccessor::GetUnit(*priest, targetGuid))
            {
                if (target->IsInDist2d(priest, distanceLimit))
                    priest->CastSpell(target, SPELL_PRIEST_ATONEMENT_HEAL, args);

                return false;
            }

            return true;
        });
    }

    void UpdateSinsOfTheManyValue() const
    {
        // Note: the damage dimish starts at the 6th application as of 10.0.5.
        constexpr std::array<float, 20> damageByStack = { 40.0f, 40.0f, 40.0f, 40.0f, 40.0f, 35.0f, 30.0f, 25.0f, 20.0f, 15.0f, 11.0f, 8.0f, 5.0f, 4.0f, 3.0f, 2.5f, 2.0f, 1.5f, 1.25f, 1.0f };

        for (SpellEffIndex effectIndex : { EFFECT_0, EFFECT_1, EFFECT_2 })
            if (AuraEffect* sinOfTheMany = GetUnitOwner()->GetAuraEffect(SPELL_PRIEST_SINS_OF_THE_MANY, effectIndex))
                sinOfTheMany->ChangeAmount(damageByStack[std::min(_appliedAtonements.size(), damageByStack.size() - 1)]);
    }
};

// 17 - Power Word: Shield
// 139 - Renew
// 2061 - Flash Heal
// 194509 - Power Word: Radiance
class spell_pri_atonement_effect : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_ATONEMENT,
            SPELL_PRIEST_ATONEMENT_EFFECT,
            SPELL_PRIEST_TRINITY,
            SPELL_PRIEST_TRINITY_EFFECT,
            SPELL_PRIEST_POWER_WORD_RADIANCE,
            SPELL_PRIEST_POWER_WORD_SHIELD
        }) && ValidateSpellEffect({
            { SPELL_PRIEST_POWER_WORD_RADIANCE, EFFECT_3 }
        });
    }

    bool Load() override
    {
        Unit* caster = GetCaster();
        if (!caster->HasAura(SPELL_PRIEST_ATONEMENT))
            return false;

        // only apply Trinity if the Priest has both Trinity and Atonement and the triggering spell is Power Word: Shield.
        if (caster->HasAura(SPELL_PRIEST_TRINITY))
        {
            if (GetSpellInfo()->Id != SPELL_PRIEST_POWER_WORD_SHIELD)
                return false;

            _effectSpellId = SPELL_PRIEST_TRINITY_EFFECT;
        }

        return true;
    }

    void HandleOnHitTarget() const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.SetTriggeringSpell(GetSpell());

        switch (GetSpellInfo()->Id)
        {
            case SPELL_PRIEST_POWER_WORD_RADIANCE:
                // Power Word: Radiance applies Atonement at 60 % (without modifiers) of its total duration.
                args.AddSpellMod(SPELLVALUE_DURATION_PCT, GetEffectInfo(EFFECT_3).CalcValue(caster));
                break;
            default:
                break;
        }

        caster->CastSpell(target, _effectSpellId, args);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_pri_atonement_effect::HandleOnHitTarget);
    }

    uint32 _effectSpellId = SPELL_PRIEST_ATONEMENT_EFFECT;
};

// 194384 - Atonement (Buff), 214206 - Atonement [Trinity] (Buff)
class spell_pri_atonement_effect_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_ATONEMENT });
    }

    void HandleOnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        RegisterHelper<&spell_pri_atonement::AddAtonementTarget>();
    }

    void HandleOnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        RegisterHelper<&spell_pri_atonement::RemoveAtonementTarget>();
    }

    template<void(spell_pri_atonement::*func)(ObjectGuid const&)>
    void RegisterHelper() const
    {
        if (Unit* caster = GetCaster())
            if (Aura* atonement = caster->GetAura(SPELL_PRIEST_ATONEMENT))
                if (spell_pri_atonement* script = atonement->GetScript<spell_pri_atonement>())
                    (script->*func)(GetTarget()->GetGUID());
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_pri_atonement_effect_aura::HandleOnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_pri_atonement_effect_aura::HandleOnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 195178 - Atonement (Passive)
class spell_pri_atonement_passive : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellEffect({ { SPELL_PRIEST_ATONEMENT, EFFECT_0 } });
    }

    static bool CheckProc(ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetDamageInfo() != nullptr;
    }

    void HandleOnProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo) const
    {
        Unit* target = GetTarget();
        Unit* summoner = target->GetOwner();
        if (!summoner)
            return;

        if (AuraEffect const* atonementEffect = summoner->GetAuraEffect(SPELL_PRIEST_ATONEMENT, EFFECT_0))
            if (spell_pri_atonement* script = atonementEffect->GetBase()->GetScript<spell_pri_atonement>())
                script->TriggerAtonementHealOnTargets(atonementEffect, eventInfo);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_atonement_passive::HandleOnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 33110 - Prayer of Mending (Heal)
class spell_pri_benediction : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_RENEW })
            && ValidateSpellEffect({ { SPELL_PRIEST_BENEDICTION, EFFECT_0 } });
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/) const
    {
        if (AuraEffect const* benediction = GetCaster()->GetAuraEffect(SPELL_PRIEST_BENEDICTION, EFFECT_0))
            if (roll_chance_i(benediction->GetAmount()))
                GetCaster()->CastSpell(GetHitUnit(), SPELL_PRIEST_RENEW, TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_benediction::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// 204883 - Circle of Healing
class spell_pri_circle_of_healing : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void FilterTargets(std::list<WorldObject*>& targets) const
    {
        // Note: we must remove one since target is always chosen.
        uint32 const maxTargets = uint32(GetSpellInfo()->GetEffect(EFFECT_1).CalcValue(GetCaster()) - 1);

        Trinity::SelectRandomInjuredTargets(targets, maxTargets, true);

        if (Unit* explicitTarget = GetExplTargetUnit())
            targets.push_front(explicitTarget);
    }

    void Register() override
    {
        // real Circle of Healing (204883) TARGET_UNIT_DEST_AREA_ALLY is on EFFECT_1, not EFFECT_0
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_circle_of_healing::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 122121 - Divine Star (Shadow)
class spell_pri_divine_star_shadow : public SpellScript
{
    void HandleHitTarget(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();

        if (caster->GetPowerType() != GetEffectInfo().MiscValue)
            PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_divine_star_shadow::HandleHitTarget, EFFECT_2, SPELL_EFFECT_ENERGIZE);
    }
};

// 110744 - Divine Star (Holy)
// 122121 - Divine Star (Shadow)
struct areatrigger_pri_divine_star : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnInitialize() override
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(at->GetSpellId(), DIFFICULTY_NONE);
        if (!spellInfo)
            return;

        if (spellInfo->GetEffects().size() <= EFFECT_1)
            return;

        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        _casterCurrentPosition = caster->GetPosition();

        // Note: max. distance at which the Divine Star can travel to is EFFECT_1's BasePoints yards.
        float maxTravelDistance = float(spellInfo->GetEffect(EFFECT_1).CalcValue(caster));

        Position destPos = _casterCurrentPosition;
        at->MovePositionToFirstCollision(destPos, maxTravelDistance, 0.0f);

        PathGenerator firstPath(at);
        firstPath.CalculatePath(destPos.GetPositionX(), destPos.GetPositionY(), destPos.GetPositionZ(), false);

        at->InitSplines(firstPath.GetPath());
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void OnUnitEnter(Unit* unit) override
    {
        HandleUnitEnterExit(unit);
    }

    void OnUnitExit(Unit* unit) override
    {
        // Note: this ensures any unit receives a second hit if they happen to be inside the AT when Divine Star starts its return path.
        HandleUnitEnterExit(unit);
    }

    void HandleUnitEnterExit(Unit* unit)
    {
        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        if (advstd::ranges::contains(_affectedUnits, unit->GetGUID()))
            return;

        constexpr TriggerCastFlags TriggerFlags = TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS;

        if (caster->IsValidAttackTarget(unit))
            caster->CastSpell(unit, at->GetSpellId() == SPELL_PRIEST_DIVINE_STAR_SHADOW ? SPELL_PRIEST_DIVINE_STAR_SHADOW_DAMAGE : SPELL_PRIEST_DIVINE_STAR_HOLY_DAMAGE,
                TriggerFlags);
        else if (caster->IsValidAssistTarget(unit))
            caster->CastSpell(unit, at->GetSpellId() == SPELL_PRIEST_DIVINE_STAR_SHADOW ? SPELL_PRIEST_DIVINE_STAR_SHADOW_HEAL : SPELL_PRIEST_DIVINE_STAR_HOLY_HEAL,
                TriggerFlags);

        _affectedUnits.push_back(unit->GetGUID());
    }

    void OnDestinationReached() override
    {
        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        if (at->GetDistance(_casterCurrentPosition) > 0.05f)
        {
            _affectedUnits.clear();

            ReturnToCaster();
        }
        else
            at->Remove();
    }

    void ReturnToCaster()
    {
        _scheduler.Schedule(0ms, [this](TaskContext task)
        {
            Unit* caster = at->GetCaster();
            if (!caster)
                return;

            _casterCurrentPosition = caster->GetPosition();

            Movement::PointsArray returnSplinePoints;

            returnSplinePoints.push_back(PositionToVector3(at));
            returnSplinePoints.push_back(PositionToVector3(caster));

            at->InitSplines(returnSplinePoints);

            task.Repeat(250ms);
        });
    }

private:
    TaskScheduler _scheduler;
    Position _casterCurrentPosition;
    std::vector<ObjectGuid> _affectedUnits;
};

// 246287 - Evangelism
class spell_pri_evangelism : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_TRINITY,
            SPELL_PRIEST_ATONEMENT_EFFECT,
            SPELL_PRIEST_TRINITY_EFFECT
        });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        Aura* atonementAura = caster->HasAura(SPELL_PRIEST_TRINITY)
            ? target->GetAura(SPELL_PRIEST_TRINITY_EFFECT, caster->GetGUID())
            : target->GetAura(SPELL_PRIEST_ATONEMENT_EFFECT, caster->GetGUID());
        if (!atonementAura)
            return;

        Milliseconds extraDuration = Seconds(GetEffectValue());

        atonementAura->SetDuration(atonementAura->GetDuration() + extraDuration.count());
        atonementAura->SetMaxDuration(atonementAura->GetDuration() + extraDuration.count());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_evangelism::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 47788 - Guardian Spirit
class spell_pri_guardian_spirit : public AuraScript
{
    uint32 healPct = 0;

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL }) && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    bool Load() override
    {
        healPct = GetEffectInfo(EFFECT_1).CalcValue();
        return true;
    }

    static void CalculateAmount(AuraScript const&, AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
    {
        // Set absorbtion amount to unlimited
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo const& dmgInfo, uint32 & absorbAmount)
    {
        Unit* target = GetTarget();
        if (dmgInfo.GetDamage() < target->GetHealth())
            return;

        int32 healAmount = int32(target->CountPctFromMaxHealth(healPct));
        // remove the aura now, we don't want 40% healing bonus
        Remove(AURA_REMOVE_BY_ENEMY_SPELL);
        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.AddSpellBP0(healAmount);
        target->CastSpell(target, SPELL_PRIEST_GUARDIAN_SPIRIT_HEAL, args);
        absorbAmount = dmgInfo.GetDamage();
    }

    void Register() override
    {
        // real Guardian Spirit (47788) SCHOOL_ABSORB is at EFFECT_2, not EFFECT_1 (which is a plain DUMMY)
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_guardian_spirit::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_guardian_spirit::Absorb, EFFECT_2);
    }
};

// 45243 - Focused Will
// Baseline Discipline/Holy passive (level 34): taking a melee hit grants a stacking
// (up to 2) damage-reduction buff (45242). Changed in patch 7.0.3 to melee-only.
class spell_pri_focused_will : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_FOCUSED_WILL_BUFF });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo const* damageInfo = eventInfo.GetDamageInfo();
        return damageInfo && (damageInfo->GetAttackType() == BASE_ATTACK || damageInfo->GetAttackType() == OFF_ATTACK);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), SPELL_PRIEST_FOCUSED_WILL_BUFF, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pri_focused_will::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pri_focused_will::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 120644 - Halo (Shadow)
class spell_pri_halo_shadow : public SpellScript
{
    void HandleHitTarget(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();

        if (caster->GetPowerType() != GetEffectInfo().MiscValue)
            PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_halo_shadow::HandleHitTarget, EFFECT_1, SPELL_EFFECT_ENERGIZE);
    }
};

// 120517 - Halo (Holy)
// 120644 - Halo (Shadow)
struct areatrigger_pri_halo : AreaTriggerAI
{
    areatrigger_pri_halo(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (caster->IsValidAttackTarget(unit))
                caster->CastSpell(unit, at->GetSpellId() == SPELL_PRIEST_HALO_SHADOW ? SPELL_PRIEST_HALO_SHADOW_DAMAGE : SPELL_PRIEST_HALO_HOLY_DAMAGE,
                    TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS);
            else if (caster->IsValidAssistTarget(unit))
                caster->CastSpell(unit, at->GetSpellId() == SPELL_PRIEST_HALO_SHADOW ? SPELL_PRIEST_HALO_SHADOW_HEAL : SPELL_PRIEST_HALO_HOLY_HEAL,
                    TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS);
        }
    }
};

// 62618 - Power Word: Barrier
// AreaTriggerId - 1489
struct areatrigger_pri_power_word_barrier : AreaTriggerAI
{
    areatrigger_pri_power_word_barrier(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    void OnUnitEnter(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster || !unit)
            return;

        if (caster->IsFriendlyTo(unit))
            caster->CastSpell(unit, SPELL_PRIEST_POWER_WORD_BARRIER_BUFF, true);
    }

    void OnUnitExit(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster || !unit)
            return;

        if (unit->HasAura(SPELL_PRIEST_POWER_WORD_BARRIER_BUFF, caster->GetGUID()))
            unit->RemoveAurasDueToSpell(SPELL_PRIEST_POWER_WORD_BARRIER_BUFF, caster->GetGUID());
    }
};

// 63733 - Serendipity ("Holy Words")
class spell_pri_holy_words : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        // Note: the cooldown-reduction magnitudes live on Serendipity's (this aura's) own
        // effects (EFFECT_0-2 = Heal/Flash Heal->Serenity, Prayer of Healing->Sanctify,
        // Smite->Chastise), not on the target spells - Sanctify/Chastise/Serenity don't carry
        // matching effect slots for this in 7.3.5.26972. Renew triggering a Sanctify cd
        // reduction (present in older code) isn't part of Serendipity's real tooltip/effect
        // layout in this build and was dropped. See ARGUSCORE_FIXES.md.
        return ValidateSpellInfo(
        {
            SPELL_PRIEST_HEAL,
            SPELL_PRIEST_FLASH_HEAL,
            SPELL_PRIEST_PRAYER_OF_HEALING,
            SPELL_PRIEST_SMITE,
            SPELL_PRIEST_HOLY_WORD_CHASTISE,
            SPELL_PRIEST_HOLY_WORD_SANCTIFY,
            SPELL_PRIEST_HOLY_WORD_SERENITY
        }) && ValidateSpellEffect(
        {
            { spellInfo->Id, EFFECT_0 },
            { spellInfo->Id, EFFECT_1 },
            { spellInfo->Id, EFFECT_2 }
        });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo const& eventInfo) const
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return;

        uint32 targetSpellId;
        SpellEffIndex cdReductionEffIndex;
        switch (spellInfo->Id)
        {
            case SPELL_PRIEST_HEAL:
            case SPELL_PRIEST_FLASH_HEAL: // reduce Holy Word: Serenity cd
                targetSpellId = SPELL_PRIEST_HOLY_WORD_SERENITY;
                cdReductionEffIndex = EFFECT_0;
                break;
            case SPELL_PRIEST_PRAYER_OF_HEALING: // reduce Holy Word: Sanctify cd
                targetSpellId = SPELL_PRIEST_HOLY_WORD_SANCTIFY;
                cdReductionEffIndex = EFFECT_1;
                break;
            case SPELL_PRIEST_SMITE: // reduce Holy Word: Chastise cd
                targetSpellId = SPELL_PRIEST_HOLY_WORD_CHASTISE;
                cdReductionEffIndex = EFFECT_2;
                break;
            default:
                TC_LOG_WARN("spells.priest", "HolyWords aura has been proced by an unknown spell: {}", GetSpellInfo()->Id);
                return;
        }

        SpellInfo const* targetSpellInfo = sSpellMgr->AssertSpellInfo(targetSpellId, GetCastDifficulty());
        int32 cdReduction = GetSpellInfo()->GetEffect(cdReductionEffIndex).CalcValue(GetTarget());
        GetTarget()->GetSpellHistory()->ModifyCooldown(targetSpellInfo, Seconds(-cdReduction), true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_holy_words::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 40438 - Priest Tier 6 Trinket
class spell_pri_item_t6_trinket : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_DIVINE_BLESSING, SPELL_PRIEST_DIVINE_WRATH });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();
        if (eventInfo.GetSpellTypeMask() & PROC_SPELL_TYPE_HEAL)
            caster->CastSpell(nullptr, SPELL_PRIEST_DIVINE_BLESSING, true);

        if (eventInfo.GetSpellTypeMask() & PROC_SPELL_TYPE_DAMAGE)
            caster->CastSpell(nullptr, SPELL_PRIEST_DIVINE_WRATH, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_item_t6_trinket::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 92833 - Leap of Faith
class spell_pri_leap_of_faith_effect_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_LEAP_OF_FAITH_EFFECT });
    }

    void HandleEffectDummy(SpellEffIndex /*effIndex*/) const
    {
        Position destPos = GetHitDest()->GetPosition();

        SpellCastTargets targets;
        targets.SetDst(destPos);
        targets.SetUnitTarget(GetCaster());
        GetHitUnit()->CastSpell(std::move(targets), GetEffectValue(), GetCastDifficulty());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_leap_of_faith_effect_trigger::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 1706 - Levitate
class spell_pri_levitate : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_LEVITATE_EFFECT });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_PRIEST_LEVITATE_EFFECT, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_levitate::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 373178 - Light's Wrath
class spell_pri_lights_wrath : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    void OnPrecast() override
    {
        Aura const* atonement = GetCaster()->GetAura(SPELL_PRIEST_ATONEMENT);
        if (!atonement)
            return;

        spell_pri_atonement const* script = atonement->GetScript<spell_pri_atonement>();
        if (!script)
            return;

        for (ObjectGuid const& atonementTarget : script->GetAtonementTargets())
        {
            if (Unit* target = ObjectAccessor::GetUnit(*GetCaster(), atonementTarget))
            {
                target->CancelSpellMissiles(SPELL_PRIEST_LIGHTS_WRATH_VISUAL, false, false);
                target->CastSpell(GetCaster(), SPELL_PRIEST_LIGHTS_WRATH_VISUAL, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
            }
        }
    }

    void CalculateDamageBonus(SpellEffectInfo const& /*spellEffectInfo*/, Unit const* /*victim*/, int32 const& /*damage*/, int32 const& /*flatMod*/, float& pctMod) const
    {
        Aura const* atonement = GetCaster()->GetAura(SPELL_PRIEST_ATONEMENT);
        if (!atonement)
            return;

        // Atonement size may have changed when missile hits, we need to take an updated count of Atonement applications.
        if (spell_pri_atonement const* script = atonement->GetScript<spell_pri_atonement>())
            AddPct(pctMod, GetEffectInfo(EFFECT_1).CalcValue(GetCaster()) * script->GetAtonementTargets().size());
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_pri_lights_wrath::CalculateDamageBonus);
    }
};

// 205369 - Mind Bomb
class spell_pri_mind_bomb : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_MIND_BOMB_STUN });
    }

    void RemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            if (Unit* caster = GetCaster())
                caster->CastSpell(GetTarget()->GetPosition(), SPELL_PRIEST_MIND_BOMB_STUN, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_pri_mind_bomb::RemoveEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 47540 - Penance
// 400169 - Dark Reprimand
class spell_pri_penance : public SpellScript
{
public:
    spell_pri_penance(uint32 damageSpellId, uint32 healingSpellId) : _damageSpellId(damageSpellId), _healingSpellId(healingSpellId)
    {
    }

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ _damageSpellId, _healingSpellId });
    }

    SpellCastResult CheckCast() const
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

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();

        if (Unit* target = GetHitUnit())
        {
            if (caster->IsFriendlyTo(target))
                caster->CastSpell(target, _healingSpellId, CastSpellExtraArgs(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD)
                    .SetTriggeringSpell(GetSpell()));
            else
                caster->CastSpell(target, _damageSpellId, CastSpellExtraArgs(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD)
                    .SetTriggeringSpell(GetSpell()));
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pri_penance::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_pri_penance::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }

private:
    uint32 _damageSpellId;
    uint32 _healingSpellId;
};

// 47758 - Penance (Channel Damage), 47757 - Penance (Channel Healing)
// 373129 - Dark Reprimand (Channel Damage), 400171 - Dark Reprimand (Channel Healing)
class spell_pri_penance_or_dark_reprimand_channeled : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_POWER_OF_THE_DARK_SIDE });
    }

    void HandleOnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (Unit* caster = GetCaster())
            caster->RemoveAura(SPELL_PRIEST_POWER_OF_THE_DARK_SIDE);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_pri_penance_or_dark_reprimand_channeled::HandleOnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 114239 - Phantasm
class spell_pri_phantasm : public SpellScript
{
    void HandleEffectHit(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->RemoveMovementImpairingAuras(false);
    }

    void Register() override
    {
        // real Phantasm (114239) EFFECT_0 is SPELL_EFFECT_APPLY_AURA, not DUMMY
        OnEffectHit += SpellEffectFn(spell_pri_phantasm::HandleEffectHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 262484 - Power Leech (Passive for Shadowfiend)
// 284621 - Power Leech (Passive for Mindbender)
class spell_pri_power_leech_passive : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_INSANITY,
            SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_MANA,
            SPELL_PRIEST_POWER_LEECH_MINDBENDER_INSANITY,
            SPELL_PRIEST_POWER_LEECH_MINDBENDER_MANA
        })
            && ValidateSpellEffect
        ({
            { SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_INSANITY, EFFECT_0 },
            { SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_MANA, EFFECT_0 },
            { SPELL_PRIEST_POWER_LEECH_MINDBENDER_INSANITY, EFFECT_0 },
            { SPELL_PRIEST_POWER_LEECH_MINDBENDER_MANA, EFFECT_0 }
        });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetDamageInfo() != nullptr;
    }

    void HandleOnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/) const
    {
        Unit* target = GetTarget();
        Player* summoner = Object::ToPlayer(target->GetOwner());
        if (!summoner)
            return;

        SpellInfo const* spellInfo = nullptr;
        int32 divisor = 1;

        if (summoner->GetPrimarySpecialization() != ChrSpecialization::PriestShadow)
        {
            if (target->GetEntry() == NPC_PRIEST_SHADOWFIEND)
            {
                // Note: divisor is 100 because effect value is 5 and its supposed to restore 0.5%
                spellInfo = sSpellMgr->AssertSpellInfo(SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_MANA, GetCastDifficulty());
                divisor = 10;
            }
            else
            {
                // Note: divisor is 100 because effect value is 20 and its supposed to restore 0.2%
                spellInfo = sSpellMgr->AssertSpellInfo(SPELL_PRIEST_POWER_LEECH_MINDBENDER_MANA, GetCastDifficulty());
                divisor = 100;
            }
        }
        else
            spellInfo = sSpellMgr->AssertSpellInfo(target->GetEntry() == NPC_PRIEST_SHADOWFIEND
                ? SPELL_PRIEST_POWER_LEECH_SHADOWFIEND_INSANITY
                : SPELL_PRIEST_POWER_LEECH_MINDBENDER_INSANITY, GetCastDifficulty());

        target->CastSpell(summoner, spellInfo->Id, CastSpellExtraArgs(aurEff)
            .AddSpellMod(SPELLVALUE_BASE_POINT0, spellInfo->GetEffect(EFFECT_0).CalcValue() / divisor));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pri_power_leech_passive::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pri_power_leech_passive::HandleOnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 198069 - Power of the Dark Side
class spell_pri_power_of_the_dark_side : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_POWER_OF_THE_DARK_SIDE_TINT });
    }

    void HandleOnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_PRIEST_POWER_OF_THE_DARK_SIDE_TINT, true);
    }

    void HandleOnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (Unit* caster = GetCaster())
            caster->RemoveAura(SPELL_PRIEST_POWER_OF_THE_DARK_SIDE_TINT);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_pri_power_of_the_dark_side::HandleOnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_pri_power_of_the_dark_side::HandleOnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 47666 - Penance (Damage)
// 373130 - Dark Reprimand (Damage)
class spell_pri_power_of_the_dark_side_damage_bonus : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_POWER_OF_THE_DARK_SIDE });
    }

    void CalculateDamageBonus(SpellEffectInfo const& /*spellEffectInfo*/, Unit* /*victim*/, int32& /*damage*/, int32& /*flatMod*/, float& pctMod) const
    {
        if (AuraEffect* powerOfTheDarkSide = GetCaster()->GetAuraEffect(SPELL_PRIEST_POWER_OF_THE_DARK_SIDE, EFFECT_0))
            AddPct(pctMod, powerOfTheDarkSide->GetAmount());
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_pri_power_of_the_dark_side_damage_bonus::CalculateDamageBonus);
    }
};

// 47750 - Penance (Healing)
// 400187 - Dark Reprimand (Healing)
class spell_pri_power_of_the_dark_side_healing_bonus : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_POWER_OF_THE_DARK_SIDE });
    }

    void CalculateHealingBonus(SpellEffectInfo const& /*spellEffectInfo*/, Unit* /*victim*/, int32& /*healing*/, int32& /*flatMod*/, float& pctMod) const
    {
        if (AuraEffect* powerOfTheDarkSide = GetCaster()->GetAuraEffect(SPELL_PRIEST_POWER_OF_THE_DARK_SIDE, EFFECT_0))
            AddPct(pctMod, powerOfTheDarkSide->GetAmount());
    }

    void Register() override
    {
        CalcHealing += SpellCalcHealingFn(spell_pri_power_of_the_dark_side_healing_bonus::CalculateHealingBonus);
    }
};

// 194509 - Power Word: Radiance
class spell_pri_power_word_radiance : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_ATONEMENT_EFFECT });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* explTarget = GetExplTargetUnit();

        // we must add one since explicit target is always chosen.
        uint32 maxTargets = GetEffectInfo(EFFECT_2).CalcValue(GetCaster()) + 1;

        if (targets.size() > maxTargets)
        {
            // priority is: a) no Atonement b) injured c) anything else (excluding explicit target which is always added).
            targets.sort([this, explTarget](WorldObject* lhs, WorldObject* rhs)
            {
                if (lhs == explTarget) // explTarget > anything: always true
                    return true;
                if (rhs == explTarget) // anything > explTarget: always false
                    return false;

                return MakeSortTuple(lhs) > MakeSortTuple(rhs);
            });

            targets.resize(maxTargets);
        }

        for (WorldObject* target : targets)
        {
            if (target == explTarget)
                continue;

            _visualTargets.push_back(target->GetGUID());
        }
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/) const
    {
        for (ObjectGuid const& guid : _visualTargets)
            if (Unit* target = ObjectAccessor::GetUnit(*GetHitUnit(), guid))
                GetHitUnit()->SendPlaySpellVisual(target, SPELL_VISUAL_PRIEST_POWER_WORD_RADIANCE, 0, 0, 70.0f);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_power_word_radiance::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
        OnEffectHitTarget += SpellEffectFn(spell_pri_power_word_radiance::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }

private:
    std::tuple<bool, bool> MakeSortTuple(WorldObject* obj) const
    {
        return std::make_tuple(IsUnitWithNoAtonement(obj), IsUnitInjured(obj));
    }

    // Returns true if obj is a unit but has no atonement
    bool IsUnitWithNoAtonement(WorldObject* obj) const
    {
        Unit* unit = obj->ToUnit();
        return unit && !unit->HasAura(SPELL_PRIEST_ATONEMENT_EFFECT, GetCaster()->GetGUID());
    }

    // Returns true if obj is a unit and is injured
    static bool IsUnitInjured(WorldObject* obj)
    {
        Unit* unit = obj->ToUnit();
        return unit && !unit->IsFullHealth();
    }

    std::vector<ObjectGuid> _visualTargets;
};

// 17 - Power Word: Shield
class spell_pri_power_word_shield : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_STRENGTH_OF_SOUL,
            SPELL_PRIEST_STRENGTH_OF_SOUL_EFFECT,
            SPELL_PRIEST_ATONEMENT_EFFECT,
            SPELL_PRIEST_TRINITY_EFFECT,
            SPELL_PRIEST_SHIELD_DISCIPLINE,
            SPELL_PRIEST_SHIELD_DISCIPLINE_EFFECT,
            SPELL_PVP_RULES_ENABLED_HARDCODED
        }) && ValidateSpellEffect({
            { SPELL_PRIEST_RAPTURE, EFFECT_1 }
        });
    }

    void CalculateAmount(AuraEffect const* auraEffect, int32& amount, bool& canBeRecalculated) const
    {
        canBeRecalculated = false;

        if (Unit* caster = GetCaster())
        {
            float modifiedAmount = caster->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()) * 3.36f;

            if (Player* player = caster->ToPlayer())
            {
                AddPct(modifiedAmount, player->GetRatingBonusValue(CR_VERSATILITY_DAMAGE_DONE));

                switch (player->GetPrimarySpecialization())
                {
                    case ChrSpecialization::PriestDiscipline:
                        modifiedAmount *= 1.37f;
                        break;
                    case ChrSpecialization::PriestShadow:
                        modifiedAmount *= 1.25f;
                        if (caster->HasAura(SPELL_PVP_RULES_ENABLED_HARDCODED))
                            modifiedAmount *= 0.8f;
                        break;
                    default:
                        break;
                }
            }

            float critChanceDone = caster->SpellCritChanceDone(nullptr, auraEffect, GetSpellInfo()->GetSchoolMask(), GetSpellInfo()->GetAttackType());
            float critChanceTaken = GetUnitOwner()->SpellCritChanceTaken(caster, nullptr, auraEffect, GetSpellInfo()->GetSchoolMask(), critChanceDone, GetSpellInfo()->GetAttackType());

            if (roll_chance_f(critChanceTaken))
                modifiedAmount *= 2;

            // Rapture talent (TBD: move into DoEffectCalcDamageAndHealing hook).
            if (AuraEffect const* raptureEffect = caster->GetAuraEffect(SPELL_PRIEST_RAPTURE, EFFECT_1))
                AddPct(modifiedAmount, raptureEffect->GetAmount());

            amount = modifiedAmount;
        }
    }

    void HandleOnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/) const
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // Note: Strength of Soul PvP talent.
        if (caster->HasAura(SPELL_PRIEST_STRENGTH_OF_SOUL))
            caster->CastSpell(GetTarget(), SPELL_PRIEST_STRENGTH_OF_SOUL_EFFECT, aurEff);
    }

    void HandleOnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/) const
    {
        GetTarget()->RemoveAura(SPELL_PRIEST_STRENGTH_OF_SOUL_EFFECT);

        // Note: Shield Discipline talent.
        if (Unit* caster = GetCaster())
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL && caster->HasAura(SPELL_PRIEST_SHIELD_DISCIPLINE))
                caster->CastSpell(caster, SPELL_PRIEST_SHIELD_DISCIPLINE_EFFECT, aurEff);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_power_word_shield::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        AfterEffectApply += AuraEffectApplyFn(spell_pri_power_word_shield::HandleOnApply, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        AfterEffectRemove += AuraEffectRemoveFn(spell_pri_power_word_shield::HandleOnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

// spell_pri_divine_aegis removed - bound to spell 47515, which doesn't exist in 7.3.5.26972's
// client data under any id or name. Divine Aegis (Cata/MoP/WoD heal-crit shield proc) was
// removed from the game in Legion's Discipline Priest redesign in favor of Atonement. See
// ARGUSCORE_FIXES.md.

// 129250 - Power Word: Solace
class spell_pri_power_word_solace : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_POWER_WORD_SOLACE_ENERGIZE });
    }

    void RestoreMana(SpellEffIndex /*effIndex*/) const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_PRIEST_POWER_WORD_SOLACE_ENERGIZE,
            CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetTriggeringSpell(GetSpell())
                .AddSpellMod(SPELLVALUE_BASE_POINT0, GetEffectValue() / 100));
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_pri_power_word_solace::RestoreMana, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// Base class used by various Prayer of Mending spells
class spell_pri_prayer_of_mending_SpellScriptBase : public SpellScript
{
public:
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_PRAYER_OF_MENDING_HEAL, SPELL_PRIEST_PRAYER_OF_MENDING_AURA })
            && ValidateSpellEffect({ { SPELL_PRIEST_PRAYER_OF_MENDING_HEAL, EFFECT_0 } });
    }

    bool Load() override
    {
        _spellInfoHeal = sSpellMgr->AssertSpellInfo(SPELL_PRIEST_PRAYER_OF_MENDING_HEAL, DIFFICULTY_NONE);
        return true;
    }

    void CastPrayerOfMendingAura(Unit* caster, Unit* target, Unit* visualSender, uint8 stack, bool firstCast) const
    {
        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        args.AddSpellMod(SPELLVALUE_AURA_STACK, stack);

        // Note: this line's purpose is to show the correct amount in Points field in SMSG_AURA_UPDATE.
        SpellEffectInfo const* healEffectDummy = &_spellInfoHeal->GetEffect(EFFECT_0);
        uint32 basePoints = caster->SpellHealingBonusDone(target, _spellInfoHeal, healEffectDummy->CalcValue(caster), HEAL, *healEffectDummy);
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, basePoints);

        // Note: Focused Mending talent.
        args.SetCustomArg(firstCast);

        caster->CastSpell(target, SPELL_PRIEST_PRAYER_OF_MENDING_AURA, args);

        // Note: the visualSender is the priest if it is first cast or the aura holder when the aura triggers.
        visualSender->SendPlaySpellVisual(target, SPELL_VISUAL_PRIEST_PRAYER_OF_MENDING, 0, 0, 40.0f);
    }

protected:
    SpellInfo const* _spellInfoHeal = nullptr;
};

// 33076 - Prayer of Mending (Dummy)
class spell_pri_prayer_of_mending_dummy : public spell_pri_prayer_of_mending_SpellScriptBase
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_PRAYER_OF_MENDING_AURA
        });
    }

    void HandleEffectDummy(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        // Note: we need to increase BasePoints by 1 since it's 4 as default. Also HACKFIX, we shouldn't reduce it by 1 if the target has the aura already.
        uint8 stackAmount = target->HasAura(SPELL_PRIEST_PRAYER_OF_MENDING_AURA, caster->GetGUID()) ? GetEffectValue() : GetEffectValue() + 1;

        CastPrayerOfMendingAura(caster, target, caster, stackAmount, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending_dummy::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 41635 - Prayer of Mending (Aura)
class spell_pri_prayer_of_mending_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_PRAYER_OF_MENDING_HEAL, SPELL_PRIEST_PRAYER_OF_MENDING_JUMP })
            && ValidateSpellEffect({ { SPELL_PRIEST_SAY_YOUR_PRAYERS, EFFECT_0 } });
    }

    void HandleHeal(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/)
    {
        // Note: caster is the priest who cast the spell and target is current holder of the aura.
        Unit* target = GetTarget();

        if (Unit* caster = GetCaster())
        {
            CastSpellExtraArgs args(aurEff);
            args.SetCustomArg(_isEmpoweredByFocusedMending);

            caster->CastSpell(target, SPELL_PRIEST_PRAYER_OF_MENDING_HEAL, args);

            // Note: jump is only executed if higher than 1 stack.
            int32 stackAmount = GetStackAmount();
            if (stackAmount > 1)
            {
                args.OriginalCaster = caster->GetGUID();

                int32 newStackAmount = stackAmount - 1;
                if (AuraEffect* sayYourPrayers = caster->GetAuraEffect(SPELL_PRIEST_SAY_YOUR_PRAYERS, EFFECT_0))
                    if (roll_chance_i(sayYourPrayers->GetAmount()))
                        ++newStackAmount;

                args.AddSpellMod(SPELLVALUE_BASE_POINT0, newStackAmount);

                target->CastSpell(target, SPELL_PRIEST_PRAYER_OF_MENDING_JUMP, args);
            }

            Remove();
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_prayer_of_mending_aura::HandleHeal, EFFECT_0, SPELL_AURA_DUMMY);
    }

public:
    void SetEmpoweredByFocusedMending(bool isEmpowered)
    {
        _isEmpoweredByFocusedMending = isEmpowered;
    }

private:
    bool _isEmpoweredByFocusedMending = false;
};

class spell_pri_prayer_of_mending : public SpellScript
{
    void HandleEffectDummy(SpellEffIndex /*effIndex*/) const
    {
        Aura* aura = GetHitAura();
        if (!aura)
            return;

        spell_pri_prayer_of_mending_aura* script = aura->GetScript<spell_pri_prayer_of_mending_aura>();
        if (!script)
            return;

        if (bool const* isEmpoweredByFocusedMending = std::any_cast<bool>(&GetSpell()->m_customArg))
            script->SetEmpoweredByFocusedMending(isEmpoweredByFocusedMending);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 155793 - Prayer of Mending (Jump)
class spell_pri_prayer_of_mending_jump : public spell_pri_prayer_of_mending_SpellScriptBase
{
    static void FilterTargets(SpellScript const&, std::list<WorldObject*>& targets)
    {
        Trinity::SelectRandomInjuredTargets(targets, 1, true);
    }

    void HandleJump(SpellEffIndex /*effIndex*/) const
    {
        if (Unit* origCaster = GetOriginalCaster())
            CastPrayerOfMendingAura(origCaster, GetHitUnit(), GetCaster(), GetEffectValue(), false);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_prayer_of_mending_jump::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending_jump::HandleJump, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 193063 - Protective Light (Aura)
class spell_pri_protective_light : public AuraScript
{
    bool CheckEffectProc(AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo) const
    {
        return eventInfo.GetProcTarget() == GetCaster();
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/) const
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_PRIEST_PROTECTIVE_LIGHT_AURA, aurEff);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pri_protective_light::CheckEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pri_protective_light::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 47536 - Rapture
class spell_pri_rapture : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_POWER_WORD_SHIELD });
    }

    void HandleEffectDummy(SpellEffIndex /*effIndex*/)
    {
        _raptureTarget = GetHitUnit()->GetGUID();
    }

    void HandleAfterCast() const
    {
        Unit* caster = GetCaster();

        if (Unit* target = ObjectAccessor::GetUnit(*caster, _raptureTarget))
            caster->CastSpell(target, SPELL_PRIEST_POWER_WORD_SHIELD,
                CastSpellExtraArgs(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_POWER_COST | TRIGGERED_IGNORE_CAST_IN_PROGRESS)
                .SetTriggeringSpell(GetSpell()));
    }

    void Register() override
    {
        // real Rapture (47536) EFFECT_0 is SPELL_EFFECT_APPLY_AURA, not DUMMY
        OnEffectHitTarget += SpellEffectFn(spell_pri_rapture::HandleEffectDummy, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        AfterCast += SpellCastFn(spell_pri_rapture::HandleAfterCast);
    }

private:
    ObjectGuid _raptureTarget;
};

// 8092 - Mind Blast
class spell_pri_schism : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_SCHISM,
            SPELL_PRIEST_SCHISM_AURA
        });
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/) const
    {
        if (GetCaster()->HasAura(SPELL_PRIEST_SCHISM))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_PRIEST_SCHISM_AURA, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_schism::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 208771 - Sanctuary (Absorb)
class spell_pri_sanctuary_absorb : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SANCTUARY_AURA });
    }

    void CalcAbsorbAmount(AuraEffect const* /*aurEff*/, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
    {
        PreventDefaultAction();

        Unit const* attacker = dmgInfo.GetAttacker();
        if (!attacker)
            return;

        AuraEffect* amountHolderEffect = attacker->GetAuraEffect(SPELL_PRIEST_SANCTUARY_AURA, EFFECT_0, GetCasterGUID());
        if (!amountHolderEffect)
            return;

        if (dmgInfo.GetDamage() >= uint32(amountHolderEffect->GetAmount()))
        {
            amountHolderEffect->GetBase()->Remove(AURA_REMOVE_BY_ENEMY_SPELL);
            dmgInfo.AbsorbDamage(amountHolderEffect->GetAmount());
        }
        else
        {
            amountHolderEffect->ChangeAmount(amountHolderEffect->GetAmount() - int32(dmgInfo.GetDamage()));
            dmgInfo.AbsorbDamage(dmgInfo.GetDamage());
        }
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_sanctuary_absorb::CalcAbsorbAmount, EFFECT_0);
    }
};

// Smite - 585
class spell_pri_sanctuary_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SANCTUARY, SPELL_PRIEST_SANCTUARY_AURA, SPELL_PRIEST_SANCTUARY_ABSORB });
    }

    void HandleEffectHit(SpellEffIndex /*effIndex*/) const
    {
        Player* caster = Object::ToPlayer(GetCaster());
        if (!caster)
            return;

        if (AuraEffect const* sanctuaryEffect = caster->GetAuraEffect(SPELL_PRIEST_SANCTUARY, EFFECT_0))
        {
            if (Unit* target = GetHitUnit())
            {
                float absorbAmount = CalculatePct<float, float>(caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW), sanctuaryEffect->GetAmount());
                AddPct(absorbAmount, caster->GetRatingBonusValue(CR_VERSATILITY_DAMAGE_DONE));

                caster->CastSpell(caster, SPELL_PRIEST_SANCTUARY_ABSORB, CastSpellExtraArgs()
                    .SetTriggerFlags(TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR)
                    .SetTriggeringSpell(GetSpell()));

                caster->CastSpell(target, SPELL_PRIEST_SANCTUARY_AURA, CastSpellExtraArgs()
                    .AddSpellMod(SPELLVALUE_BASE_POINT0, absorbAmount)
                    .SetTriggerFlags(TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR)
                    .SetTriggeringSpell(GetSpell()));
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_sanctuary_trigger::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 280391 - Sins of the Many
class spell_pri_sins_of_the_many : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SINS_OF_THE_MANY });
    }

    void HandleOnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        GetTarget()->CastSpell(GetTarget(), SPELL_PRIEST_SINS_OF_THE_MANY, true);
    }

    void HandleOnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        GetTarget()->RemoveAura(SPELL_PRIEST_SINS_OF_THE_MANY);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_pri_sins_of_the_many::HandleOnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_pri_sins_of_the_many::HandleOnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 20711 - Spirit of Redemption
class spell_pri_spirit_of_redemption : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SPIRIT_OF_REDEMPTION });
    }

    void HandleAbsorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& /*absorbAmount*/)
    {
        Unit* target = GetTarget();
        if (dmgInfo.GetDamage() >= target->GetHealth())
        {
            target->CastSpell(target, SPELL_PRIEST_SPIRIT_OF_REDEMPTION, aurEff);
            target->SetFullHealth();
            return;
        }

        PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_spirit_of_redemption::HandleAbsorb, EFFECT_0);
    }
};

// 186263 - Shadow Mend
class spell_pri_shadow_mend : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_ATONEMENT,
            SPELL_PRIEST_ATONEMENT_EFFECT,
            SPELL_PRIEST_TRINITY,
            SPELL_PRIEST_MASOCHISM_TALENT,
            SPELL_PRIEST_MASOCHISM_PERIODIC_HEAL,
            SPELL_PRIEST_SHADOW_MEND_PERIODIC_DUMMY
        });
    }

    void HandleEffectHit() const
    {
        if (Unit* target = GetHitUnit())
        {
            Unit* caster = GetCaster();

            int32 periodicAmount = GetHitHeal() / 20;
            int32 damageForAuraRemoveAmount = periodicAmount * 10;

            // Handle Masochism talent
            if (caster->HasAura(SPELL_PRIEST_MASOCHISM_TALENT) && caster->GetGUID() == target->GetGUID())
                caster->CastSpell(caster, SPELL_PRIEST_MASOCHISM_PERIODIC_HEAL, CastSpellExtraArgs(GetSpell()).AddSpellMod(SPELLVALUE_BASE_POINT0, periodicAmount));
            else if (target->IsInCombat() && periodicAmount)
            {
                CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
                args.SetTriggeringSpell(GetSpell());
                args.AddSpellMod(SPELLVALUE_BASE_POINT0, periodicAmount);
                args.AddSpellMod(SPELLVALUE_BASE_POINT1, damageForAuraRemoveAmount);
                caster->CastSpell(target, SPELL_PRIEST_SHADOW_MEND_PERIODIC_DUMMY, args);
            }
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_pri_shadow_mend::HandleEffectHit);
    }
};

// 187464 - Shadow Mend (Damage)
class spell_pri_shadow_mend_periodic_damage : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SHADOW_MEND_DAMAGE });
    }

    void HandleDummyTick(AuraEffect const* aurEff) const
    {
        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.SetOriginalCaster(GetCasterGUID());
        args.SetTriggeringAura(aurEff);
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, aurEff->GetAmount());
        GetTarget()->CastSpell(GetTarget(), SPELL_PRIEST_SHADOW_MEND_DAMAGE, args);
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetDamageInfo() != nullptr;
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo const& eventInfo)
    {
        int32 newAmount = aurEff->GetAmount() - eventInfo.GetDamageInfo()->GetDamage();

        aurEff->ChangeAmount(newAmount);
        if (newAmount < 0)
            Remove();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_shadow_mend_periodic_damage::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        DoCheckProc += AuraCheckProcFn(spell_pri_shadow_mend_periodic_damage::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pri_shadow_mend_periodic_damage::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 32379 - Shadow Word: Death
class spell_pri_shadow_word_death : public SpellScript
{
    // real Shadow Word: Death (32379) only has 3 effects (EFFECT_0 SCHOOL_DAMAGE, EFFECT_1 DUMMY
    // "health threshold %", EFFECT_2 DUMMY "damage bonus %") - the health-check/damage-bonus
    // effects were off by one (EFFECT_2/EFFECT_3), and there's no EFFECT_5 or SCRIPT_EFFECT at
    // all, so the backlash-on-non-kill mechanic (SPELL_PRIEST_SHADOW_WORD_DEATH_DAMAGE, 32409,
    // also confirmed absent from Spell.db2) has no data source in this build - left unresolved.
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 }, { spellInfo->Id, EFFECT_2 } })
            && spellInfo->GetEffect(EFFECT_1).IsEffect(SPELL_EFFECT_DUMMY)
            && spellInfo->GetEffect(EFFECT_2).IsEffect(SPELL_EFFECT_DUMMY);
    }

    void HandleDamageCalculation(SpellEffectInfo const& /*spellEffectInfo*/, Unit const* victim, int32 const& /*damage*/, int32 const& /*flatMod*/, float& pctMod) const
    {
        if (victim->HealthBelowPct(GetEffectInfo(EFFECT_1).CalcValue(GetCaster())))
            AddPct(pctMod, GetEffectInfo(EFFECT_2).CalcValue(GetCaster()));
    }

    void Register() override
    {
        CalcDamage += SpellCalcDamageFn(spell_pri_shadow_word_death::HandleDamageCalculation);
    }
};

// 78203 - Shadowy Apparitions
// Shadow Word: Pain crits have a chance to spawn a homing missile that deals damage to its target.
class spell_pri_shadowy_apparitions : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SHADOWY_APPARITION_MISSILE, SPELL_PRIEST_SHADOW_WORD_PAIN });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_PRIEST_SHADOW_WORD_PAIN
            && (eventInfo.GetHitMask() & PROC_HIT_CRITICAL);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* target = GetTarget();
        Unit* actionTarget = eventInfo.GetActionTarget();
        if (!target || !actionTarget)
            return;

        target->CastSpell(actionTarget, SPELL_PRIEST_SHADOWY_APPARITION_MISSILE, true);
        target->SendPlaySpellVisual(actionTarget, SPELL_VISUAL_PRIEST_SHADOWY_APPARITION, SPELL_MISS_NONE, SPELL_MISS_NONE, 6.0f, false);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_shadowy_apparitions::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        DoCheckProc += AuraCheckProcFn(spell_pri_shadowy_apparitions::CheckProc);
    }
};

// 228260 - Void Eruption
// Shadow's core cooldown: deals initial damage to targets already carrying Shadow Word: Pain or
// Vampiric Touch, then enters Voidform.
class spell_pri_void_eruption : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_VOID_ERUPTION_DAMAGE, SPELL_PRIEST_VOIDFORM_BUFFS, SPELL_PRIEST_SHADOWFORM_STANCE });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        targets.remove_if([caster](WorldObject* target)
        {
            Unit* unit = target->ToUnit();
            if (!unit)
                return true;

            return !(unit->HasAura(SPELL_PRIEST_SHADOW_WORD_PAIN, caster->GetGUID()) || unit->HasAura(SPELL_PRIEST_VAMPIRIC_TOUCH, caster->GetGUID()));
        });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_PRIEST_VOID_ERUPTION_DAMAGE + urand(0, 1), true);
    }

    void EnterVoidform()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_PRIEST_VOIDFORM_BUFFS, true);
        if (!caster->HasAura(SPELL_PRIEST_SHADOWFORM_STANCE))
            caster->CastSpell(caster, SPELL_PRIEST_SHADOWFORM_STANCE, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_void_eruption::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_pri_void_eruption::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        AfterCast += SpellCastFn(spell_pri_void_eruption::EnterVoidform);
    }
};

// 194249 - Voidform (buffs)
// Every tick, spawns escalating tentacle visuals at specific stack thresholds and refreshes its
// own stacking haste buff; cancels itself once Insanity is fully drained. On removal, converts
// its current haste bonus (plus this file's own Voidform Buffs rank-3 bonus, if any) into
// Lingering Insanity's snapshot amount.
class spell_pri_voidform : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->RemoveAurasDueToSpell(SPELL_PRIEST_LINGERING_INSANITY);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->GetPower(POWER_INSANITY) == 0)
        {
            caster->RemoveAura(GetAura());
            return;
        }

        int32 tick = GetAura()->GetStackAmount() - 1;
        switch (tick)
        {
            case 0: caster->CastSpell(caster, SPELL_PRIEST_VOIDFORM_TENTACLES, true); break;
            case 3: caster->CastSpell(caster, SPELL_PRIEST_VOIDFORM_TENTACLES + 1, true); break;
            case 6: caster->CastSpell(caster, SPELL_PRIEST_VOIDFORM_TENTACLES + 2, true); break;
            case 9: caster->CastSpell(caster, SPELL_PRIEST_VOIDFORM_TENTACLES + 3, true); break;
            default: break;
        }

        caster->CastSpell(caster, SPELL_PRIEST_VOIDFORM_BUFFS, true);
    }

    void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        for (int32 i = 0; i < 4; ++i)
            caster->RemoveAurasDueToSpell(SPELL_PRIEST_VOIDFORM_TENTACLES + i);

        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, aurEff->GetAmount());

        if (AuraEffect* rank3 = caster->GetAuraEffectOfRankedSpell(SPELL_PRIEST_VOIDFORM_BUFFS, EFFECT_3, caster->GetGUID()))
            args.AddSpellMod(SPELLVALUE_BASE_POINT1, rank3->GetAmount());

        caster->CastSpell(caster, SPELL_PRIEST_LINGERING_INSANITY, args);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_voidform::HandlePeriodic, EFFECT_4, SPELL_AURA_PERIODIC_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_pri_voidform::HandleRemove, EFFECT_2, SPELL_AURA_MELEE_SLOW, AURA_EFFECT_HANDLE_REAL);
        AfterEffectApply += AuraEffectApplyFn(spell_pri_voidform::HandleApply, EFFECT_4, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 234746 - Void Bolt
// Extends the duration of Shadow Word: Pain and Vampiric Touch on its target.
class spell_pri_void_bolt : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Aura* durationBuff = caster->GetAura(SPELL_PRIEST_VOID_BOLT_DURATION);
        if (!durationBuff)
            return;

        AuraEffect* durationEffect = durationBuff->GetEffect(EFFECT_0);
        if (!durationEffect)
            return;

        int32 durationIncreaseMs = durationEffect->GetBaseAmount();

        if (Aura* pain = target->GetAura(SPELL_PRIEST_SHADOW_WORD_PAIN, caster->GetGUID()))
            pain->SetDuration(pain->GetDuration() + durationIncreaseMs);

        if (Aura* vampiricTouch = target->GetAura(SPELL_PRIEST_VAMPIRIC_TOUCH, caster->GetGUID()))
            vampiricTouch->SetDuration(vampiricTouch->GetDuration() + durationIncreaseMs);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_void_bolt::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 108920 - Void Tendrils
// Summons a root-effect totem at the target location.
class spell_pri_void_tendrils : public SpellScript
{
    void HandleOnHit()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (player && target)
            player->CastSpell(target, SPELL_PRIEST_VOID_TENDRILS_SUMMON, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_pri_void_tendrils::HandleOnHit);
    }
};

// 109186 - Surge of Light
class spell_pri_surge_of_light : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_SMITE,
            SPELL_PRIEST_SURGE_OF_LIGHT_EFFECT
        });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        if (eventInfo.GetSpellInfo()->Id == SPELL_PRIEST_SMITE)
            return true;

        if (eventInfo.GetSpellInfo()->SpellFamilyName == SPELLFAMILY_PRIEST)
            return eventInfo.GetHealInfo();

        return false;
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/) const
    {
        if (roll_chance_i(aurEff->GetAmount()))
            GetTarget()->CastSpell(GetTarget(), SPELL_PRIEST_SURGE_OF_LIGHT_EFFECT, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pri_surge_of_light::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pri_surge_of_light::HandleEffectProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 28809 - Greater Heal
class spell_pri_t3_4p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_ARMOR_OF_FAITH });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), SPELL_PRIEST_ARMOR_OF_FAITH, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_t3_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 37594 - Greater Heal Refund
class spell_pri_t5_heal_2p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_ITEM_EFFICIENCY });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        if (HealInfo* healInfo = eventInfo.GetHealInfo())
            if (Unit* healTarget = healInfo->GetTarget())
                if (healInfo->GetEffectiveHeal())
                    if (healTarget->GetHealth() >= healTarget->GetMaxHealth())
                        return true;

        return false;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SPELL_PRIEST_ITEM_EFFICIENCY, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pri_t5_heal_2p_bonus::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pri_t5_heal_2p_bonus::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 70770 - Item - Priest T10 Healer 2P Bonus
class spell_pri_t10_heal_2p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_BLESSED_HEALING });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_PRIEST_BLESSED_HEALING, GetCastDifficulty());
        int32 amount = CalculatePct(static_cast<int32>(healInfo->GetHeal()), aurEff->GetAmount());

        ASSERT(spellInfo->GetMaxTicks() > 0);
        amount /= spellInfo->GetMaxTicks();

        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(amount);
        caster->CastSpell(target, SPELL_PRIEST_BLESSED_HEALING, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_t10_heal_2p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 200128 - Trail of Light
class spell_pri_trail_of_light : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_TRAIL_OF_LIGHT_HEAL });
    }

    bool CheckProc(ProcEventInfo const& eventInfo)
    {
        if (_healQueue.empty() || _healQueue.back() != eventInfo.GetActionTarget()->GetGUID())
            _healQueue.push(eventInfo.GetActionTarget()->GetGUID());

        if (_healQueue.size() > 2)
            _healQueue.pop();

        if (_healQueue.size() == 2)
            return true;

        return false;
    }

    void HandleOnProc(AuraEffect* aurEff, ProcEventInfo const& eventInfo) const
    {
        Unit* caster = GetTarget();
        Unit* oldTarget = ObjectAccessor::GetUnit(*caster, _healQueue.front());
        if (!oldTarget)
            return;

        // Note: old target may not be friendly anymore due to charm and faction change effects.
        if (!caster->IsValidAssistTarget(oldTarget))
            return;

        SpellInfo const* healSpellInfo = sSpellMgr->GetSpellInfo(SPELL_PRIEST_TRAIL_OF_LIGHT_HEAL, DIFFICULTY_NONE);
        if (!healSpellInfo)
            return;

        // Note: distance may be greater than the heal's spell range.
        if (!caster->IsWithinDist(oldTarget, healSpellInfo->GetMaxRange(true, caster)))
            return;

        uint32 healAmount = CalculatePct(eventInfo.GetHealInfo()->GetHeal(), aurEff->GetAmount());

        caster->CastSpell(oldTarget, SPELL_PRIEST_TRAIL_OF_LIGHT_HEAL, CastSpellExtraArgs(aurEff).AddSpellBP0(healAmount));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pri_trail_of_light::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_pri_trail_of_light::HandleOnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

private:
    std::queue<ObjectGuid> _healQueue;
};

// 390693 - Train of Thought
// Called by Flash Heal, Renew, Smite
class spell_pri_train_of_thought : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_PRIEST_POWER_WORD_SHIELD,
            SPELL_PRIEST_PENANCE
        });
    }

    static bool CheckEffect0(AuraScript const&, AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        // Renew & Flash Heal
        return eventInfo.GetSpellInfo()->IsAffected(SPELLFAMILY_PRIEST, { 0x840 });
    }

    static bool CheckEffect1(AuraScript const&, AuraEffect const* /*aurEff*/, ProcEventInfo const& eventInfo)
    {
        // Smite
        return eventInfo.GetSpellInfo()->IsAffected(SPELLFAMILY_PRIEST, { 0x80 });
    }

    void ReducePowerWordShieldCooldown(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/) const
    {
        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_PRIEST_POWER_WORD_SHIELD, Milliseconds(aurEff->GetAmount()));
    }

    void ReducePenanceCooldown(AuraEffect const* aurEff, ProcEventInfo const& /*eventInfo*/) const
    {
        GetTarget()->GetSpellHistory()->ModifyCooldown(SPELL_PRIEST_PENANCE, Milliseconds(aurEff->GetAmount()));
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pri_train_of_thought::CheckEffect0, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pri_train_of_thought::ReducePowerWordShieldCooldown, EFFECT_0, SPELL_AURA_DUMMY);
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pri_train_of_thought::CheckEffect1, EFFECT_1, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pri_train_of_thought::ReducePenanceCooldown, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 109142 - Twist of Fate (Shadow)
// 265259 - Twist of Fate (Discipline)
class spell_pri_twist_of_fate : public AuraScript
{
    static bool CheckProc(AuraScript const&, AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        return eventInfo.GetProcTarget()->GetHealthPct() < aurEff->GetAmount();
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_pri_twist_of_fate::CheckProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 108968 - Void Shift
// Swaps the caster's and target's current health percentages, both floored at 25%.
class spell_pri_void_shift : public SpellScript
{
    SpellCastResult CheckTarget()
    {
        if (Unit* target = GetExplTargetUnit())
            if (target->GetTypeId() != TYPEID_PLAYER)
                return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        float casterPct = std::max(caster->GetHealthPct(), 25.0f);
        float targetPct = std::max(target->GetHealthPct(), 25.0f);

        caster->SetHealth(caster->CountPctFromMaxHealth(int32(targetPct)));
        target->SetHealth(target->CountPctFromMaxHealth(int32(casterPct)));
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pri_void_shift::CheckTarget);
        OnEffectHitTarget += SpellEffectFn(spell_pri_void_shift::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 109964 - Spirit Shell
// Discipline's capstone: while active, converts the priest's outgoing healing into a
// stacking absorb shield on the target instead of direct healing, capped at 60% of the
// target's own max health. Generic heal-redirect hooked to whichever heal spell ids get a
// spell_script_names row bound to this ScriptName, matching this file's existing
// spell_pri_atonement_effect pattern - bound to Heal/Flash Heal/Prayer of Healing/Renew for
// now (the confirmed core heal spells already known to this file); the full intended list of
// intercepted heals needs Legion 7.3.5 tooltip/DB2 verification before broadening further.
// CORRECTION from the reference implementation: the reference checked the absorb aura
// effect on the *caster* (the priest) to decide whether to extend an existing shield or
// apply a new one - that doesn't match a per-target stacking shield, so this checks the
// aura on the *target* being healed instead, and caps at the target's own max health.
class spell_pri_spirit_shell : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SPIRIT_SHELL_AURA, SPELL_PRIEST_SPIRIT_SHELL_ABSORPTION });
    }

    void HandleHeal()
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->HasAura(SPELL_PRIEST_SPIRIT_SHELL_AURA))
            return;

        Unit* target = GetHitUnit();
        if (!target)
            return;

        int32 healAmount = GetHitHeal();
        SetHitHeal(0);

        if (AuraEffect* shell = target->GetAuraEffect(SPELL_PRIEST_SPIRIT_SHELL_ABSORPTION, EFFECT_0))
            shell->SetAmount(std::min(shell->GetAmount() + healAmount, int32(target->CountPctFromMaxHealth(60))));
        else
            caster->CastSpell(target, SPELL_PRIEST_SPIRIT_SHELL_ABSORPTION, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, healAmount));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_pri_spirit_shell::HandleHeal);
    }
};

// 15286 - Vampiric Embrace
class spell_pri_vampiric_embrace : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL });
    }

    static bool CheckProc(AuraScript const&, ProcEventInfo const& eventInfo)
    {
        // Not proc from Mind Sear
        return !(eventInfo.GetDamageInfo()->GetSpellInfo()->SpellFamilyFlags[1] & 0x80000);
    }

    void HandleEffectProc(AuraEffect const* aurEff, ProcEventInfo const& eventInfo)
    {
        PreventDefaultAction();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        int32 selfHeal = int32(CalculatePct(damageInfo->GetDamage(), aurEff->GetAmount()));
        int32 teamHeal = selfHeal / 2;

        CastSpellExtraArgs args(aurEff);
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, teamHeal);
        args.AddSpellMod(SPELLVALUE_BASE_POINT1, selfHeal);
        GetTarget()->CastSpell(nullptr, SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_pri_vampiric_embrace::CheckProc);
        // real Vampiric Embrace (15286) EFFECT_0 is SPELL_AURA_PERIODIC_DUMMY, not plain DUMMY
        OnEffectProc += AuraEffectProcFn(spell_pri_vampiric_embrace::HandleEffectProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 15290 - Vampiric Embrace (heal)
class spell_pri_vampiric_embrace_target : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& unitList) const
    {
        unitList.remove(GetCaster());
    }

    void Register() override
    {
        // real Vampiric Embrace Target (15290) implicit target is TARGET_UNIT_CASTER_AREA_RAID, not _PARTY
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_vampiric_embrace_target::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
    }
};

// 8122 - Psychic Scream
// Breaks the fear early once the feared target has taken more than 10% of its max health in
// cumulative damage - standard "fear breaks on sufficient damage" behavior, which isn't
// automatic engine behavior and needs to be tracked explicitly.
// NOTE: the reference tracks the running damage total via Aura::Variables, a generic runtime
// key-value scripting-storage member that doesn't exist anywhere in ArgusCore's engine. Since an
// AuraScript instance already lives for exactly one aura application's lifetime, a plain member
// variable does the same job with no functional difference - the same workaround already
// established for spell_dh_sigil_of_misery_fear elsewhere in this pass.
// 527 - Purify
// Requires a dispellable magic effect to be present on the target; if the Purified Resolve
// (Honor Talent) is known, also grants the target an absorb shield; if Spiritual Cleansing is
// known, refunds part of this spell's own cooldown after casting.
class spell_pri_purify : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_PURIFIED_RESOLVE, SPELL_PRIEST_PURIFIED_RESOLVE_BUFF, SPELL_PRIEST_SPIRITUAL_CLEANSING });
    }

    SpellCastResult CheckCleansing()
    {
        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();
        if (!caster || !target)
            return SPELL_CAST_OK;

        DispelChargesList dispelList;
        for (SpellEffectInfo const& effect : GetSpellInfo()->GetEffects())
        {
            if (!effect.IsEffect())
                continue;

            uint32 dispelMask = GetSpellInfo()->GetDispelMask(DispelType(effect.MiscValue));
            target->GetDispellableAuraList(caster, dispelMask, dispelList);
        }

        return dispelList.empty() ? SPELL_FAILED_NOTHING_TO_DISPEL : SPELL_CAST_OK;
    }

    void HandleCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetExplTargetUnit();
        if (!player || !target || !player->HasAura(SPELL_PRIEST_PURIFIED_RESOLVE))
            return;

        SpellInfo const* purifiedResolve = sSpellMgr->AssertSpellInfo(SPELL_PRIEST_PURIFIED_RESOLVE, GetCastDifficulty());
        int32 absorb = CalculatePct(target->GetMaxHealth(), purifiedResolve->GetEffect(EFFECT_0).CalcValue(player));

        player->CastSpell(target, SPELL_PRIEST_PURIFIED_RESOLVE_BUFF, CastSpellExtraArgs(TRIGGERED_FULL_MASK)
            .AddSpellMod(SPELLVALUE_BASE_POINT0, absorb));
    }

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (player && player->HasAura(SPELL_PRIEST_SPIRITUAL_CLEANSING) && player->GetSpellHistory()->HasCooldown(GetSpellInfo()->Id))
            player->GetSpellHistory()->ModifyCooldown(GetSpellInfo()->Id, Seconds(-8), true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pri_purify::CheckCleansing);
        OnCast += SpellCastFn(spell_pri_purify::HandleCast);
        AfterCast += SpellCastFn(spell_pri_purify::HandleAfterCast);
    }
};

// 8122 - Psychic Scream
// Breaks the fear early once the feared target has taken more than 10% of its max health in
// cumulative damage - standard "fear breaks on sufficient damage" behavior, which isn't
// automatic engine behavior and needs to be tracked explicitly.
class spell_pri_psychic_scream : public AuraScript
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
        DoCheckProc += AuraCheckProcFn(spell_pri_psychic_scream::CheckProc);
    }
};

// 32375 - Mass Dispel
// Special-cases removing Cyclone (33786, plus its PvP-talent variant 209753) from friendly
// targets - Cyclone is normally flagged undispellable by anything else, so this is a deliberate
// carve-out on top of whatever this spell's own DB2 dispel effect already handles.
class spell_pri_mass_dispel : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/) const
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !caster->IsFriendlyTo(target))
            return;

        target->RemoveAura(33786);
        target->RemoveAura(209753);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_mass_dispel::HandleHit, EFFECT_0, SPELL_EFFECT_DISPEL);
    }
};

// 34914 - Vampiric Touch
class spell_pri_vampiric_touch : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PRIEST_SIN_AND_PUNISHMENT, SPELL_PRIEST_SHADOW_WORD_PAIN });
    }

    void HandleDispel(DispelInfo const* dispelInfo) const
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(dispelInfo->GetDispeller(), SPELL_PRIEST_SIN_AND_PUNISHMENT, true);
    }

    void HandleApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_PRIEST_MISERY))
                caster->CastSpell(GetTarget(), SPELL_PRIEST_SHADOW_WORD_PAIN, true);
    }

    void Register() override
    {
        AfterDispel += AuraDispelFn(spell_pri_vampiric_touch::HandleDispel);
        OnEffectApply += AuraEffectApplyFn(spell_pri_vampiric_touch::HandleApplyEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};


void AddSC_priest_spell_scripts()
{
    RegisterSpellScript(spell_pri_angelic_feather_trigger);
    RegisterAreaTriggerAI(areatrigger_pri_angelic_feather);
    RegisterSpellScript(spell_pri_aq_3p_bonus);
    RegisterSpellScript(spell_pri_atonement);
    RegisterSpellScript(spell_pri_atonement_effect);
    RegisterSpellScript(spell_pri_atonement_effect_aura);
    RegisterSpellScript(spell_pri_atonement_passive);
    RegisterSpellScript(spell_pri_benediction);
    RegisterSpellScript(spell_pri_circle_of_healing);
    RegisterSpellScript(spell_pri_divine_star_shadow);
    RegisterAreaTriggerAI(areatrigger_pri_divine_star);
    RegisterSpellScript(spell_pri_evangelism);
    RegisterSpellScript(spell_pri_guardian_spirit);
    RegisterSpellScript(spell_pri_focused_will);
    RegisterSpellScript(spell_pri_void_shift);
    RegisterSpellScript(spell_pri_spirit_shell);
    RegisterSpellScript(spell_pri_halo_shadow);
    RegisterAreaTriggerAI(areatrigger_pri_halo);
    RegisterSpellScript(spell_pri_holy_words);
    RegisterAreaTriggerAI(areatrigger_pri_power_word_barrier);
    RegisterSpellScript(spell_pri_item_t6_trinket);
    RegisterSpellScript(spell_pri_leap_of_faith_effect_trigger);
    RegisterSpellScript(spell_pri_levitate);
    RegisterSpellScript(spell_pri_lights_wrath);
    RegisterSpellScript(spell_pri_mind_bomb);
    RegisterSpellScriptWithArgs(spell_pri_penance, "spell_pri_penance", SPELL_PRIEST_PENANCE_CHANNEL_DAMAGE, SPELL_PRIEST_PENANCE_CHANNEL_HEALING);
    RegisterSpellScript(spell_pri_penance_or_dark_reprimand_channeled);
    RegisterSpellScript(spell_pri_phantasm);
    RegisterSpellScript(spell_pri_power_leech_passive);
    RegisterSpellScript(spell_pri_power_of_the_dark_side);
    RegisterSpellScript(spell_pri_power_of_the_dark_side_damage_bonus);
    RegisterSpellScript(spell_pri_power_of_the_dark_side_healing_bonus);
    RegisterSpellScript(spell_pri_power_word_radiance);
    RegisterSpellScript(spell_pri_power_word_shield);
    RegisterSpellScript(spell_pri_power_word_solace);
    RegisterSpellScript(spell_pri_prayer_of_mending_dummy);
    RegisterSpellAndAuraScriptPair(spell_pri_prayer_of_mending, spell_pri_prayer_of_mending_aura);
    RegisterSpellScript(spell_pri_prayer_of_mending_jump);
    RegisterSpellScript(spell_pri_protective_light);
    RegisterSpellScript(spell_pri_rapture);
    RegisterSpellScript(spell_pri_sanctuary_absorb);
    RegisterSpellScript(spell_pri_sanctuary_trigger);
    RegisterSpellScript(spell_pri_schism);
    RegisterSpellScript(spell_pri_sins_of_the_many);
    RegisterSpellScript(spell_pri_spirit_of_redemption);
    RegisterSpellScript(spell_pri_shadow_mend);
    RegisterSpellScript(spell_pri_shadow_mend_periodic_damage);
    RegisterSpellScript(spell_pri_shadow_word_death);
    RegisterSpellScript(spell_pri_shadowy_apparitions);
    RegisterSpellScript(spell_pri_void_eruption);
    RegisterSpellScript(spell_pri_voidform);
    RegisterSpellScript(spell_pri_void_bolt);
    RegisterSpellScript(spell_pri_void_tendrils);
    RegisterSpellScript(spell_pri_surge_of_light);
    RegisterSpellScript(spell_pri_trail_of_light);
    RegisterSpellScript(spell_pri_train_of_thought);
    RegisterSpellScript(spell_pri_t3_4p_bonus);
    RegisterSpellScript(spell_pri_t5_heal_2p_bonus);
    RegisterSpellScript(spell_pri_t10_heal_2p_bonus);
    RegisterSpellScript(spell_pri_twist_of_fate);
    RegisterSpellScript(spell_pri_vampiric_embrace);
    RegisterSpellScript(spell_pri_vampiric_embrace_target);
    RegisterSpellScript(spell_pri_purify);
    RegisterSpellScript(spell_pri_psychic_scream);
    RegisterSpellScript(spell_pri_mass_dispel);
    RegisterSpellScript(spell_pri_vampiric_touch);
}
