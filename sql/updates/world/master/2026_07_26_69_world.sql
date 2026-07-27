-- Implement Warrior's Ravager (Fury/Protection talent, 152277) - was previously only tracked as
-- a cooldown id inside Anger Management's spec arrays, never actually implemented as a castable
-- ability. No existing spell_script_names row for this ScriptName anywhere in this repo's SQL or
-- the base TDB dump.
--
-- creature_template entry 76168 ("Ravager") already exists in the base TDB dump with an empty
-- ScriptName - same "row exists, never bound" shape as the Priest AreaTriggers fixed earlier this
-- pass, confirmed by cross-referencing DestinyCore's own creature_template dump (which has the
-- identical entry already bound to 'npc_warr_ravager').
--
-- Also corrected two related bugs found while implementing this: Anger Management's Fury array
-- was reducing the cooldown of Arms' Ravager (228920) instead of Fury's own Ravager (152277) -
-- a pure C++ constant fix, no SQL involved - and Protection's array was missing Ravager (152277)
-- entirely despite Protection also having access to the talent.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_ravager_fury_prot';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(152277, 'spell_warr_ravager_fury_prot');

UPDATE `creature_template` SET `ScriptName` = 'npc_warr_ravager' WHERE `entry` = 76168;
