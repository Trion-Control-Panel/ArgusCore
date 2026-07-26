-- Bind the newly-implemented Monk Touch of Death and Touch of Karma C++ spell
-- scripts to their spell ids. Both are brand-new classes - no existing
-- spell_script_names row for either anywhere in this repo's SQL, so without
-- these bindings Touch of Death would deal no damage and Touch of Karma
-- would provide no absorb or damage redirect.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_touch_of_death', 'spell_monk_touch_of_karma');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(115080, 'spell_monk_touch_of_death'), -- Touch of Death
(122470, 'spell_monk_touch_of_karma'); -- Touch of Karma
