-- Remove gameobject_template_addon rows whose base gameobject_template entry no longer
-- exists. These are already treated as dead data by ObjectMgr::LoadGameObjectTemplateAddons
-- (logged and skipped, never used) - this just removes the orphaned rows themselves.
--
-- Excludes the 29 entries restored by 2026_08_06_03_world.sql (their base gameobject_template
-- row was missing from this DB's dump, not the addon data - see that migration for detail).
DELETE a FROM `gameobject_template_addon` a
LEFT JOIN `gameobject_template` t ON a.`entry` = t.`entry`
WHERE t.`entry` IS NULL
AND a.`entry` NOT IN (244798,246387,246670,249382,250897,250898,251966,252188,259417,259418,
    259419,259420,259421,259422,259427,259431,259433,259434,259437,266959,272062,273661,
    266295,266576,266577,266592,266826,267190,268621);
