-- Bind seven newly-implemented Warrior C++ spell scripts to their spell ids.
-- All seven are brand-new classes - no existing spell_script_names row for any
-- of them anywhere in this repo's SQL, so without these bindings none would fire.
--
-- Note on spell_warr_inspiring_presence: the outer passive this script binds to is
-- 205484, confirmed via a reference implementation's own committed spell_script_names
-- data - this is DIFFERENT from SPELL_WARRIOR_INSPIRING_PRESENCE (222944), which is
-- the inner heal spell the script casts when it procs, not the id it's registered under.
--
-- Note on spell_warr_precise_strikes: its spell id (248579) comes from a reference
-- implementation's own source comment ("// 248579 - Precise Strikes / 7.3.5"), not an
-- independently confirmed spell_script_names row - that reference's own DB dump has no
-- binding for this script name at all, so this one carries slightly lower confidence
-- than the other six.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warr_defensive_stance', 'spell_warr_focused_rage_arms', 'spell_warr_inspiring_presence', 'spell_warr_precise_strikes', 'spell_warr_safeguard', 'spell_warr_soul_of_the_slaughter', 'spell_warr_wrecking_ball_effect');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(71, 'spell_warr_defensive_stance'),
(207982, 'spell_warr_focused_rage_arms'),
(205484, 'spell_warr_inspiring_presence'),
(248579, 'spell_warr_precise_strikes'),
(223657, 'spell_warr_safeguard'),
(238111, 'spell_warr_soul_of_the_slaughter'),
(215570, 'spell_warr_wrecking_ball_effect');
