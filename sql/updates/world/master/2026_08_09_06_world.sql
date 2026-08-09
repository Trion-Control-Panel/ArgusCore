-- Paladin Infusion of Light (54149): removed spell_pal_infusion_of_light entirely. Real 54149 has
-- exactly 2 effects (both classmask-restricted SpellMods matching its own tooltip word for word),
-- auto-applied/consumed natively by the engine's generic SpellMod system - no script needed for
-- the buff's own consumption. The removed class's "energize" target (356717) is confirmed
-- post-Legion (added patch 9.1.0, an unrelated Holy Power mechanic). What actually grants 54149
-- (a Holy Shock/Holy Radiance critical heal) is still unimplemented and needs its own dedicated
-- investigation - not fixed by this migration.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_pal_infusion_of_light';
