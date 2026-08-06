-- spell_custom_attr entries 48517/48518 are WotLK-era "Eclipse (Solar)"/"Eclipse (Lunar)",
-- Balance Druid spells that predate Legion's Eclipse redesign and no longer exist in this
-- build's spell data (absent from Spell.csv for TDB 735.26972). Neither DestinyCore nor
-- AshamaneCore carries these rows in spell_custom_attr at all - both correctly omit them.
DELETE FROM `spell_custom_attr` WHERE `entry` IN (48517, 48518);
