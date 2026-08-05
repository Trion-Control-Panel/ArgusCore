-- spell_monk_surging_mist_glyphed was bound to 123273 ("Glyph of Surging Mist" from the old
-- pre-Legion Glyph system, confirmed absent from this build's Spell.db2 entirely - the Glyph
-- system doesn't exist in Legion). The C++ class has been removed; unbind the stale row.
DELETE FROM `spell_script_names` WHERE `spell_id` = 123273 AND `ScriptName` = 'spell_monk_surging_mist_glyphed';
