-- Beginning of the Arcane Mage implementation pass (previously almost entirely missing - no
-- Arcane Blast, Arcane Missiles, Arcane Barrier, or Touch of the Magi cast scripts existed).
-- Bind the first two newly-implemented pieces: Arcane Blast's Presence of Mind interaction and
-- Arcane Barrier's spell-power-scaled absorb bonus. No existing spell_script_names rows for
-- either ScriptName anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_mage_arcane_blast', 'spell_mage_arcane_barrier');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(30451, 'spell_mage_arcane_blast'),
(235450, 'spell_mage_arcane_barrier');
