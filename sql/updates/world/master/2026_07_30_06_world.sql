-- Odyn's Fury (214871), Heavy Repercussions (203177), Frothing Berserker (215571) -
-- all entirely missing from ArgusCore. Confirmed genuine Legion 7.3.5 content via
-- Warcraft Wiki/community-guide patch history cross-checks.
--
-- NOTE: Devastate's chance-based Shield Slam cooldown reset was investigated and deliberately
-- NOT implemented - it was removed in patch 7.1.5, well before 7.3.5 (ArgusCore's target
-- patch), so it is correctly absent already. Opportunity Strikes (level 100 Arms talent) was
-- also investigated but left unimplemented - the paired "extra attack" trigger spell id could
-- not be confirmed from available sources without risking a wrong guess.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (214871, 203177, 215571);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(214871, 'spell_warr_odyns_fury'),
(203177, 'spell_warr_heavy_repercussions'),
(215571, 'spell_warr_frothing_berserker');
