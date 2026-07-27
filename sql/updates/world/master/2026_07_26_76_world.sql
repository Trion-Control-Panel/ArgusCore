-- Continuing the Arcane Mage implementation pass: bind Arcane Missiles' four newly-implemented
-- scripts (the channel's per-missile visual/power handling, its damage sub-effect's self-target
-- safety filter, Clearcasting's anti-feedback-loop proc gate, and the charges resource pool that
-- drives the stack-count visual). No existing spell_script_names rows for any of these
-- ScriptNames anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_mage_arcane_missiles', 'spell_mage_arcane_missiles_damage',
    'spell_mage_arcane_missiles_proc', 'spell_mage_arcane_missiles_charges'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5143, 'spell_mage_arcane_missiles'),
(7268, 'spell_mage_arcane_missiles_damage'),
(79684, 'spell_mage_arcane_missiles_proc'),
(79683, 'spell_mage_arcane_missiles_charges');
