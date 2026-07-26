-- Rebind Death Knight's Soul Reaper from modern-retail spell ids to the real Legion-era ones,
-- and drop the "spell_dk_soul_reaper_reaper_of_souls" registration entirely - that ScriptName's
-- id (469180) is the modern War Within Hero Talent tree's "Reaper of Souls" ability, confirmed
-- forward drift (same family as spell_dk_reaper_of_souls, already removed).
--
-- Soul Reaper itself is real Legion content (both DestinyCore and AshamaneCore implement it;
-- self-marked "/// 6.x" by DestinyCore's own convention for old code needing review, and its
-- health-threshold logic references a Mists of Pandaria tier-set bonus that has no place in
-- Legion, but the ability and its spell ids predate and outlive that specific WoD-era wrapper).
-- The reference's own header comment lists three ids sharing one script -
-- "114866 - Soul Reaper, 130735 - Soul Reaper, 130736 - Soul Reaper" - almost certainly
-- per-spec copies of the same baseline ability. ArgusCore already independently uses 130736
-- elsewhere in this file (Dark Infusion's "does the target have Soul Reaper's debuff" check,
-- SPELL_DK_SOUL_REAPER_OLD_DEBUFF) - binding all three ids to the same script keeps that
-- existing check correct instead of orphaning it.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dk_soul_reaper', 'spell_dk_soul_reaper_reaper_of_souls'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(114866, 'spell_dk_soul_reaper'),
(130735, 'spell_dk_soul_reaper'),
(130736, 'spell_dk_soul_reaper');
