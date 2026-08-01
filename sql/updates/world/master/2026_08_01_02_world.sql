-- ArgusCore DB Error Cleanup (part 2) — bulk generic referential noise from the
-- TDB community dump, triaged against logs/databaseErrors/DBErrors.log. See
-- ARGUSCORE_FIXES.md, "DB Startup Error Log Triage" section, for the full
-- category breakdown and per-item verdicts this migration implements.
--
-- Scope is deliberately narrow: only categories where (a) the orphaned side is
-- unambiguously the row to remove, not the referenced side, and (b) the
-- affected ID range was confirmed non-Legion by sampling. Categories where the
-- "fix" would actually be to add missing data (loot template stubs, etc.), or
-- where the ID range overlaps real Legion content (spawn_tracking_template
-- PhaseId, quest RewardSpellCast, spell_proc, creature equipment_id), are
-- intentionally NOT included here — see the doc's "needs spot-check" items.

-- ============================================================
-- 1. quest_poi — delete rows for quest ids that don't exist in quest_template.
--    Error: "`quest_poi` quest id (N) IdxN (N) does not exist in `quest_template`"
--    (ObjectMgr.cpp:8437). 5300 log lines, 1753 unique quest ids spanning
--    WoD-tail through Legion range. This does NOT fix the underlying
--    quest_template coverage gap (those 1753 ids are still absent from
--    quest_template itself) - that's a bulk content-population effort, out of
--    scope here. This only removes the now-meaningless POI markers pointing at
--    quests that don't exist in this DB at all.
-- ============================================================

DELETE FROM `quest_poi` WHERE `QuestID` NOT IN (SELECT `ID` FROM `quest_template`);

-- ============================================================
-- 2. spawn_tracking_template — delete rows referencing maps that don't exist
--    in this server's map data at all.
--    Error: "Table `spawn_tracking_template` references non-existing map N, skipped"
--    (ObjectMgr.cpp:11358). All 22 affected map ids (1642-2792) confirmed BfA+
--    range - none are Legion content.
-- ============================================================

DELETE FROM `spawn_tracking_template` WHERE `MapId` IN (
    1642, 1643, 2127, 2175, 2222, 2444, 2552, 2600, 2601, 2651,
    2657, 2661, 2678, 2683, 2686, 2690, 2698, 2706, 2731, 2739, 2769, 2792
);

-- ============================================================
-- 3. gameobject — delete spawns on maps that don't exist in this server's map
--    data, EXCLUDING map 2106 (that subset is handled by the dedicated
--    Warsong Gulch/Arathi Basin item in ARGUSCORE_FIXES.md - those rows need
--    their map corrected to 489/529, not deleted).
--    Error: "Table `gameobject` has gameobject (GUID: N Entry: N) spawned on a
--    non-existed map (Id: N), skip" (ObjectMgr.cpp:2549). Remaining map ids
--    (1642, 1643, 2241, 2444, 2454) confirmed BfA+ range - none are Legion.
-- ============================================================

DELETE FROM `gameobject` WHERE `map` IN (1642, 1643, 2241, 2444, 2454);

-- ============================================================
-- 4. spell_script_names — delete confirmed old-expansion/post-Legion orphan
--    rows. Error: "Scriptname: `X` spell (Id: N) does not exist." (ObjectMgr.cpp:6002).
--
--    IMPORTANT: this is a small, deliberately-conservative subset of the full
--    orphan list. Of the 121 scriptnames in this table with no matching
--    RegisterSpellScript() call anywhere in ArgusCore's src/, 86 are
--    class-ability names (spell_dh_*, spell_dk_*, spell_dru_*, spell_hun_*,
--    spell_mage_*, spell_monk_*, spell_pal_*, spell_pri_*, spell_rog_*,
--    spell_sha_*, spell_warl_*, spell_warr_*) that may represent genuine
--    undocumented Legion content gaps rather than dead data - those are
--    intentionally NOT deleted here. See ARGUSCORE_FIXES.md for the full
--    reasoning and the worklist of what still needs individual verification.
--    Only the 24 confirmed old-expansion-boss/event or post-Legion names,
--    plus the 10 already deep-dive-verified not-Legion names from the
--    separately-tracked 18-live-script investigation, are included below.
-- ============================================================

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_anubarak_leeching_swarm',
    'spell_argaloth_consuming_darkness',
    'spell_black_knight_ghoul_explode_risen_ghoul',
    'spell_bronjahm_magic_bane',
    'spell_bronjahm_soulstorm_targeting',
    'spell_earthrager_ptah_flame_bolt',
    'spell_faction_champion_death_grip',
    'spell_faction_champion_warl_unstable_affliction',
    'spell_flame_warder_meteor_fists',
    'spell_garfrost_permafrost',
    'spell_icecrown_through_the_eye_the_eye_of_the_lk',
    'spell_jormungars_burning_bile',
    'spell_koralon_meteor_fists',
    'spell_koralon_meteor_fists_damage',
    'spell_krick_pursuit_confusion',
    'spell_midsummer_test_ribbon_pole_channel',
    'spell_mistress_kiss',
    'spell_mistress_kiss_area',
    'spell_pos_ice_shards',
    'spell_power_of_the_twins',
    'spell_valkyr_essences',
    'spell_bg_seething_shore_parachute',
    'spell_bg_seething_shore_speed_up',
    'spell_chapter1_runeforging_credit',
    'spell_gen_50pct_count_pct_from_max_hp',
    'spell_alysrazor_fieroblast',
    'spell_eadric_radiance',
    'spell_elementium_spike_shield',
    'spell_faction_champion_dru_lifebloom',
    'spell_halion_blazing_aura',
    'spell_halion_twilight_cutter',
    'spell_marwyn_shared_suffering',
    'spell_occuthar_occuthars_destruction',
    'spell_stalactite_mod_dest_height'
);
