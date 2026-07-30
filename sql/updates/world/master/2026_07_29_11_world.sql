-- Shaman: removed the modern standalone Stormflurry talent bindings (344357, ScriptNames
-- "spell_sha_stormflurry_stormstrike"/"spell_sha_stormflurry_windstrike") - confirmed drift, no
-- Legion connection. Part of the ongoing Legion 7.3.5 forward-drift removal pass (see
-- ARGUSCORE_FIXES.md).
--
-- Stormflurry genuinely existed in Legion as the Doomhammer ARTIFACT TRAIT (198367, added patch
-- 7.0.3 2016-07-19, removed patch 8.0.1/BFA) - that binding
-- ("spell_sha_artifact_stormflurry_stormstrike"/"spell_sha_artifact_stormflurry_windstrike",
-- SPELL_SHAMAN_STORMFLURRY_ARTIFACT) is untouched, still correct, and still uses the shared
-- spell_sha_stormflurry class. The id removed here (344357) is the unrelated Shadowlands 9.0.1
-- (2020-10-13) standalone Enhancement talent that reused the same name/mechanic and replaced the
-- Landslide talent - it has no place in Legion 7.3.5.
--
-- Surgical removal: only the two registrations for the modern talent variant and the now-unused
-- SPELL_SHAMAN_STORMFLURRY constant were removed. The shared spell_sha_stormflurry class itself,
-- its two artifact-trait registrations, and the SPELL_SHAMAN_STORMFLURRY_ARTIFACT constant are
-- kept exactly as-is.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sha_stormflurry_stormstrike', 'spell_sha_stormflurry_windstrike');
