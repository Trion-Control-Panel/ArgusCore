-- Bind five newly-implemented Priest/Shadow C++ scripts to their spell ids: Shadowy
-- Apparitions, Void Eruption, Voidform (buffs/periodic driver), Void Bolt, and Void Tendrils.
-- This is Shadow's entire Legion-defining Voidform system - none of it existed in ArgusCore at
-- all before this fix, and none of these ScriptNames have an existing spell_script_names row
-- anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_pri_shadowy_apparitions', 'spell_pri_void_eruption', 'spell_pri_voidform',
    'spell_pri_void_bolt', 'spell_pri_void_tendrils'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(78203, 'spell_pri_shadowy_apparitions'), -- Shadowy Apparitions
(228260, 'spell_pri_void_eruption'),      -- Void Eruption
(194249, 'spell_pri_voidform'),           -- Voidform (buffs/periodic driver)
(234746, 'spell_pri_void_bolt'),          -- Void Bolt
(108920, 'spell_pri_void_tendrils');      -- Void Tendrils
