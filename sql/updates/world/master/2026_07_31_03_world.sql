-- Death Knight: Dark Transformation (63560) - Unholy pet cooldown ability, entirely missing
-- from ArgusCore. Consumes a Dark Infusion stack from both player and pet on hit (granted by
-- Festering Wound popping a Soul-Reaper-marked target, spell_dk_festering_wound).
--
-- Bug fix folded in: the pre-existing SPELL_DK_DARK_INFUSION constant in spell_dk.cpp held id
-- 215711, which does not exist as a real spell (confirmed 404 on Wowhead) - this meant
-- spell_dk_festering_wound's Soul Reaper synergy was silently casting a nonexistent spell and
-- doing nothing. Corrected to 91342, corroborated by both DestinyCore and AshamaneCore's
-- independently-written spell_dk_dark_transformation_form, which both consume exactly this id
-- from player and pet. This is a pre-existing bug (not introduced this session) caught while
-- implementing Dark Transformation itself.
DELETE FROM `spell_script_names` WHERE `spell_id` = 63560 AND `ScriptName` = 'spell_dk_dark_transformation_form';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(63560, 'spell_dk_dark_transformation_form');
