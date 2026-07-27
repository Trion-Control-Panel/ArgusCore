-- Mage: bind four more newly-implemented/newly-completed spells found while continuing the
-- class-diff sweep (Jouster, Jouster's damage-taken-reduction buff, the shared Ice Nova/Supernova
-- "Nova Talent" double-damage-vs-explicit-target script, and Water Elemental's Freeze which now
-- also grants the Fingers of Frost visual UI cue on top of its existing proc). No existing
-- spell_script_names rows for the Jouster/Nova Talent ScriptNames anywhere in this repo's SQL or
-- the base TDB dump; Water Elemental's Freeze (33395) was already bound to
-- spell_mage_water_elemental_freeze and only needed a code change, not a rebind.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_mage_jouster', 'spell_mage_jouster_buff', 'spell_mage_ice_nova', 'spell_mage_supernova'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(214626, 'spell_mage_jouster'),
(195391, 'spell_mage_jouster_buff'),
(157997, 'spell_mage_ice_nova'),
(157980, 'spell_mage_supernova');
