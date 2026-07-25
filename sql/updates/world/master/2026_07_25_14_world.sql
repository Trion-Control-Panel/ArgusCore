-- Bind the newly-implemented Warrior Shattering Throw C++ spell script to its spell ids.
-- spell_warr_shattering_throw is a brand-new class bound to both 64380 and 65941
-- (two historically-registered variants of this ability, confirmed via Wowhead) -
-- no existing spell_script_names row for either anywhere in this repo's SQL, so
-- without this binding Shattering Throw would deal its direct damage but never
-- remove the target's immunity shield.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_shattering_throw';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(64380, 'spell_warr_shattering_throw'),
(65941, 'spell_warr_shattering_throw');
