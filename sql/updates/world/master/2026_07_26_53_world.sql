-- Unbind one more Death Knight forward-drift ScriptName missed by the earlier six-ability
-- cleanup (2026_07_26_49_world.sql): 'spell_dk_death_and_decay_increase_targets' (188290).
-- It was never its own top-level C++ class - it only existed as a cast target inside the
-- removed at_dk_death_and_decay AreaTrigger's Cleaving Strikes interaction (confirmed forward
-- drift, spell id 316916, well beyond Legion's ceiling). Left unremoved it dangles as a
-- "script not found" warning at every startup. Confirmed via LegionCore-7.3.5-merged that
-- spell 188290 itself ("Death and Decay" aura) is real Legion content, checked elsewhere via
-- plain HasAura() with no script needed - only this specific ScriptName binding is stale.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_death_and_decay_increase_targets';
