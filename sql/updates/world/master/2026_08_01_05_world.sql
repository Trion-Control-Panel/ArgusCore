-- Cleanup: orphaned gameobject_template_addon rows with no matching gameobject_template row.
-- Addon rows exist purely to supplement a base template; with no base row they can never be
-- loaded (ObjectMgr logs "GameObject template (Entry: X) does not exist but has a record in
-- gameobject_template_addon" and skips them at every startup) - same dead-child-record pattern
-- as the quest_poi/spell_script_names cleanup earlier this session. See ARGUSCORE_FIXES.md.
DELETE FROM `gameobject_template_addon` WHERE `entry` IN (
    244798, 246387, 246670, 249382, 250897, 250898, 251966, 252188, 252551,
    259417, 259418, 259419, 259420, 259421, 259422, 259427, 259431, 259433, 259434, 259437,
    259684, 259689, 259690, 259694, 259697, 259702, 259728, 259730, 259928, 259985,
    265486, 265488, 266295, 266508, 266510, 266576, 266577, 266592, 266593, 266656,
    266803, 266804, 266806, 266826, 266846, 266959, 267190, 267924,
    268372, 268373, 268374, 268375, 268621, 268680, 268709, 268710, 268711, 268712, 268713, 268714,
    269042, 269121, 269122, 269221, 269222, 269223, 269224, 269904, 269977, 269978,
    272062, 273661, 280905, 280936, 280937, 280938, 280939, 280940, 280941
);

-- Cleanup: 4 rows reference a WorldEffectID (16227) that doesn't exist in this build's
-- WorldEffect.db2. ArgusCore already self-heals this at runtime (ObjectMgr clamps it to 0 and
-- logs a warning every startup), so this is purely to silence the repeated log spam - no
-- behavior change, since the in-memory value was already being reset to 0 either way.
UPDATE `gameobject_template_addon` SET `WorldEffectID` = 0
WHERE `entry` IN (191028, 191029, 191030, 241920) AND `WorldEffectID` = 16227;
