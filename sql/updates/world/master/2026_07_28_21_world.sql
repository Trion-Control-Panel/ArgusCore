-- Warlock: fifth pass of the class-diff sweep. One more genuinely missing ability - Grimoire of
-- Synergy (171975) - applies its own passive aura to both the caster and their guardian pet;
-- whichever one procs (deals damage or heals) has a chance to buff the other. Self-contained, no
-- dependency on any other missing content. Confirmed via
-- logs/DestinyCore/sql/DB_world_735.02.sql's own spell_script_names dump.
--
-- Dark Regeneration (108359) - investigated, deferred. A Cataclysm-era emergency self-heal that
-- also heals the pet when it triggers; simple to implement, but its continued relevance in current
-- Warlock design couldn't be confirmed and it isn't corroborated by any other reference core, so
-- left uninvestigated further rather than guessed at.
--
-- This closes out the remaining candidates from the original ~51-item Warlock class-diff. Five
-- passes total have resolved roughly 33 genuine/false-positive/dead-confirmed items; what remains
-- (Void Ray, Soul Conduit, Soul Harvest, Dark Regeneration, Eye Laser, Grimoire of Service) is
-- either low-confidence (no cross-core corroboration, old-style APIs with no clear modern
-- equivalent) or blocked on pet AI infrastructure outside a plain spell-script sweep's scope.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warl_grimoire_of_synergy';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(171975, 'spell_warl_grimoire_of_synergy');
