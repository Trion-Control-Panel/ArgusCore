-- DK Icy Talons (194878): the old spell_dk_icy_talons AuraScript hooked an aura type
-- (SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE) that doesn't exist on the real spell's data at all
-- (EFFECT_0 is a plain SPELL_EFFECT_DUMMY) - it could never fire regardless of its proc-check
-- logic, and that logic was also wrong for Legion (gated on any Runic Power-spending ability,
-- which is the Battle for Azeroth 8.0.1+ behavior; real Legion 7.3.5 Icy Talons is Frost Strike
-- specific, confirmed via its own tooltip text). Replaced with a check inside spell_dk_frost_strike
-- (already bound to 66196/222026, Frost Strike's real per-hit sub-spells), gated on
-- HasSpell(194878) and casting the real buff (194879) directly. Drop the old, now-nonexistent
-- script binding.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_icy_talons';
