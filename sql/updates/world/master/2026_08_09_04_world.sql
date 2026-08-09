-- Shaman Windfury Weapon: real 33757 (confirmed real across three separate 7.3.5 client builds -
-- 26365, 26654, 26972) is self-referential - the outer cast spell IS the resulting buff/proc aura,
-- a single EFFECT_0 SPELL_EFFECT_APPLY_AURA/SPELL_AURA_DUMMY targeting self. There is no separate
-- "enchantment" id; 319773 and 334302 (formerly used here) are both confirmed absent across the
-- same three builds. Removed spell_sha_windfury_weapon entirely (its cast-onto-weapon-item +
-- PreventHitDefaultEffect premise was wrong for this build; native APPLY_AURA now applies on its
-- own) and rebound spell_sha_windfury_weapon_proc from the dead 319773 to the real 33757.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sha_windfury_weapon', 'spell_sha_windfury_weapon_proc');
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(33757, 'spell_sha_windfury_weapon_proc');
