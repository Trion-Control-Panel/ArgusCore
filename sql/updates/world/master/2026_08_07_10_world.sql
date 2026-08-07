-- Death Knight: Blinding Sleet and Blooddrinker were already found to be fully native (real
-- SpellEffect data covers both mechanics with no gap) and their C++ classes removed in an earlier
-- pass, but the spell_script_names rows binding them to now-nonexistent classes were never
-- cleaned up.
DELETE FROM `spell_script_names` WHERE `spell_id` = 207167 AND `ScriptName` = 'spell_dk_blinding_sleet';
DELETE FROM `spell_script_names` WHERE `spell_id` = 206931 AND `ScriptName` = 'spell_dk_blooddrinker';

-- Blood Charge/Blood Tap (114851/45529): both ids confirmed completely absent from this build
-- (no Spell record, no SpellEffect record, no local dump entry) - overrides an earlier
-- [IMPLEMENTED] conclusion based on Warcraft Wiki patch history, which doesn't hold up against
-- this build's actual client data. Classes removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 45529 AND `ScriptName` = 'spell_dk_blood_charge';
DELETE FROM `spell_script_names` WHERE `spell_id` = 45529 AND `ScriptName` = 'spell_dk_blood_tap';

-- Obliteration (207256) is fully native (real EFFECT_0/EFFECT_1 already cover its whole tooltip);
-- class was never bound live, nothing to unbind. Just documenting alongside the rest of this pass.

-- Death Siphon (108196) was removed in patch 7.0.3, the Legion pre-patch - never existed in this
-- expansion. Confirmed absent from this build under any id. Class removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 108196 AND `ScriptName` = 'spell_dk_death_siphon';

-- Improved Death Strike (374277) is Dragonflight 10.0.0 content, confirmed absent from this build.
-- Class removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 374277 AND `ScriptName` = 'spell_dk_improved_death_strike';

-- Death's Caress (195292) applying Bone Shield stacks is a later Blood DK rework - real 195292 in
-- this build only deals Shadow damage and applies Blood Plague (confirmed via its own tooltip),
-- no Bone Shield component at all. The bound scriptname has no matching C++ registration either
-- way. Marrowrend (195182), the genuine Legion Bone Shield source, is untouched.
DELETE FROM `spell_script_names` WHERE `spell_id` = 195292 AND `ScriptName` = 'spell_dk_deaths_caress_apply_bone_shield';

-- Runic Empowerment (81229) and Runic Corruption (51460): neither id has any proc-eligibility
-- data anywhere (this build's own SpellAuraOptions.db2, or either reference core's spell_proc
-- table), so the AuraScript-proc versions of these classes could never fire regardless of
-- binding or hook correctness. Rewritten as a single PlayerScript::OnSpellCast handler instead
-- (playerScript_dk_runic_power_procs), which needs no spell_script_names row at all - no SQL
-- change needed for these two.
