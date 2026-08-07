-- Demon Hunter: full recheck against real Legion 7.3.5 client data.

-- Immolation Aura was bound to 258920, confirmed completely absent from this build (no Spell
-- record at all) - this core, iconic ability has been fully non-functional. Real ids are 178740
-- (Havoc) and 219830 (Vengeance), both structurally identical and confirmed via SpellEffect.db2;
-- the existing script's EFFECT_1/TRIGGER_SPELL hook already matches both real ids exactly, no
-- other code change needed beyond the id fix.
DELETE FROM `spell_script_names` WHERE `spell_id` = 258920 AND `ScriptName` = 'spell_dh_immolation_aura';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(178740, 'spell_dh_immolation_aura'),
(219830, 'spell_dh_immolation_aura');

-- spell_dh_vengeful_retreat_fury_refiller was already found to be fully native (real 203650
-- EFFECT_0 is a plain MOD_POWER_REGEN, already handled by AuraEffect::HandleModPowerRegen) and
-- removed in an earlier pass, but the spell_script_names row was never cleaned up.
DELETE FROM `spell_script_names` WHERE `spell_id` = 203650 AND `ScriptName` = 'spell_dh_vengeful_retreat_fury_refiller';
