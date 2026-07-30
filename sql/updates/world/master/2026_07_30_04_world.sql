-- Fervent Martyr (196923) - Holy level 90 talent.
-- The consuming half (spell_pal_light_of_the_martyr removing the 223316 buff) already existed
-- in ArgusCore; the granting half (Holy Light/Flash of Light casts stacking the buff) did not
-- exist anywhere. This adds the missing granting proc.
DELETE FROM `spell_script_names` WHERE `spell_id` = 196923;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(196923, 'spell_pal_fervent_martyr');
