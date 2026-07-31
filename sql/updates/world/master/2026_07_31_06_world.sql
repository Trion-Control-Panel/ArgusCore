-- Death Knight: Glacial Advance (194913) - Frost's level-100 AOE talent, entirely missing
-- from ArgusCore. Previously logged as blocked (both DestinyCore and AshamaneCore schedule
-- its staggered line-of-ice-spikes casts via Unit::GetScheduler(), which does not exist in
-- ArgusCore) - unblocked using the same Unit::m_Events/AddEventAtOffset lambda idiom already
-- established for Hunter's Throwing Axes and spell_mage_flurry. See in-code comment on
-- spell_dk_glacial_advance for the GetFirstCollisionPosition/MovePosition angle-argument and
-- CalcRadius translation details.
DELETE FROM `spell_script_names` WHERE `spell_id` = 194913 AND `ScriptName` = 'spell_dk_glacial_advance';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(194913, 'spell_dk_glacial_advance');
