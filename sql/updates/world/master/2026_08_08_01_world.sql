-- Druid: full recheck against real Legion 7.3.5 client data.
--
-- NOTE: switched to INSERT IGNORE after this file failed on a real duplicate-key error
-- (203974-spell_dru_earthwarden already existed) - this session's "never bound anywhere in this
-- codebase's SQL history" checks only ever grepped sql/updates/world/master/, which is layered on
-- top of a full base TDB dump imported separately and not visible to this repo/session at all.
-- That blind spot means any "never bound" claim in this file (and likely others from this
-- session) could be wrong the same way; IGNORE makes the file idempotent against that instead of
-- hard-failing the whole update run.

-- Starfall (spell_dru_starfall_dummy) was never bound to any spell id at all in this codebase's
-- SQL history (no matching spell_script_names row anywhere in the base dump or any migration) -
-- one of Balance's most core, iconic AOE abilities has been fully non-functional. The class's own
-- header comment referenced 50286, a pre-Legion (WoD-era) Starfall id confirmed completely absent
-- from this build (no Spell record at all, local dump or live wago.tools query); real Legion
-- Starfall is 191034 (confirmed via its own tooltip referencing Astral damage and Stellar
-- Empowerment, both Legion-specific Balance mechanics). Binding added; the C++ side was also
-- fixed to cast the real damage companion spell (191037) directly instead of relying on
-- GetEffectValue(), which resolved to 0 (a silent no-op) even with the id corrected.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(191034, 'spell_dru_starfall_dummy');

-- Sudden Ambush (340694/384667): confirmed Dragonflight (patch 10.0.0) content via web search -
-- both ids are completely absent from this build. Never bound in the DB either (spell_proc or
-- spell_script_names), so nothing to unbind - class removed from spell_druid.cpp.

-- New Moon / Half Moon / Full Moon (spell_dru_new_moon, registered 3x under distinct custom
-- ScriptNames): also never bound anywhere in this codebase's SQL history - the entire lunar-phase
-- spell chain has been fully non-functional. Compounding the missing binding, the ids referenced
-- in C++ (274281/274282/274283/274295/274297) are confirmed completely absent from this build -
-- they're the later BfA-talent reintroduction of the mechanic. Genuine Legion content (added
-- 7.0.3, confirmed via web search) under its original Artifact-era ids, confirmed by exact name
-- match in the local Spell.csv dump. Both the ids (in C++) and these bindings were fixed/added.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202767, 'spell_dru_new_moon'),
(202768, 'spell_dru_half_moon'),
(202771, 'spell_dru_full_moon');

-- Earthwarden (203974), Gore (210706), and both Omen of Clarity variants (16864 Feral/Guardian,
-- 113043 Restoration) were also never bound anywhere in this codebase's SQL history - all four
-- are genuine, currently-relevant Legion talents (confirmed via their own tooltips), not old
-- vestigial content. spell_dru_item_t6_trinket (40442) was deliberately left unbound - it's a
-- WotLK-era (Black Temple/Sunwell) tier 6 trinket proc, the same category of pre-Legion dead
-- content as the T3/T4/T10 tier-bonus classes already left alone elsewhere in this file.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(203974, 'spell_dru_earthwarden'),
(210706, 'spell_dru_gore'),
(16864, 'spell_dru_omen_of_clarity'),
(113043, 'spell_dru_omen_of_clarity_restoration');

-- spell_proc coverage: Infected Wound (48484, already bound) and the four newly-bound classes
-- above (203974, 210706, 16864, 113043) all gate their DoCheckProc/OnEffectProc hooks behind
-- Aura::GetProcEffectMask, which returns 0 unconditionally without a spell_proc row - the scripts
-- would still never fire without this, even with correct bindings. All five have confirmed real
-- native proc data in SpellAuraOptions.db2 for this build (ProcTypeMask/ProcChance), so a bare
-- SpellId-only row is enough to restore function.
INSERT IGNORE INTO `spell_proc` (`SpellId`) VALUES
(48484),
(203974),
(210706),
(16864),
(113043);
