-- Demon Hunter/Havoc: Demonic Appetite (206478) was half-missing - the Fury refund on
-- consuming a fragment was already correctly gated by spell_dh_consume_soul, but nothing ever
-- actually spawned the fragment on a Chaos Strike/Annihilation crit. Confirmed via
-- DestinyCore/AshamaneCore (identical implementations). Bound directly to the talent's own
-- id (206478), matching how the passive already exists in ArgusCore as a bare HasAura check
-- with no class of its own.
DELETE FROM `spell_script_names` WHERE `spell_id` = 206478 AND `ScriptName` = 'spell_dh_demonic_appetite';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(206478, 'spell_dh_demonic_appetite');
