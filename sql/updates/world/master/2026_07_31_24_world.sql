-- Warlock: Soul Leech (228974/108366) - baseline passive (all specs), dealing damage grants
-- an absorb shield scaled by damage dealt, capped at 15% max health. Entirely missing from
-- ArgusCore. Confirmed via DestinyCore and AshamaneCore (identical implementations),
-- simplified to skip the Demonskin talent's cap increase (Demonskin itself not yet
-- implemented). Deliberately did not port the references' separate pre-Legion "Soul Leach
-- appliers" system (137046/137044/137043) - unrelated drift under a similar name.
DELETE FROM `spell_script_names` WHERE `spell_id` = 228974 AND `ScriptName` = 'spell_warl_soul_leech';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(228974, 'spell_warl_soul_leech');
