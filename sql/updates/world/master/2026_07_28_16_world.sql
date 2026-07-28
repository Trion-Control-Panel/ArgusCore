-- Shaman: third pass of the class-diff sweep, closing out the remaining candidates from the
-- original diff. Two genuinely missing abilities found - Earth Elemental (198103) and Fire
-- Elemental (198067), the baseline Elemental-spec cooldown pet summons - both simple dummy-effect
-- triggers for their real summon spells (188616/188592). Confirmed via
-- logs/DestinyCore/sql/DB_world_735.02.sql's own spell_script_names dump.
--
-- Also confirmed as false positives this pass, no change needed: Healing Stream / Healing Stream
-- Totem (both glyph-only or superseded by the totem-casts-its-own-heal design already used by the
-- existing spell_sha_healing_stream_totem_heal). Stormbringer (201845) - the exact same id as the
-- already-implemented spell_sha_stormsurge (Blizzard renamed the talent, kept the id); ArgusCore's
-- version is a modern DB2-proc-driven design vs. the reference's manual "only proc off base-attack,
-- manually reset Stormstrike" hack. Lava Lash Spread Flame Shock (105792, unconditional on every
-- Lava Lash hit) - superseded by the existing, talent-gated spell_sha_path_of_flames_spread (same
-- copy-the-aura-onto-a-new-target logic, but correctly bound to the Path of Flames talent's own
-- trigger spell instead of being hardcoded into Lava Lash unconditionally). Liquid Magma Effect
-- (192223) - already covered by the existing spell_sha_liquid_magma_totem, which deals the same
-- damage via a precise OnEffectHitTarget cast instead of the reference's blunter OnCast self-damage
-- approach. Wind Rush Totem's spell-side class - the reference's own Register() is completely
-- empty, i.e. dead code with nothing to port (the areatrigger half, already covered by ArgusCore's
-- existing areatrigger_sha_wind_rush_totem, is the only half that ever did anything).
-- "Lightning Bolt (Elemental)"/enhancement_lightning_bolt/flame_shock_elem - despite the misleading
-- "_elem" naming (not pet-related - "Elemental" refers to the spec), these are old pre-Legion
-- Elemental-spec resource mechanics (a pre-Maelstrom "Elemental Power" charge system); the
-- Lightning Bolt id involved (187837) doesn't even match ArgusCore's current Lightning Bolt
-- (188196, already fully implemented with its own modern Maelstrom overload/energize scripts) -
-- treated as superseded/removed rather than a gap.
--
-- Remaining deferred, unresolved this pass: Earthen Shield Totem (201633) needs its own
-- AreaTrigger to apply/track the absorb on nearby allies - same class of problem as Paladin's Aura
-- of Sacrifice and this class's own Ancestral Protection Totem/Crashing Storm, all blocked on
-- unconfirmable `areatrigger_create_properties` data. Spiritwalker's Grace's secondary interaction
-- (checks aura 159651, casts 159652 on hit) - identity of those two ids couldn't be confirmed, and
-- the ability's core "cast while moving" effect doesn't depend on this script to function, so left
-- uninvestigated rather than guessed at. Voodoo Totem - appears NPC-bound (tied to
-- npc_voodoo_totem) rather than a player-facing Shaman spell; low priority, not pursued.
--
-- This effectively closes out the original ~49-candidate Shaman class-diff. Warlock remains
-- entirely unaudited.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_sha_earth_elemental', 'spell_sha_fire_elemental');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(198103, 'spell_sha_earth_elemental'),
(198067, 'spell_sha_fire_elemental');
