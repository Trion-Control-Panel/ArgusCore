-- Camp Narache ambient combat: Fledgling Brave (36942) vs Bristleback Invader (36943).
-- Both use SmartAI so spell abilities are fully DB-configured.  Combat begins via
-- natural faction hostility (Quilboar vs Tauren).  When killed they respawn and
-- re-engage, giving a continuously refreshing fight.
-- Spells used:
--   36942 Fledgling Brave     - War Stomp (24375) AoE stun, every 15-25 s
--   36943 Bristleback Invader - Frenzy    (8269)  attack-speed buff, every 20-35 s

UPDATE `creature_template`
    SET `AIName` = 'SmartAI', `ScriptName` = ''
    WHERE `entry` IN (36942, 36943);

-- Sheath state 1 (melee drawn) so weapons are visible during combat.
INSERT INTO `creature_template_addon` (`entry`, `PathId`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvpFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(36942, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''), -- Fledgling Brave
(36943, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '') -- Bristleback Invader
ON DUPLICATE KEY UPDATE `SheathState` = 1;

-- Smart scripts
-- event_type 0  = SMART_EVENT_UPDATE_IC  (params: InitialMin, InitialMax, RepeatMin, RepeatMax)
-- action_type 11 = SMART_ACTION_CAST     (params: SpellId, CastFlags)
-- target_type 1  = SMART_TARGET_SELF
-- CastFlag 2     = SMARTCAST_TRIGGERED   (no cast time / mana cost, passes most restrictions)

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (36942, 36943) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `Difficulties`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param_string`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `action_param7`, `action_param_string`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_param_string`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
-- 36942 Fledgling Brave: War Stomp every 15-25 s (initial delay 10-20 s)
(36942, 0, 0, 0, '', 0, 0, 100, 0, 10000, 20000, 15000, 25000, 0, '', 11, 24375, 2, 0, 0, 0, 0, 0, NULL, 1, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Fledgling Brave - IC Update - Cast War Stomp'),
-- 36943 Bristleback Invader: Frenzy every 20-35 s (initial delay 15-25 s)
(36943, 0, 0, 0, '', 0, 0, 100, 0, 15000, 25000, 20000, 35000, 0, '', 11, 8269,  2, 0, 0, 0, 0, 0, NULL, 1, 0, 0, 0, 0, NULL, 0, 0, 0, 0, 'Bristleback Invader - IC Update - Cast Frenzy');
