-- Warrior: second, deeper recheck pass against real Legion 7.3.5 client data.

-- Vigilance (50720/50725) was removed from the game entirely in patch 7.0.3, the Legion
-- pre-patch - it never existed at any point in this expansion. Confirmed absent from this build
-- under both ids. Class removed.
DELETE FROM `spell_script_names` WHERE `spell_id` = 50725 AND `ScriptName` = 'spell_warr_vigilance_trigger';
