-- Druid: Activate Cat Form - Dash (1850) and Displacer Beast (102280) should auto-shift the
-- caster into Cat Form when used outside of any form, matching Prowl's already-working
-- behavior (spell_dru_prowl / spell_dru_base_transformer). Previously an open item in this
-- doc ("no ArgusCore equivalent by name, purpose/trigger not identified"); resolved by
-- confirming spell_dru_base_transformer is ArgusCore's own already-existing generic mechanism
-- for exactly this (already used by Prowl and both Incarnation talents), reused here rather
-- than porting AshamaneCore's separate one-off implementation.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (1850, 102280) AND `ScriptName` = 'spell_dru_activate_cat_form';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(1850, 'spell_dru_activate_cat_form'),
(102280, 'spell_dru_activate_cat_form');
