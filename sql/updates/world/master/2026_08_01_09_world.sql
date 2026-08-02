-- Druid: spell_dru_entangling_roots removed, its CheckProc-only sibling
-- (spell_dru_entangling_roots_aura) is now registered standalone under its own name instead of
-- via the RegisterSpellAndAuraScriptPair(spell_dru_entangling_roots, spell_dru_entangling_roots_aura)
-- pairing (which bound both to the single name 'spell_dru_entangling_roots'). Rebind the two
-- existing spell_id rows to the new standalone name so the still-legitimate CheckProc logic
-- (damage from the root itself doesn't count toward breaking it) keeps attaching.
-- See ARGUSCORE_FIXES.md for why spell_dru_entangling_roots itself was removed
-- (SPELL_DRUID_CURIOUS_BRAMBLEPATCH, 330670, confirmed absent from Spell.db2 under any id/name).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (339, 102359) AND `ScriptName` = 'spell_dru_entangling_roots';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
    (339, 'spell_dru_entangling_roots_aura'),
    (102359, 'spell_dru_entangling_roots_aura');
