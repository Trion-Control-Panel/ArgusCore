-- Mage: bind newly-implemented Unstable Magic (157976, legendary ring proc) - one shared
-- SpellScript bound under three separate ScriptNames to Arcane Blast (30451), Fireball (133), and
-- the Frostbolt damage trigger (228597), matching the existing one-class/multiple-ids pattern used
-- elsewhere in this file (e.g. spell_mage_nova_talent). No existing spell_script_names rows for
-- any of these ScriptNames anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_mage_unstable_magic_arcane_blast', 'spell_mage_unstable_magic_fireball',
    'spell_mage_unstable_magic_frostbolt'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(30451, 'spell_mage_unstable_magic_arcane_blast'),
(133, 'spell_mage_unstable_magic_fireball'),
(228597, 'spell_mage_unstable_magic_frostbolt');
