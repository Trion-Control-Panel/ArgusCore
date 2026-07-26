-- Bind eight newly-implemented Hunter utility/proc-gate C++ spell scripts
-- to their spell ids: Bombardment, Camouflage, Farstrider, Lock and Load,
-- Ranger's Net, Sentinel, Sticky Bomb, Wild Call. None of these
-- ScriptNames have an existing spell_script_names row anywhere in this
-- repo's SQL.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_hun_bombardment', 'spell_hun_camouflage', 'spell_hun_farstrider', 'spell_hun_lock_and_load',
    'spell_hun_rangers_net', 'spell_hun_sentinel', 'spell_hun_sticky_bomb', 'spell_hun_wild_call'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(35110,  'spell_hun_bombardment'),   -- Bombardment
(199483, 'spell_hun_camouflage'),    -- Camouflage
(199523, 'spell_hun_farstrider'),    -- Farstrider
(194595, 'spell_hun_lock_and_load'), -- Lock and Load
(200108, 'spell_hun_rangers_net'),   -- Ranger's Net
(206817, 'spell_hun_sentinel'),      -- Sentinel
(191241, 'spell_hun_sticky_bomb'),   -- Sticky Bomb
(185789, 'spell_hun_wild_call');     -- Wild Call
