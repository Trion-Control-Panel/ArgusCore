-- Aura of Sacrifice (183416) - Holy level 60 talent, entirely missing from ArgusCore.
-- Implemented as a periodic raid-range scan (spell_pal_aura_of_sacrifice) applying a
-- redirect-absorb buff (spell_pal_aura_of_sacrifice_ally, 210372) to nearby raid members,
-- rather than the reference cores' persistent AreaTrigger with a made-up custom AT id - avoids
-- inventing a non-Blizzard AreaTrigger id/SQL row.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (183416, 210372);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(183416, 'spell_pal_aura_of_sacrifice'),
(210372, 'spell_pal_aura_of_sacrifice_ally');
