-- Unbind one more Demon Hunter forward-drift ScriptName missed by the
-- original 29-name cleanup (2026_07_26_32_world.sql): 'spell_dh_calcified_spikes'
-- (203819). It was never a class in spell_dh.cpp or in the DestinyCore
-- reference file - it's a stale row baked into the base TDB dump itself
-- (a later-expansion rename of Demon Spikes' damage-reduction talent,
-- confirmed via TDB_world_735.26972_2025_05_11.sql's own
-- spell_script_names table). Left unremoved it dangles alongside the
-- legitimate spell_dh_demon_spikes_buff binding also on 203819
-- (2026_07_26_41_world.sql), causing a "script not found" warning at
-- every startup.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_calcified_spikes';
