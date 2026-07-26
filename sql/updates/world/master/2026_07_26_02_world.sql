-- Bind the newly-implemented Monk Fists of Fury C++ spell scripts to their spell ids.
-- Four brand-new classes across three spell ids - no existing spell_script_names
-- row for any of them anywhere in this repo's SQL, so without these bindings
-- Fists of Fury would channel with no damage and an unending visual sweep aura.
--
-- Note: spell_monk_fists_of_fury_damage and spell_monk_fists_of_fury_visual_filter
-- are two independent script classes both bound to the same spell id (117418) -
-- this is intentional; TrinityCore-style script binding supports multiple script
-- names per spell id.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_fists_of_fury', 'spell_monk_fists_of_fury_damage', 'spell_monk_fists_of_fury_visual_filter', 'spell_monk_fists_of_fury_visual');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(113656, 'spell_monk_fists_of_fury'),                 -- Fists of Fury (channel)
(117418, 'spell_monk_fists_of_fury_damage'),          -- Fists of Fury (damage)
(117418, 'spell_monk_fists_of_fury_visual_filter'),   -- Fists of Fury (target filter)
(123154, 'spell_monk_fists_of_fury_visual');          -- Fists of Fury (visual sweep)
