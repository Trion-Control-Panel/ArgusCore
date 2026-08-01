-- ArgusCore DB Error Cleanup (part 1)
-- Fixes confirmed HIGH/MEDIUM priority orphan-data errors from DBErrors.log.
-- Supersedes sql/pending/world/2026_05_30_00_world.sql, which drafted this same
-- pass but was never applied; its item 5 (battleground_scripts) is dropped here
-- because 2026_07_31_31_world.sql already fixed that bug correctly (UPDATE the
-- MapId to the real 489/529 instead of deleting the row - see that migration's
-- own comment for why DELETE would have orphaned working WSG/AB C++ code). All
-- other items below were re-verified still present in the current
-- logs/databaseErrors/DBErrors.log before being carried forward unchanged.

-- ============================================================
-- 1. spell_custom_attr — delete entries for non-existent spells
--    Error: "Table `spell_custom_attr` has wrong spell (entry: 48517/48518), ignored."
--    Spells 48517 and 48518 do not exist in this server's spell data.
-- ============================================================

DELETE FROM `spell_custom_attr` WHERE `entry` IN (48517, 48518);

-- ============================================================
-- 2. spell_custom_attr — delete entries for spells whose only attribute
--    is SPELL_ATTR0_CU_SHARE_DAMAGE (0x8) but have no SPELL_EFFECT_SCHOOL_DAMAGE.
--    Error: "Spell 66765/66809 listed in table `spell_custom_attr` with
--    SPELL_ATTR0_CU_SHARE_DAMAGE has no SPELL_EFFECT_SCHOOL_DAMAGE, ignored."
--    Confirmed: both entries have attributes = 8 only (TDB_world_735.26972_2025_05_11.sql).
--    The flag is invalid for these spells; no other custom attrs are set.
-- ============================================================

DELETE FROM `spell_custom_attr` WHERE `entry` IN (66765, 66809) AND `attributes` = 8;

-- ============================================================
-- 3. spell_proc — delete entries for spells that don't exist in this server's spell data.
--    Error: "The spell XXXXX listed in `spell_proc` does not exist"
-- ============================================================

DELETE FROM `spell_proc` WHERE `SpellId` IN (
    47515,   -- Divine Aegis (not present in this server's spell data)
    132158,
    135286,
    135288,
    255148,
    255150,
    255151,
    255800,
    258881,
    260895,
    265259
);

-- ============================================================
-- 4. transports — remove Seething Shore transport ships.
--    Error: "Table `transports` have transport (GUID: 35/36 Entry: 278407/279254)
--    with unknown gameobject `entry` set, skipped."
--    278407 (Sword of Dawn) and 279254 (The Warbringer) are BfA-only BG
--    transport ships with no GO template in this 7.3.5 DB.
-- ============================================================

-- Deleting by `entry` (not `guid`) because GUIDs were assigned via @TGUID variable
-- at insert time (sql/old/7/world/03_2025_05_11/2024_07_10_00_world.sql) and are not stable.
DELETE FROM `transports` WHERE `entry` IN (278407, 279254);

-- ============================================================
-- 5. command — remove entries for debug commands removed from code.
--    Error: "Table `command` contains data for non-existant command 'debug XXXX'. Skipped."
--    (No longer reproduces in the current DBErrors.log - the underlying rows may
--    already be gone. Left in as a harmless no-op guard in case they resurface.)
-- ============================================================

DELETE FROM `command` WHERE `name` IN (
    'debug getitemvalue',
    'debug getvalue',
    'debug Mod32Value',
    'debug setbit',
    'debug setitemvalue',
    'debug setvalue',
    'debug update'
);

-- ============================================================
-- 6. disables — remove phase 11595 which doesn't exist in this server's phase data.
--    Error: "Phase entry 11595 from `disables` doesn't exist in dbc, skipped."
--    sourceType 9 = DISABLE_TYPE_PHASE_AREA (DisableMgr.h)
-- ============================================================

DELETE FROM `disables` WHERE `sourceType` = 9 AND `entry` = 11595;
