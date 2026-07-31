-- Mage: Frost Bomb (112948 aura / 113092 detonation damage) - Frost talent, entirely missing
-- from ArgusCore. Detonates via Ice Lance hitting a frozen target (or with Fingers of Frost
-- up); explicit target takes more damage than splash targets. Confirmed genuine Legion
-- content (added pre-Legion, persisted through 7.3.5, removed 8.0.1) via Warcraft Wiki.
-- The 1.84275/1.15128 spell-power damage ratios are ported verbatim from the reference -
-- unlike most hardcoded-formula cases this project is cautious about, these match to 5
-- decimal places across two independently-written reference cores (DestinyCore and
-- AshamaneCore), treated as corroboration for genuine historical data rather than a shared
-- guess. The detonation trigger itself is wired into the existing spell_mage_ice_lance class
-- (no new binding needed for that part - only the damage effect below needs one).
DELETE FROM `spell_script_names` WHERE `spell_id` = 113092 AND `ScriptName` = 'spell_mage_frost_bomb_damage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(113092, 'spell_mage_frost_bomb_damage');
