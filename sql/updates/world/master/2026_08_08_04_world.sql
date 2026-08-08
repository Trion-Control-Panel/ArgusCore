-- Monk: full recheck against real Legion 7.3.5 client data.

-- Jade Walk, Open Palm Strikes, Pressure Points, and Save Them All (spell_monk_jade_walk,
-- spell_monk_open_palm_strikes, spell_monk_pressure_points, spell_monk_save_them_all) removed
-- from spell_monk.cpp: all four ids bound below, and all four "target"/gating ids each class
-- referenced internally, are confirmed completely absent from this build (no Spell record at
-- all, local dump or live wago.tools query) - none of the four ability names exist under any id
-- in this build either. Numeric id range (389000-450000+) and web-search results both point to
-- current-retail-only content (Pressure Points specifically confirmed as a Paralysis redesign
-- from a much later expansion), not Legion 7.3.5. All four classes' own Validate() calls already
-- made them permanently inert regardless of binding (silently never loaded), so this was already
-- harmless dead code with a live DB row - cleaning up the row now that the C++ side is removed.
-- (spell_monk_pressure_points specifically upgrades an earlier "left unresolved" FIXME to a
-- confirmed removal, now that the same clustered pattern was independently found across all four.)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_monk_jade_walk',
    'spell_monk_open_palm_strikes',
    'spell_monk_pressure_points',
    'spell_monk_save_them_all'
);

-- Mana Tea (spell_monk_mana_tea/spell_monk_mana_tea_stacks) removed from spell_monk.cpp: despite
-- an earlier "confirmed via two independent reference sources" note, both references ported the
-- pre-Legion (MoP/WoD) "brew Chi into stacks, then channel them into mana" design. Web search
-- confirms Legion's own 7.0.3 redesign replaced this entirely with a simple "reduces mana cost of
-- spells by 50% for 10 sec" buff (real id 197908) - no stacks or channel at all. All four ids the
-- old design referenced (115294, 123766, 115867, 123760) are confirmed completely absent from
-- this build. The real Legion Mana Tea (197908) is a single native
-- SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT effect, already fully handled by the engine - no
-- spell_script_names row needed for it at all.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_monk_mana_tea',
    'spell_monk_mana_tea_stacks'
);

-- Two further bugs found and left flagged in spell_monk.cpp (not fixed this pass - no confident
-- real-id replacement found, matching this session's "don't guess" rule):
--
-- Legacy of the Emperor (spell_monk_legacy_of_the_emperor, bound to 115921): 115921 is confirmed
-- completely absent from this build. The only "Legacy of the Emperor" name match (125560) has a
-- mismatched effect structure (a single SPELL_EFFECT_CREATE_ITEM effect, not a stat-buff aura)
-- and an empty tooltip, so it isn't a confident replacement. The raid stat buff is currently
-- fully non-functional.
--
-- Ring of Peace (spell_monk_ring_of_peace_aura, bound to 140023): 140023 and both ids it casts
-- (137460/137461) are confirmed completely absent. Real Legion Ring of Peace is 116844 (confirmed
-- via name search + tooltip), but it's a completely different mechanic - a ground-targeted
-- AreaTrigger zone (SPELL_EFFECT_CREATE_AREATRIGGER, id 718) that ejects enemies who enter, not a
-- proc-driven silence+disarm. Needs a dedicated AreaTriggerAI (same category of gap as Druid's
-- Starfall from an earlier pass this session), not a hook/id fix to the existing class.
