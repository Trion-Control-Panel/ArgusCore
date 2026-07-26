-- Hunter forward-drift audit: remove ten confirmed post-Legion features whose spell ids were
-- already flagged as suspiciously high (see ARGUSCORE_FIXES.md). Each was verified individually
-- against real WoW patch history rather than by id magnitude alone:
--   - Aspect of the Fox (1219162) - The War Within 11.1 Marksmanship redesign
--   - Emergency Salve (459517/459521) - The War Within 11.0 launch content
--   - Manhunter / Grievous Injury (1217788/1217789) - The War Within 11.1 Marksmanship revamp
--   - Implosive Trap (462032/462033) - a modern choice-node talent (post-Dragonflight tree system)
--   - Intimidation (Marksmanship variant, 474421) - The War Within 11.1 Marksmanship revamp
--   - Latent Poison + Latent Poison Injectors (378015/378016, 336903/336904) - Battle for Azeroth
--     Azerite trait
--   - Rapid Fire + Streamline + Surging Shots (257044/257045/263585, 260367/342076, 391559) -
--     Rapid Fire itself was introduced in Battle for Azeroth (8.0), after Legion; Streamline and
--     Surging Shots both only exist to modify Rapid Fire and can't predate it
--   - Rejuvenating Wind (385539/385540) - Dragonflight/The War Within pet-talent content
--   - Scout's Instincts (459455) - The War Within 11.0 content
--   - Wilderness Medicine (343242/384784) - confirmed Dragonflight 10.0 despite being bound to
--     the ancient, stable Mend Pet (136) spell id; the low id belongs to Mend Pet, not to this
--     talent's own era
-- All of these had a live spell_script_names (or, for Implosive Trap, areatrigger_create_properties)
-- row in the base TDB dump despite being confirmed non-Legion content - cleaning up the dangling
-- bindings alongside the C++ removal, same pattern as the earlier spell_dh_calcified_spikes and
-- Death Knight forward-drift cleanups this pass.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_hun_aspect_of_the_fox', 'spell_hun_emergency_salve', 'spell_hun_manhunter',
    'spell_hun_latent_poison_trigger', 'spell_hun_latent_poison_injectors_trigger',
    'spell_hun_rapid_fire', 'spell_hun_rapid_fire_damage', 'spell_hun_rejuvenating_wind',
    'spell_hun_scouts_instincts', 'spell_hun_surging_shots', 'spell_hun_wilderness_medicine'
);

UPDATE `areatrigger_create_properties` SET `ScriptName` = '' WHERE `ScriptName` = 'areatrigger_hun_implosive_trap';
