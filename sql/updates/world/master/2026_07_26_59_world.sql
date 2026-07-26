-- Bind two newly-implemented Warrior C++ scripts to their spell ids: Last Stand (Protection's
-- temporary-max-health defensive cooldown) and Vigilance's Taunt-cooldown-reset trigger. Neither
-- ScriptName has an existing spell_script_names row anywhere in this repo's SQL or the base TDB
-- dump. Also corrects a pre-existing bug in SPELL_WARRIOR_RALLYING_CRY, which was set to 97463
-- instead of the correct 97462 (confirmed via the base TDB dump's own spell_script_names row for
-- 'spell_warr_rallying_cry') - no SQL change needed for that fix since the database binding was
-- already correct; only the C++ constant was wrong.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_warr_last_stand', 'spell_warr_vigilance_trigger');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(12975, 'spell_warr_last_stand'),        -- Last Stand
(50725, 'spell_warr_vigilance_trigger'); -- Vigilance (Taunt cooldown reset)
