-- Bind the newly-implemented Monk Blackout Kick C++ spell script to its spell id.
-- spell_monk_blackout_kick is a brand-new class bound to spell 100784 (confirmed
-- via a reference implementation's own committed spell_script_names data) - no existing
-- spell_script_names row for it anywhere in this repo's SQL, so without this
-- binding Blackout Kick would never consume Teachings of the Monastery stacks,
-- reset Rising Sun Kick, or refund mana via Spirit of the Crane.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_blackout_kick';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(100784, 'spell_monk_blackout_kick'); -- Blackout Kick
