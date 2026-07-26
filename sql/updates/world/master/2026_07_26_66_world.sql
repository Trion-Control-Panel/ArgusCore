-- Bind the newly-implemented Priest Power Word: Barrier (62618) AreaTrigger script.
-- AreaTriggerCreatePropertiesId 1489 already exists in the base TDB dump (SpellMiscId 5802,
-- 10s duration, 8yd radius) with an empty ScriptName - same "row exists, ScriptName blank"
-- pattern as the other Priest AreaTriggers (angelic_feather/halo/divine_star) before they were
-- bound, confirmed by cross-referencing DestinyCore's own "// AreaTriggerID - 1489" comment.

UPDATE `areatrigger_create_properties` SET `ScriptName` = 'areatrigger_pri_power_word_barrier' WHERE `Id` = 1489;
