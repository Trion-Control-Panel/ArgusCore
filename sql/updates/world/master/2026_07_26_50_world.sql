-- Replace the forward-drift 'spell_dk_ice_prison' binding on Chains of Ice (45524) with the
-- Legion-correct 'spell_dk_chilblains' script. Without this, Chains of Ice would either still
-- reference the removed ice_prison ScriptName (causing a "script not found" warning) or, before
-- that removal, would have double-rooted targets once chilblains was added alongside it.

DELETE FROM `spell_script_names` WHERE `spell_id` = 45524;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(45524, 'spell_dk_chilblains');
