-- Warlock: removed the entire modern Succubus/Incubus "Sayaad" pact-summon system
-- (spell_warl_random_sayaad, spell_warl_sayaad_precast_disorientation,
-- spell_warl_strengthen_pact_succubus, spell_warl_strengthen_pact_incubus, spell_warl_summon_sayaad)
-- - confirmed drift, no Legion connection. Part of the ongoing Legion 7.3.5 forward-drift removal
-- pass (see ARGUSCORE_FIXES.md).
--
-- The Incubus pet was added in patch 9.2.0 (2022-02-22, Shadowlands); the "Sayaad" gender-neutral
-- species/customization system merging Succubus and Incubus came later in Dragonflight patch
-- 10.1.5. Neither existed at any point during Legion. The genuine Legion-era ability is simply
-- "712 - Summon Succubus" cast directly, with no wrapper/pact/random-pet-choice layer at all -
-- confirmed by AshamaneCore having no script for this ability whatsoever (its plain SPELL_EFFECT_SUMMON
-- effect data needs none). SPELL_WARLOCK_SUMMON_SUCCUBUS (712) itself, and the class implementing
-- Succubus's own Seduction/Whiplash abilities, are untouched.
--
-- Also removed the matching engine-level surface in TemporarySummon.h: PET_INCUBUS (creature entry
-- 184600, itself a modern-era id) and Minion::IsPetSayaad() were deleted, restoring the genuine
-- Legion-era Minion::IsPetSuccubus() idiom (confirmed against AshamaneCore) in IsWarlockPet()'s
-- checklist.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_warl_random_sayaad',
    'spell_warl_sayaad_precast_disorientation',
    'spell_warl_strengthen_pact_succubus',
    'spell_warl_strengthen_pact_incubus',
    'spell_warl_summon_sayaad'
);
