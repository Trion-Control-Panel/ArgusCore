-- Warlock: Demonskin (219272, Affliction/Destruction PvP Honor Talent) - periodically tops up
-- the Soul Leech absorb shield, capped at a higher threshold than the baseline. Entirely
-- missing from ArgusCore. Confirmed genuine Legion content throughout 7.3.5 (removed patch
-- 9.2.5) via Warcraft Wiki. Confirmed via DestinyCore and AshamaneCore (identical
-- implementations).
DELETE FROM `spell_script_names` WHERE `spell_id` = 219272 AND `ScriptName` = 'spell_warl_demonskin';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(219272, 'spell_warl_demonskin');
