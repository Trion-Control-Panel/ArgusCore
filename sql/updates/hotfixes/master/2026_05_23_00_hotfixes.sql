-- Revert: Fury Warrior Whirlwind (190411) has no Rage cost by design in Legion 7.3.5.
-- It applies the Whirlwind buff for Rampage cleave and gives rage on multi-target hit.
-- Remove any erroneously added spell_power entry for this spell.
DELETE FROM `spell_power` WHERE `SpellID` = 190411;
