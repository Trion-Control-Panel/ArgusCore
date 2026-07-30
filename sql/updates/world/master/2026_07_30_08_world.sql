-- Rogue: Shadow Dance's own cast effect, Shadow Techniques, Weaponmaster, Between the Eyes'
-- CP-scaled duration, Grappling Hook, and Cannonball Barrage - all confirmed entirely missing
-- from ArgusCore during this session's gap-analysis re-audit.
--
-- 185313 (Shadow Dance) already has an existing row for 'spell_rog_shadow_focus' (a generic
-- cost-reduction class shared across all stealth-family spells, bound to EFFECT_1) - the new
-- spell_rog_shadow_dance script binds to the same spell's EFFECT_0 independently and does not
-- conflict with it.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (185313, 196912, 193537, 199804, 195457, 185767)
    AND `ScriptName` IN ('spell_rog_shadow_dance', 'spell_rog_shadow_techniques', 'spell_rog_weaponmaster',
        'spell_rog_between_the_eyes', 'spell_rog_grappling_hook', 'spell_rog_cannonball_barrage');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(185313, 'spell_rog_shadow_dance'),
(196912, 'spell_rog_shadow_techniques'),
(193537, 'spell_rog_weaponmaster'),
(199804, 'spell_rog_between_the_eyes'),
(195457, 'spell_rog_grappling_hook'),
(185767, 'spell_rog_cannonball_barrage');
