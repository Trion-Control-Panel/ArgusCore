-- Shaman: second pass of the class-diff sweep. Three more genuinely missing abilities found and
-- implemented - Cloudburst Totem (157503, splits its heal across nearby allies) plus its storage
-- half (157504, banks a percentage of the shaman's own healing while the totem is up and releases
-- it as a burst via 157503 when the totem expires), Resonance Totem (202192, periodically grants
-- the owner Maelstrom), and Earthgrab Totem (51485, DR-style root-then-snare escalation). All
-- three confirmed via logs/DestinyCore/sql/DB_world_735.02.sql's own spell_script_names dump
-- rather than guessed, matching the same verification approach used for Spirit Link Totem.
--
-- Also confirmed as false positives this pass, no change needed: Ascendance (Water) - already
-- covered by the existing spell_sha_ascendance_restoration + spell_sha_restorative_mists pair
-- (same ids, 114052/114083, just a more modern proc-accumulate-then-redistribute design instead of
-- the reference's per-heal proc cast). Nature's Guardian - already covered by the existing
-- spell_sha_natures_guardian (same id, 30884, already-established modern cooldown-tracking design
-- instead of the reference's manual SpellHistory::HasCooldown hack). Ancestral Guidance - already
-- covered (false positive from the initial diff, not a new finding). Old Flametongue proc (10400/
-- 194084, a Vanilla-era weapon-enchant mechanic) - superseded by the existing, much newer
-- spell_sha_flametongue_weapon/_aura (319778/334294). Thunderstorm - the reference's only logic is
-- a Glyph of Thunderstorm check, and glyphs don't exist since Legion 7.0; needs no script at all.
-- Fulmination (88766, Lightning Shield-charge-consumption) - a Cataclysm/MoP-era mechanic
-- superseded by Legion's Maelstrom-based Elemental resource rework; no trace of it (or Lightning
-- Shield stacking) anywhere in ArgusCore's Earth Shock. Glyph of Healing Wave/Lakestrider/
-- Shamanistic Rage - the entire glyph system is gone since Legion 7.0. Totem Mastery, old Windfury
-- Totem, and Resurgence - all Cata/MoP-era mechanics with no trace of an equivalent anywhere in
-- ArgusCore's current design, and no corroboration from either LegionCore-7.3.5 or LegionCore-
-- 7.3.5V2; treated as removed/superseded rather than genuine gaps.
--
-- Crash Lightning's "aura" gate (187878, whether the cleave buff's own proc should be restricted to
-- Stormstrike/Lava Lash hits) is left unresolved - can't confirm locally whether ArgusCore's DB2
-- data for that buff even carries a proc-trigger effect for the gate to restrict, and adding an
-- inert DoCheckProc would be dead code if it doesn't.
--
-- Deferred over the same class of problem as Paladin's Aura of Sacrifice (needs AreaTrigger
-- infrastructure not confirmable from this repo's own data): Ancestral Protection Totem (207498 +
-- its areatrigger) and Crashing Storm (192246/210797/210801). Also deferred: the Fire/Storm/Earth
-- Elemental permanent-pet copy-cast cluster (spell_sha_lightning_bolt_elem, flame_shock_elem,
-- earth_elemental, fire_elemental, enhancement_lightning_bolt) - needs pet AI infrastructure, a
-- bigger undertaking than a self-contained spell script.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'spell_sha_cloudburst',
    'spell_sha_cloudburst_effect',
    'spell_sha_resonance_effect',
    'spell_sha_earthgrab'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(157503, 'spell_sha_cloudburst'),
(157504, 'spell_sha_cloudburst_effect'),
(202192, 'spell_sha_resonance_effect'),
(51485, 'spell_sha_earthgrab');
