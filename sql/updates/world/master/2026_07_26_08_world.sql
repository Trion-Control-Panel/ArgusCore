-- Bind the newly-implemented Monk Mastery: Combo Strikes C++ spell scripts to
-- their spell ids. Three brand-new classes across ten spell ids - no existing
-- spell_script_names row for any of these three script names anywhere in this
-- repo's SQL, so without these bindings Windwalker's core mastery would never
-- apply its damage bonus.
--
-- Several of these spell ids already have OTHER scripts bound from earlier
-- fixes this session (100784/Blackout Kick, 113656/Fists of Fury,
-- 117418/Fists of Fury Damage) - this migration only touches the three new
-- script names below and does not affect those existing bindings, since
-- TrinityCore-style script binding supports multiple independent script
-- names per spell id (already relied upon for Fists of Fury's own dual
-- binding earlier).

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_monk_mastery_combo_strikes', 'spell_monk_mastery_combo_strikes_periodic_auras', 'spell_monk_mastery_combo_strikes_periodic_triggers');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(100780, 'spell_monk_mastery_combo_strikes'), -- Tiger Palm
(100784, 'spell_monk_mastery_combo_strikes'), -- Blackout Kick
(123586, 'spell_monk_mastery_combo_strikes'), -- Flying Serpent Kick (AoE)
(185099, 'spell_monk_mastery_combo_strikes'), -- Rising Sun Kick (secondary id)
(113656, 'spell_monk_mastery_combo_strikes_periodic_auras'), -- Fists of Fury
(101546, 'spell_monk_mastery_combo_strikes_periodic_auras'), -- Spinning Crane Kick
(152175, 'spell_monk_mastery_combo_strikes_periodic_auras'), -- Whirling Dragon Punch
(117418, 'spell_monk_mastery_combo_strikes_periodic_triggers'), -- Fists of Fury (damage)
(107270, 'spell_monk_mastery_combo_strikes_periodic_triggers'), -- Spinning Crane Kick (damage)
(158221, 'spell_monk_mastery_combo_strikes_periodic_triggers'); -- Whirling Dragon Punch (damage)
