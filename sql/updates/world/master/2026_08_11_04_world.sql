-- See ARGUSCORE_FIXES.md > DB Startup Error Log Triage > Spell script hook/id triage (2026-08-11 Server.log batch)

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_gen_major_healing_cooldown_modifier_aura';
