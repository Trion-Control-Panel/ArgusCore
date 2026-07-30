-- Throwing Axes (200163) - Survival level 15 talent, entirely missing from ArgusCore.
-- Confirmed genuine Legion 7.3.5 content via community-guide patch history.
DELETE FROM `spell_script_names` WHERE `spell_id` = 200163;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(200163, 'spell_hun_throwing_axes');
