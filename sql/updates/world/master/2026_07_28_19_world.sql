-- Warlock: third pass of the class-diff sweep. Three more pet-command abilities found -
-- Felstorm (119914, Felguard), Meteor Strike (171152, Infernal), and Whiplash (119909,
-- Succubus/Sayaad) - completing the same "player command -> pet acts, mirror the pet's cooldown
-- onto the player's spell" family as the Spell Lock/Shadow Lock/Suffering trio from the previous
-- pass. Confirmed via logs/DestinyCore/sql/DB_world_735.02.sql's own spell_script_names dump.
--
-- Felstorm's own reference script never actually casts the pet ability - only the cooldown mirror
-- is scripted, meaning the pet's own DB2 data handles the mirrored cast on its own; the other two
-- (and the previous pass's trio) do cast the pet ability explicitly. Preserved that distinction
-- rather than adding an unscripted-in-the-reference CastSpell call that might double-trigger it.
--
-- Also confirmed as dead-in-reference this pass, no change needed: Eradication (196414) - its
-- CheckProc unconditionally returns false, permanently disabling its own proc; nothing to port.
-- Void Ray - extends the caster's own Corruption duration on hit; deferred rather than ported,
-- since its relevance/currentness couldn't be confirmed and it isn't corroborated by any other
-- reference core. Immolate's own cast script and Immolate Dot/proc - same "Channel Demonfire
-- Activator" pattern flagged as superseded in the previous pass, re-confirmed here (Immolate's own
-- reference script also references the same activator).
--
-- Remaining unchecked: Cauterize Master, Grimoire of Service/Synergy, Soul Conduit/Harvest, Eye
-- Laser, Molten Core, Twilight Ward, Sindorei Spite, Create Healthstone (Soulwell)/Glyph of
-- Soulwell (likely glyph-dead).

DELETE FROM `spell_script_names` WHERE `ScriptName` IN
(
    'spell_warl_felstorm',
    'spell_warl_meteor_strike',
    'spell_warl_whiplash'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(119914, 'spell_warl_felstorm'),
(171152, 'spell_warl_meteor_strike'),
(119909, 'spell_warl_whiplash');
