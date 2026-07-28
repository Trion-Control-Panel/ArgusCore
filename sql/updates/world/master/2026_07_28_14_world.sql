-- Shaman: first pass of a class-diff sweep (Shaman was previously entirely unaudited). Four
-- genuinely missing abilities found and implemented, corroborated across all four reference cores
-- (Bloodlust/Heroism/Feral Lunge) or confirmed directly from a reference core's own base world DB
-- dump (Spirit Link Totem, whose reference script carries no spell id in a code comment - its
-- binding was confirmed as 98020 via logs/DestinyCore/sql/DB_world_735.02.sql rather than guessed).
--
-- Bloodlust (2825) and Heroism (32182) apply the correct sister-exhaustion debuff (Sated /
-- Exhaustion respectively) and exclude any target already under one of the four raid-wide lockout
-- debuffs (Sated, Exhaustion, Hunter's Insanity, Mage's Temporal Displacement, the Netherwinds
-- spirit beast's Fatigued) from being re-hasted - both already have spell_script_names rows in the
-- base TDB dump for 'spell_sha_bloodlust'/'spell_sha_heroism', so only Feral Lunge and Spirit Link
-- Totem need new rows here.
--
-- Also confirmed as false positives during this pass, no change needed: Chain Heal (1064) - the
-- reference's "High Tide" chain-target-count talent looks superseded; ArgusCore's existing
-- spell_sha_deluge (a generic CalcHealing-based script) is already bound to Chain Heal via
-- 2025_08_28_00_world.sql alongside Healing Wave/Healing Surge, matching LegionCore-7.3.5's
-- independently-written, differently-shaped Chain Heal script (a Deluge healing-bonus check, not a
-- High Tide extra-target one) - the two secondary/primary reference angles disagree with each
-- other, and ArgusCore's existing generic implementation already matches the secondary's angle.
-- Undulation - re-confirmed already covered by ArgusCore's existing spell_sha_undulation_passive,
-- which achieves the same "every 3rd Healing Surge/Wave cast grants the buff" result via a
-- proc-gated aura on the Undulation passive itself instead of the reference's per-spell counter
-- variable hack.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sha_feral_lunge', 'spell_sha_spirit_link');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(196884, 'spell_sha_feral_lunge'),
(98020, 'spell_sha_spirit_link');
