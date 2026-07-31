-- Demon Hunter/Havoc: Fel Barrage (211053/211052) and its Fel Mastery charge-builder passive
-- (222703) were entirely non-functional - all 3 constants were declared but never referenced.
-- Confirmed via DestinyCore/AshamaneCore (identical implementations). See in-code comments on
-- spell_dh_fel_barrage/spell_dh_fel_barrage_damage/spell_dh_fel_barrage_proc for the API
-- translations (no direct "current charge count" getter in ArgusCore's SpellHistory; the
-- damage spell's charge-count payload is carried via GetSpellValue()->EffectBasePoints[1]
-- rather than EFFECT_0, which is the damage effect itself).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (211053, 211052, 222703)
    AND `ScriptName` IN ('spell_dh_fel_barrage', 'spell_dh_fel_barrage_damage', 'spell_dh_fel_barrage_proc');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(211053, 'spell_dh_fel_barrage'),
(211052, 'spell_dh_fel_barrage_damage'),
(222703, 'spell_dh_fel_barrage_proc');
