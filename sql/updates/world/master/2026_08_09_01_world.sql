-- DK Soul Reaper (130736): reworked from the old WoD-era "periodic execute damage" design to the
-- real Legion 7.3.5 mechanic (immediate strike + Festering-Wound-burst-triggered stacking Haste,
-- confirmed via build-pinned 7.3.5.26972 client tooltip/effect data - see the comment left in
-- spell_dk.cpp at 130736's old class location for full sourcing). The real mechanic needs no
-- AuraScript on 130736 itself (its EFFECT_0 strike is native, EFFECT_1 is an inert marker aura
-- checked from spell_dk_apocalypse/spell_dk_scourge_strike instead), so the old binding to the
-- now-removed spell_dk_soul_reaper class is dropped entirely rather than repointed.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_soul_reaper';
