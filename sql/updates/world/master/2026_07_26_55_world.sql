-- Bind the newly-implemented Death Knight Vampiric Aura (Artifact Power trait) C++ script to
-- its spell id. No existing spell_script_names row for this ScriptName anywhere in this repo's
-- SQL or the base TDB dump, so without this binding the raid-wide cast would hit hostile
-- targets instead of being filtered to the caster plus up to 5 allies.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_vampiric_aura';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(238698, 'spell_dk_vampiric_aura');
