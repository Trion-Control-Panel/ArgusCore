-- Warlock: fourth pass of the class-diff sweep. Two more genuinely missing abilities -
-- Create Healthstone (Soulwell) (34130, grants a healthstone to whoever clicks the Soulwell
-- object - the party-utility counterpart to the player's own already-implemented personal Create
-- Healthstone at 6201) and Cauterize Master (119905, the sixth and final member of the pet-command
-- family alongside Spell Lock/Shadow Lock/Suffering/Felstorm/Meteor Strike/Whiplash - commands an
-- Imp to heal the caster). Confirmed via logs/DestinyCore/sql/DB_world_735.02.sql's own
-- spell_script_names dump.
--
-- Also confirmed dead-in-reference this pass, no change needed: Sin'dorei Spite (208868) - its
-- CheckProc unconditionally returns false, same as Eradication two passes ago; nothing to port.
--
-- Confirmed drift/removed, no change needed: Molten Core Dot - energizes POWER_DEMONIC_FURY, a
-- WoD-era Demonology resource entirely replaced by Legion's Soul Shard/Fel rework. Twilight Ward
-- S12 - "S12" is an old PvP season tier-set bonus interaction referencing Metamorphosis (itself
-- long since removed/reworked into the standalone Demon Hunter class). Shadow Bulwark - a
-- Vanilla-era spell id (17767) with no identifiable current equivalent; simple to implement but
-- deferred over unclear relevance rather than guessed at.
--
-- Deferred over unclear/unconfirmed relevance: Soul Conduit (215941, refunds mana on any cast with
-- a chance) and Soul Harvest (196098, duration scales with DoT'd enemy count) - both use old-style
-- APIs (PLAYER_FIELD_CURRENT_SPEC_ID, GetAttackableUnitListInRange) already established as
-- pre-refactor idioms elsewhere in this sweep, and neither is clearly identifiable as still-current
-- Warlock kit; left uninvestigated rather than guessed at.
--
-- Deferred over needing pet AI infrastructure (a bigger undertaking than a self-contained spell
-- script): Eye Laser (205231) - a Wild Imp pet ability target filter, tied into the broader
-- Wild Imp PetAI (`npc_pet_warlock_wild_imp`) that sits outside this sweep's scope. Grimoire of
-- Service (111859/111895-98) - summons a temporary pet of any type via a Creature-OnSummon hook
-- (not a simple spell-effect hook like the rest of this sweep) and was a Legion 7.0-era talent
-- believed pruned in a later expansion; not pursued given both the added complexity and doubtful
-- current relevance. Grimoire of Synergy - not yet investigated.
--
-- Still unchecked: Dark Regeneration, Grimoire of Synergy.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'spell_warl_create_healthstone_soulwell',
    'spell_warl_cauterize_master'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(34130, 'spell_warl_create_healthstone_soulwell'),
(119905, 'spell_warl_cauterize_master');
