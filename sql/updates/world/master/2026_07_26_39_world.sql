-- Bind nine newly-implemented Demon Hunter PvP/talent C++ spell scripts to
-- their spell ids: Razor Spikes, Master of the Glaive, Bloodlet, Nether
-- Bond (+ its periodic tick), Vengeful Retreat's Prepared trigger (+ its
-- Fury refiller), Consume Soul, Burning Alive. None of these ScriptNames
-- have an existing spell_script_names row anywhere in this repo's SQL.
-- Consume Soul binds to three separate spell ids sharing one script.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dh_razor_spikes', 'spell_dh_master_of_the_glaive', 'spell_dh_bloodlet',
    'spell_dh_nether_bond', 'spell_dh_nether_bond_periodic', 'spell_dh_vengeful_retreat_trigger',
    'spell_dh_vengeful_retreat_fury_refiller', 'spell_dh_consume_soul', 'spell_dh_burning_alive'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(209400, 'spell_dh_razor_spikes'),                  -- Razor Spikes
(203556, 'spell_dh_master_of_the_glaive'),           -- Master of the Glaive
(206473, 'spell_dh_bloodlet'),                       -- Bloodlet
(207810, 'spell_dh_nether_bond'),                    -- Nether Bond
(207811, 'spell_dh_nether_bond_periodic'),           -- Nether Bond (periodic)
(198813, 'spell_dh_vengeful_retreat_trigger'),       -- Vengeful Retreat (Prepared trigger)
(203650, 'spell_dh_vengeful_retreat_fury_refiller'), -- Prepared (Fury refiller)
(178963, 'spell_dh_consume_soul'),                   -- Consume Soul
(203794, 'spell_dh_consume_soul'),                   -- Consume Soul
(228532, 'spell_dh_consume_soul'),                   -- Consume Soul
(207760, 'spell_dh_burning_alive');                  -- Burning Alive (Fiery Brand rank 2 spread)
