-- Bind the newly-implemented Monk Gift of the Ox proc script to its spell id.
-- spell_monk_gift_of_the_ox_aura is a brand-new class bound to spell 124502 -
-- no existing spell_script_names row for it anywhere in this repo's SQL, so
-- without this binding the talent would never have a chance to spawn a
-- healing sphere when taking damage.
--
-- NOTE: this does NOT bind the companion at_monk_gift_of_the_ox_sphere
-- AreaTriggerAI (the script that makes the spawned orb heal on pickup).
-- That requires a row in `areatrigger_create_properties` referencing real
-- client DB2 visual/shape asset ids (Id, AreaTriggerId, Shape, ShapeData0-5,
-- MoveCurveId, etc.) that this repo's SQL has no record of and that can't be
-- safely inferred from the C++ alone - guessing at those ids would risk
-- pointing at unrelated real client data. The orb-spawn spells will still
-- cast per this migration's binding; picking up the resulting orb will not
-- yet heal until that DB2-sourced row is supplied separately.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_gift_of_the_ox_aura';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(124502, 'spell_monk_gift_of_the_ox_aura'); -- Gift of the Ox
