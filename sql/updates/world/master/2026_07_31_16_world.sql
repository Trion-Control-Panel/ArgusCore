-- Demon Hunter/Vengeance: the entire builder-spender Soul Fragment loop was non-functional -
-- Shear (203782) generated no resource at all, Fracture (209795) was entirely missing, and
-- Spirit Bomb (247454/247455) plus its Frailty debuff (224509) were declared but never
-- implemented. Confirmed genuine and high-severity via DestinyCore/AshamaneCore (Shear,
-- Fracture, Spirit Bomb count-then-multi-cast shape) and LegionCore-7.3.5V2 (Spirit Bomb
-- count-then-scale shape, corroborating a third, structurally independent way).
--
-- Also corrected ArgusCore's own pre-existing SPELL_DH_SPIRIT_BOMB_DAMAGE constant (was 218677,
-- never referenced anywhere and thus unverified) to 247455, the id both DestinyCore and
-- AshamaneCore agree on exactly; SPELL_DH_SPIRIT_BOMB_HEAL (227255) was already correct.
--
-- See in-code comments on spell_dh_spirit_bomb/spell_dh_spirit_bomb_damage for why the
-- consumed-fragment count is carried between the two casts via the caster's own
-- SPELL_DH_SOUL_FRAGMENT_COUNTER (203981) stack amount rather than a SPELLVALUE_BASE_POINT0
-- override, which would have collided with the damage spell's own EFFECT_0.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (203782, 209795, 247454, 247455, 224509)
    AND `ScriptName` IN ('spell_dh_shear', 'spell_dh_fracture', 'spell_dh_spirit_bomb', 'spell_dh_spirit_bomb_damage', 'aura_dh_frailty');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(203782, 'spell_dh_shear'),
(209795, 'spell_dh_fracture'),
(247454, 'spell_dh_spirit_bomb'),
(247455, 'spell_dh_spirit_bomb_damage'),
(224509, 'aura_dh_frailty');
