-- Warlock: Demonic Gateway (111771) - core party-teleport utility, entirely missing from
-- ArgusCore. Confirmed via DestinyCore and AshamaneCore (identical implementations), but
-- substantially redesigned around ArgusCore's actual engine idioms - see in-code comments on
-- spell_warl_demonic_gateway_interact in spell_warlock.cpp for the full explanation (the
-- references drive interaction through a legacy UNIT_FIELD_INTERACT_SPELLID field and a
-- manual CreatureAI::OnSpellClick override; ArgusCore's version uses the DB-driven
-- npc_spellclick_spells system instead, including its built-in SPELL_CLICK_USER_PARTY
-- group-membership check, which replaces the reference's manual Group comparison entirely).
--
-- npcflag/unit_flags updated via bitwise OR to avoid clobbering any other flags already set on
-- these rows.
UPDATE `creature_template` SET
    `ScriptName` = 'npc_warl_demonic_gateway',
    `npcflag` = `npcflag` | 0x01000000,     -- UNIT_NPC_FLAG_SPELLCLICK
    `unit_flags` = `unit_flags` | 0x6       -- UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL
WHERE `entry` IN (59262, 59271);

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (59262, 59271);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(59262, 113902, 2, 3),  -- green: NPC_CLICK_CAST_TARGET_CLICKER, SPELL_CLICK_USER_PARTY
(59271, 113902, 2, 3);  -- purple: same

DELETE FROM `spell_script_names` WHERE `spell_id` IN (111771, 113902)
    AND `ScriptName` IN ('spell_warl_demonic_gateway', 'spell_warl_demonic_gateway_interact');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(111771, 'spell_warl_demonic_gateway'),
(113902, 'spell_warl_demonic_gateway_interact');
