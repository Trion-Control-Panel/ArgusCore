-- Warlock Demonic Circle: Teleport range gate (48020): real client behavior used a fake-aura
-- marker (62388) with no genuine Spell.db2 record - confirmed absent (not even a blank placeholder
-- row) across three separate 7.3.5 client builds, and ArgusCore's engine has no SendFakeAuraUpdate
-- equivalent. Reimplemented without any marker spell: a direct OnCheckCast range check against the
-- circle's own GameObject, in a new spell_warl_demonic_circle_teleport_check class bound to the
-- same id (48020) as the existing spell_warl_demonic_circle_teleport AuraScript.
INSERT IGNORE INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(48020, 'spell_warl_demonic_circle_teleport_check');
