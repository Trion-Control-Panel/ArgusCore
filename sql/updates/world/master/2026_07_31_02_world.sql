-- Plea (200829) - Discipline level-24 ability, one of the spells that applies Atonement in
-- Legion's Discipline redesign (alongside Power Word: Shield and Shadow Mend, both already
-- bound). The earlier investigation of this gap recorded the wrong id (200292, which does
-- not exist / 404s on Wowhead) - confirmed the correct id is 200829 before writing this.
-- No C++ change needed: spell_pri_atonement_effect is already a generic, database-driven
-- binding (RegisterSpellScript, not RegisterSpellScriptWithArgs).
DELETE FROM `spell_script_names` WHERE `spell_id` = 200829 AND `ScriptName` = 'spell_pri_atonement_effect';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(200829, 'spell_pri_atonement_effect');
