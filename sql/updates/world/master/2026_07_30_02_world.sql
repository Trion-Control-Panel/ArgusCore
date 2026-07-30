-- Word of Glory (210191) - Legion's Retribution Holy Power heal talent.
-- Was entirely missing from ArgusCore (no constant, no class, no DB row of any kind).
-- Confirmed genuine Legion 7.0.3+ content via Warcraft Wiki patch history.
DELETE FROM `spell_script_names` WHERE `spell_id` = 210191;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(210191, 'spell_pal_word_of_glory');
