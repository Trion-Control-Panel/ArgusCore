-- Monk: Transcendence (101643) / Transcendence: Transfer (119996) - summons a spirit clone
-- whose position can be swapped with the caster's. Both halves entirely missing from
-- ArgusCore. Previously blocked in this doc on unconfirmed creature_template data for the
-- spirit (entry 54569); unblocked by redesigning around a safe runtime guard
-- (sObjectMgr->GetCreatureTemplate() checked in Validate(), matching the same pattern already
-- used for DK's Dancing Rune Weapon) instead of assuming the row exists. Also avoids
-- Object::Variables (used by the reference to cache the spirit's GUID, confirmed to not exist
-- anywhere in ArgusCore) by re-finding the spirit via Unit::m_Controlled on demand instead.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (101643, 119996) AND `ScriptName` IN ('spell_monk_transcendence', 'spell_monk_transcendence_transfer');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(101643, 'spell_monk_transcendence'),
(119996, 'spell_monk_transcendence_transfer');
