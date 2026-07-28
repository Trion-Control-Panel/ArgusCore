-- Paladin: bind five more newly-implemented spells found while resuming the class-diff sweep after
-- it was previously "wrapped up" - Avenger's Shield (31935, boosts damage to the primary target
-- when First Avenger - 203776 - is talented; the old Grand Crusader-proc-removal half of the
-- reference's version was NOT ported, since ArgusCore's existing spell_pal_grand_crusader is
-- already a modern DB2-proc-driven cooldown reset with no separate "proc buff" to consume), Crusade
-- (231895, Retribution's Avenging Wrath replacement - grows in power per Holy Power spent while
-- active), Judgment of Light (183778) plus its proc half (196941, heals the paladin for personally
-- damaging a judged target), Holy Wrath (210220, damage scaling with the caster's own missing
-- health, read from the spell's own per-target-type DB2 effects), and Light of the Protector /
-- Hand of the Protector (184092 / 213652, a self-heal scaling with the caster's own missing
-- health). All five corroborated across at least two of the four reference cores; Holy Wrath,
-- Light/Hand of the Protector, and Cleanse were the only paladin candidates present in all four,
-- but Cleanse's own registration is commented out even in DestinyCore/AshamaneCore's reference
-- (dead code left in place - the "nothing to dispel" check is already handled generically by the
-- engine's own dispel path) and was deliberately skipped rather than revived.
--
-- Also confirmed as false positives during this pass, no change needed: Crusader Strike (already
-- covered - its own script only ever existed to manually reduce Holy Shock/Light of Dawn cooldown
-- when Crusader's Might was talented, which ArgusCore already implements the other way around as a
-- DB2 proc-driven aura), Flash of Light (already covered - the reference manually strips the
-- Infusion of Light buff on cast, but ArgusCore's Infusion of Light is charge/DB2-driven and
-- consumes itself), Activate Forbearance (already covered - ArgusCore already applies Forbearance
-- inline from each of Blessing of Protection/Divine Shield/Lay on Hands individually, matching
-- LegionCore-7.3.5's older per-spell duplication rather than DestinyCore/AshamaneCore's later
-- single-helper refactor; not worth refactoring to match purely for its own sake).
--
-- Still deliberately deferred: Word of Glory (needs its Holy Power accounting reworked around the
-- GetPowerTypeCostAmount() idiom before it's safe to port - see the prior Greater Blessing of
-- Kings/Light of the Martyr entry), The Fires of Justice (a Word of Glory-cost-discount proc gate
-- that would be inert dead code until Word of Glory itself is implemented), and Aura of Sacrifice
-- (needs new custom AreaTrigger infrastructure - see the prior Bastion of Light/Divine
-- Intervention/Blade of Wrath/Seraphim entry).
--
-- No existing spell_script_names rows for any of these seven spell ids anywhere in this repo's SQL
-- or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'spell_pal_avengers_shield',
    'spell_pal_crusade',
    'spell_pal_judgment_of_light',
    'spell_pal_judgment_of_light_proc',
    'spell_pal_holy_wrath',
    'spell_pal_light_of_the_protector',
    'spell_pal_hand_of_the_protector'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(31935, 'spell_pal_avengers_shield'),
(231895, 'spell_pal_crusade'),
(183778, 'spell_pal_judgment_of_light'),
(196941, 'spell_pal_judgment_of_light_proc'),
(210220, 'spell_pal_holy_wrath'),
(184092, 'spell_pal_light_of_the_protector'),
(213652, 'spell_pal_hand_of_the_protector');
