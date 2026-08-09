-- Monk Legacy of the Emperor (spell_monk_legacy_of_the_emperor, bound to 115921): removed
-- entirely. Confirmed via patch history (not just id absence) that this ability predates Legion:
-- the primary-stat buff was replaced by Legacy of the White Tiger in patch 6.0.2 (Warlords of
-- Draenor), and both were removed in patch 7.0.3, before Legion launched. All three candidate ids
-- checked (115921, 117666, 117667 - the latter two are what DestinyCore/AshamaneCore still use for
-- this same ability, which is drift on their part, not evidence for this build) are confirmed
-- absent from real 7.3.5.26972 client data. Monks have no raid stat buff to replace this with in
-- Legion 7.3.5.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_legacy_of_the_emperor';
