-- Rogue: rebind spell 193315 from the mislabeled 'spell_rog_sinister_strike' ScriptName to the
-- correct 'spell_rog_saber_slash' - the spell id itself doesn't change, but the class it maps to
-- was rewritten entirely (was carrying a dead Burning Crusade Tier 5 set-bonus check with no
-- relation to what 193315 actually is; the real ability, Saber Slash, is Outlaw's basic builder
-- and had no implementation at all before this fix). See ARGUSCORE_FIXES.md for the full writeup.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_rog_sinister_strike', 'spell_rog_saber_slash');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(193315, 'spell_rog_saber_slash');
