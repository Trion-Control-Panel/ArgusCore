-- Death Knight: Blood Mirror (206977) - Blood PvP talent, entirely missing from ArgusCore.
-- Absorbs a percentage of damage taken and redirects that amount as damage (221847) onto the
-- enemy the caster had targeted when Blood Mirror was applied. Confirmed genuine and
-- corroborated identically by DestinyCore and AshamaneCore; matched structurally (if
-- independently) by LegionCore-7.3.5V2. See in-code comment on spell_dk_blood_mirror for why
-- ArgusCore's implementation resolves the mirrored target from the caster's current selection
-- at apply time rather than the references' Aura::Variables scratch store, which does not
-- exist in ArgusCore's Aura class.
DELETE FROM `spell_script_names` WHERE `spell_id` = 206977 AND `ScriptName` = 'spell_dk_blood_mirror';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(206977, 'spell_dk_blood_mirror');
