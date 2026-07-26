-- Bind the newly-implemented Demon Hunter Chaos Nova C++ spell script to
-- its spell id. No existing spell_script_names row for this ScriptName
-- anywhere in this repo's SQL, so without this binding Chaos Nova would
-- stun but deal no damage.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_chaos_nova';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(179057, 'spell_dh_chaos_nova'); -- Chaos Nova
