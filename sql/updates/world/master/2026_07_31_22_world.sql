-- Warlock: Soul Conduit (215941/215942) - talent (all specs), each Soul Shard spent has a
-- chance to be refunded. Entirely missing from ArgusCore. Confirmed via DestinyCore/
-- AshamaneCore, but corrected a bug shared by both references: they check POWER_MANA instead
-- of POWER_SOUL_SHARDS when looking up how much was just spent - see in-code comment on
-- spell_warl_soul_conduit.
DELETE FROM `spell_script_names` WHERE `spell_id` = 215941 AND `ScriptName` = 'spell_warl_soul_conduit';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(215941, 'spell_warl_soul_conduit');
