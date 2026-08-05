-- spell_monk_surging_mist_glyphed was bound to 123273 ("Glyph of Surging Mist" from the old
-- pre-Legion Glyph system, confirmed absent from this build's Spell.db2 entirely - the Glyph
-- system doesn't exist in Legion). The C++ class has been removed; unbind the stale row.
DELETE FROM `spell_script_names` WHERE `spell_id` = 123273 AND `ScriptName` = 'spell_monk_surging_mist_glyphed';

-- spell_warr_sweeping_strikes was bound to 12328/18765/35429, an old Cata/MoP-era proc-based
-- design (its two triggered spells, 12723/26654, are confirmed absent from this build). Real
-- Legion Sweeping Strikes is a completely different, fully-native spell (202161, a passive
-- ChainTargets modifier via the engine's spell-mod system, no script needed) - so the old ids
-- just needed to be unbound, not rebound to 202161. The C++ class has been removed.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (12328, 18765, 35429) AND `ScriptName` = 'spell_warr_sweeping_strikes';

-- spell_dk_festering_wound (bound to 194311, the old "Soul Reaper pop grants a Shadow Infusion
-- stack" mechanic) and spell_dk_dark_transformation_form (bound to 63560, the old "consume the
-- stack" mechanic) both modeled WotLK/Cata-era Shadow Infusion design. Real Legion Shadow
-- Infusion is a completely different spell (198943, a flat Death-Coil-reduces-cooldown proc, not
-- a stacking mechanic) - the two old classes were removed and replaced with
-- spell_dk_shadow_infusion, bound to the real id instead.
DELETE FROM `spell_script_names` WHERE `spell_id` = 194311 AND `ScriptName` = 'spell_dk_festering_wound';
DELETE FROM `spell_script_names` WHERE `spell_id` = 63560 AND `ScriptName` = 'spell_dk_dark_transformation_form';
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (198943, 'spell_dk_shadow_infusion');
