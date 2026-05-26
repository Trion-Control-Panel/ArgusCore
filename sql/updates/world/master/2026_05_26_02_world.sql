-- spell_script_names: bind missing Fel Rush scripts

-- Main-cast script was renamed from spell_dh_fel_rush_main to spell_dh_fel_rush.
-- spell_dh_fel_rush_charge on 197922/197923 triggers the damage spell (192611).
-- 192611 needs no script binding: its AoE effect damages nearby enemies on its own.

-- Remove the stale main-cast binding (old name, class no longer exists)
DELETE FROM `spell_script_names` WHERE `spell_id` = 195072 AND `ScriptName` = 'spell_dh_fel_rush_main';

-- Idempotent cleanup
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dh_fel_rush',
    'spell_dh_fel_rush_charge'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
-- Main Fel Rush cast: StopMoving + picks ground vs air variant, casts it at forward destination
(195072, 'spell_dh_fel_rush'),
-- Ground charge (197922) and air/water charge (197923): after movement lands, cast the damage AoE
(197922, 'spell_dh_fel_rush_charge'),
(197923, 'spell_dh_fel_rush_charge');
