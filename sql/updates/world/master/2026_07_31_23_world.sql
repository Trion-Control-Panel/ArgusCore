-- Warlock/Demonology: Demonic Calling (205145/205146) - Shadow Bolt/Demonbolt hits have a
-- chance to make the next Call Dreadstalkers free/instant. Entirely missing from ArgusCore.
-- Confirmed via DestinyCore/AshamaneCore (identical implementations) - deliberately did not
-- port both references' second, apparently-erroneous copy of this same check inside their
-- unrelated Demonwrath class (leftover/copy-paste drift, not a genuine second trigger).
DELETE FROM `spell_script_names` WHERE `spell_id` = 205145 AND `ScriptName` = 'spell_warl_demonic_calling';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(205145, 'spell_warl_demonic_calling');
