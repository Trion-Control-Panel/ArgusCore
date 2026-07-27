-- Mage: bind newly-implemented Displacement (195676), and unbind three confirmed post-Legion
-- forward-drift scripts found incidentally while investigating Displacement/Blast Wave:
--   - Ethereal Blink + its triggered half (410939/410941) - a Dragonflight 10.1.0 PvP talent
--   - spell_mage_ice_block's Everwarm Socks branching (320913, a Shadowlands 9.0.2 Torghast
--     anima power) - removed only the anima-power-conditional logic; Ice Block itself (45438)
--     is untouched real Legion content and needs no script at all once the dead branching is
--     gone, so its ScriptName is cleared entirely rather than rebound
--   - Feel the Burn (383391) - a Dragonflight 10.0 Fire Mastery talent
-- Blast Wave's "Improved Flamestrike" bonus was also investigated and confirmed backward drift
-- (a pre-Legion point-based-talent-tree lookup with no equivalent in Legion's talent system) -
-- correctly left unimplemented, no SQL change needed since it was never bound.
-- See ARGUSCORE_FIXES.md for the full writeup.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_mage_displacement';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(195676, 'spell_mage_displacement');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_mage_ice_block';
