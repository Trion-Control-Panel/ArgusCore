-- Bind the newly-implemented Death Knight Decomposing Aura (PvP Honor Talent) C++ script to
-- its spell id. No existing spell_script_names row for this ScriptName anywhere in this repo's
-- SQL or the base TDB dump. Previously documented as blocked on missing AreaTrigger client data
-- (DestinyCore's own implementation ties this to a personal-zone AreaTrigger with no matching
-- areatrigger_create_properties row), but a newly-added reference (LegionCore-7.3.5V2)
-- implements the same ability as a plain debuff with a distance-based stack decay instead - no
-- AreaTrigger needed, so the blocker didn't actually apply.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_decomposing_aura';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(199721, 'spell_dk_decomposing_aura');
