-- Druid: bind eight more newly-implemented spells found while continuing the class-diff sweep -
-- Blessing of the Ancients (202360), Killer Instinct (108299), Living Seed (48500), Infected
-- Wound (48484), the three spec-specific Incarnation talents (Chosen of Elune 102560, King of the
-- Jungle 102543, Guardian of Ursoc 102558 - each auto-shifts into the matching form if not already
-- in it), and Wild Charge/Moonkin (102383). All corroborated across DestinyCore and AshamaneCore
-- (Wild Charge, Incarnations, Killer Instinct, Living Seed, Infected Wound) or DestinyCore alone
-- with the mechanic independently verified as genuine Legion Artifact content (Blessing of the
-- Ancients). No existing spell_script_names rows for any of these ScriptNames anywhere in this
-- repo's SQL or the base TDB dump.
--
-- Note: Ironbark (DestinyCore only, not in AshamaneCore) was investigated and deliberately NOT
-- implemented - its reference logic calls `ModifyCooldown(id, 0)`, a no-op that cannot be what was
-- intended, and with no second reference to corroborate a fix, left as a documented gap instead of
-- guessed at.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_dru_blessing_of_the_ancients', 'spell_dru_killer_instinct', 'spell_dru_living_seed',
    'spell_dru_infected_wound', 'spell_dru_incarnation_chosen_of_elune',
    'spell_dru_incarnation_king_of_the_jungle', 'spell_dru_incarnation_guardian_of_ursoc',
    'spell_dru_wild_charge_moonkin'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202360, 'spell_dru_blessing_of_the_ancients'),
(108299, 'spell_dru_killer_instinct'),
(48500, 'spell_dru_living_seed'),
(48484, 'spell_dru_infected_wound'),
(102560, 'spell_dru_incarnation_chosen_of_elune'),
(102543, 'spell_dru_incarnation_king_of_the_jungle'),
(102558, 'spell_dru_incarnation_guardian_of_ursoc'),
(102383, 'spell_dru_wild_charge_moonkin');
