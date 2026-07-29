-- Warlock: second pass of the class-diff sweep. Three genuinely missing pet-command abilities
-- found - Spell Lock (119910), Shadow Lock (171140), and Suffering (119907), which command a
-- Felhunter/Doomguard/Voidwalker guardian pet respectively to use its own ability, mirroring that
-- pet ability's cooldown onto the player's own command spell. These match an already-established
-- pattern in this file (the existing spell_warl_devour_magic/spell_warl_seduction scripts cover
-- the same "player command -> pet acts" shape for Felhunter/Succubus's other abilities), so they
-- were low-risk to port. Confirmed via logs/DestinyCore/sql/DB_world_735.02.sql's own
-- spell_script_names dump. The reference's own pet-entry-type checks are commented out consistently
-- across all three scripts (not just one) - read as a deliberate simplification rather than a bug,
-- and followed as-is rather than reintroduced.
--
-- Investigated and deferred this pass (increasingly interdependent/uncertain territory, not worth
-- rushing): Immolate Dot/Immolate proc (157736/193541) - the shard-generation-on-tick and
-- "Channel Demonfire Activator" mechanic look like a superseded, pre-rework alternate path to the
-- Channel Demonfire already implemented last pass (which uses its own independent periodic timer,
-- not an Immolate-tick trigger); left alone rather than risk double-implementing the same ability
-- two different ways. Demonic Call/Demonic Calling - a two-piece talent pair (free Wild Imp summon
-- proc) needing both halves implemented together, deferred as a unit alongside Demonwrath from the
-- previous pass since Demonwrath also references Demonic Calling. Demonic Empowerment - depends on
-- the separately-missing Shadowy Inspiration talent. Demonskin/Soul Leech (applier + aura) - a
-- tightly-coupled three-piece Demonology defensive kit; deferred as a unit rather than partially
-- implemented. Demonic Gateway - the reference's own HandleSummon only casts one end of what should
-- be a linked two-way portal pair, suggesting the reference itself is incomplete; not safe to port
-- as-is. Dark Regeneration, Cauterize Master, Grimoire of Service/Synergy, Soul Conduit/Harvest,
-- Eradication, Eye Laser, Felstorm, Void Ray, Whiplash, Meteor Strike, Molten Core, Twilight Ward,
-- Sindorei Spite, Create Healthstone (Soulwell)/Glyph of Soulwell (likely glyph-dead) - not yet
-- investigated.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'spell_warl_spell_lock',
    'spell_warl_shadow_lock',
    'spell_warl_suffering'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(119910, 'spell_warl_spell_lock'),
(171140, 'spell_warl_shadow_lock'),
(119907, 'spell_warl_suffering');
