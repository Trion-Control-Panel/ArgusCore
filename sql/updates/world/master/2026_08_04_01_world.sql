-- spell_monk_zen_pilgrimage was split into two C++ classes (spell_monk_zen_pilgrimage for
-- 126892 - Zen Pilgrimage, and spell_monk_zen_pilgrimage_return for 126895 - Zen Pilgrimage:
-- Return) to stop a benign-but-noisy startup hook-mismatch warning caused by one shared class
-- registering hooks shaped for two different spells' effect data. Rebind 126895's row to the new
-- scriptname; 126892's row already correctly points at 'spell_monk_zen_pilgrimage' and needs no
-- change.
DELETE FROM `spell_script_names` WHERE `spell_id` = 126895 AND `ScriptName` = 'spell_monk_zen_pilgrimage';
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (126895, 'spell_monk_zen_pilgrimage_return');

-- spell_hun_barrage: bound to 120360 (Barrage, the periodic-trigger container - only 1 real
-- effect, a PERIODIC_TRIGGER_SPELL with no cone-area target at all), but the class's LOS-filtering
-- logic needs a TARGET_UNIT_CONE_ENEMY_24-shaped spell - that's 120361 (the damage tick 120360
-- casts repeatedly, confirmed via SpellEffect data: both of its effects have ImplicitTarget 24).
DELETE FROM `spell_script_names` WHERE `spell_id` = 120360 AND `ScriptName` = 'spell_hun_barrage';
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (120361, 'spell_hun_barrage');

-- spell_gen_clone: 49889/50218/51719 (Mirror Image family) each have EFFECT_0 =
-- SPELL_AURA_CLONE_CASTER - a dedicated native aura type the engine already handles generically,
-- with no SCRIPT_EFFECT anywhere on any of their effects. spell_gen_clone's whole mechanism is
-- SCRIPT_EFFECT-based, so it can never fire for these three regardless of index/EFFECT_ALL -
-- unbinding them removes the permanent no-op registration and its startup warning.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (49889, 50218, 51719) AND `ScriptName` = 'spell_gen_clone';

-- spell_warr_bloodthirst: bound to 215568 (Fresh Meat - both real effects are plain DUMMY
-- auras, no weapon-damage-type hit effect at all, so OnEffectHitTarget can never fire), but the
-- class's own header comment and logic (reads GetHitDamage() to trigger a % self-heal on hit)
-- both clearly describe the real Bloodthirst ability (23881, confirmed via SpellEffect data:
-- EFFECT_0 is NORMALIZED_WEAPON_DMG, matching the C++ hook already fixed to expect it).
DELETE FROM `spell_script_names` WHERE `spell_id` = 215568 AND `ScriptName` = 'spell_warr_bloodthirst';
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (23881, 'spell_warr_bloodthirst');

-- spell_jaws_of_death_claw_swipe_pct_damage: was bound to both 60776 (Claw Swipe) and 60864
-- (Jaws of Death). Real SpellEffect data (confirmed via wago.tools) shows 60864's EFFECT_0 is
-- SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT (matching this script's pct-of-max-health computation
-- exactly - the C++ hook was fixed to expect this instead of SPELL_EFFECT_SCHOOL_DAMAGE), but
-- 60776's real EFFECT_0 is plain SPELL_EFFECT_SCHOOL_DAMAGE with a flat EffectBasePoints (8) -
-- no pct calc needed, so it doesn't need this script at all and the base engine already applies
-- its flat damage natively. Unbind 60776 only; 60864 keeps the (now-corrected) script.
DELETE FROM `spell_script_names` WHERE `spell_id` = 60776 AND `ScriptName` = 'spell_jaws_of_death_claw_swipe_pct_damage';

-- battleground_template row 1014 has no matching BattlemasterList.dbc entry in this build
-- (confirmed via wago.tools: no row exists for BattlemasterList ID 1014 in 7.3.5.26972), and no
-- BattlegroundTypeId enum constant in the engine references 1014 either - it's an orphaned row
-- with no DBC backing and no code path, producing a harmless-but-noisy startup error
-- ("Battleground ID 1014 could not be found in BattlemasterList.dbc") every launch. Same category
-- as the earlier orphaned gameobject_template_addon cleanup - remove it.
DELETE FROM `battleground_template` WHERE `ID` = 1014;

-- spell_gen_feast: bound to 57337/57397/58466/58475/66477. Real SpellEffect data confirms EFFECT_0
-- is SPELL_EFFECT_SCRIPT_EFFECT (matching the hook) for the first four, but 66477 (Bountiful
-- Feast) no longer has a SCRIPT_EFFECT at all in this build - its three effects are plain
-- TRIGGER_SPELL (Food 175780, Drink 175787) and APPLY_AURA/PERIODIC_TRIGGER_SPELL (Well Fed buff
-- 65418), i.e. the same Food+Drink+WellFed combo this script's SPELL_BOUNTIFUL_FEAST case
-- manually casts, just already delivered natively now. Unbind 66477 only; the other four ids keep
-- the (unchanged, already-correct) script.
DELETE FROM `spell_script_names` WHERE `spell_id` = 66477 AND `ScriptName` = 'spell_gen_feast';
