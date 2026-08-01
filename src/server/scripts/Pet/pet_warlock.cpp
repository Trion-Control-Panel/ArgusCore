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
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "npc_pet_warl_".
 */

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "PetAI.h"

enum WarlockPetSpells
{
    SPELL_WARLOCK_DOOM       = 603,
    SPELL_WARLOCK_EYE_LASER  = 205231
};

// 103673 - Darkglare (Summon Darkglare, Affliction cooldown): fires Eye Laser at any nearby
// enemy affected by Doom - the actual damage/refresh targeting is entirely handled by
// spell_warl_eye_laser's own area-target-select override in spell_warlock.cpp (clears the
// default target list and refills it with every Doom-afflicted enemy within range), so this
// AI only needs to find one valid Doom target to trigger the cast. Confirmed via two
// independent reference sources (identical implementations). Deliberately does not call PetAI::UpdateAI() -
// Darkglare has no melee/movement behavior in either reference, it only ever casts.
// Translated Unit::GetAttackableUnitListInRange (doesn't exist in ArgusCore) to the
// established Trinity::AnyUnfriendlyUnitInObjectRangeCheck/UnitListSearcher idiom already used
// elsewhere this session (e.g. Monk's Expel Harm), and the old
// CastSpell(target, id, triggered, item, aura, originalCasterGuid) overload (doesn't exist) to
// CastSpellExtraArgs::SetOriginalCaster.
struct npc_pet_warlock_darkglare : public PetAI
{
    npc_pet_warlock_darkglare(Creature* creature) : PetAI(creature) { }

    void UpdateAI(uint32 /*diff*/) override
    {
        Unit* owner = me->GetOwner();
        if (!owner)
            return;

        std::list<Unit*> nearby;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck checker(owner, owner, 100.0f);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(owner, nearby, checker);
        Cell::VisitAllObjects(owner, searcher, 100.0f);

        for (Unit* target : nearby)
        {
            if (target->HasAura(SPELL_WARLOCK_DOOM, owner->GetGUID()))
            {
                me->CastSpell(target, SPELL_WARLOCK_EYE_LASER, CastSpellExtraArgs(TRIGGERED_FULL_MASK).SetOriginalCaster(owner->GetGUID()));
                break;
            }
        }
    }
};

void AddSC_warlock_pet_scripts()
{
    RegisterCreatureAI(npc_pet_warlock_darkglare);
}
