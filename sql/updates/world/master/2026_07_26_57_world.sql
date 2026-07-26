-- Unbind 35 Priest script names whose C++ classes were removed from spell_priest.cpp for
-- being confirmed post-Legion forward drift (Shadowlands/Dragonflight/War Within covenant
-- abilities, tier-set bonuses, and talents that never existed in Legion 7.3.5). Priest predates
-- Legion, so this file mixes real backward-drift risk too, but every one of these was
-- independently verified against two Legion-era reference codebases (DestinyCore and
-- LegionCore-7.3.5-merged) and found in neither under any name, plus confirmed via spell-id
-- magnitude (all well beyond Legion's ~270000 ceiling) and, in several cases, explicit patch
-- version numbers or covenant/hero-talent names in the code's own comments (e.g.
-- "Patch 10.1.0", "Voidwrath" - a War Within hero talent). See ARGUSCORE_FIXES.md for the full
-- writeup, including several classes that were investigated and confirmed *not* to be forward
-- drift despite superficially similar names (Divine Star Shadow, Halo Shadow, Power of the Dark
-- Side, and Penance's own Dark-Reprimand-shared classes were all kept - only the Dark Reprimand-
-- specific registration binding on the shared Penance class was removed, not the class itself).

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_pri_abyssal_reverie', 'spell_pri_answered_prayers', 'spell_pri_assured_safety',
    'spell_pri_blaze_of_light', 'spell_pri_crystalline_reflection', 'spell_pri_dark_indulgence',
    'spell_pri_divine_image', 'spell_pri_divine_image_spell_triggered', 'spell_pri_divine_image_stack_timer',
    'spell_pri_divine_service', 'spell_pri_empowered_renew', 'spell_pri_epiphany',
    'spell_pri_essence_devourer_heal', 'spell_pri_focused_mending', 'spell_pri_from_darkness_comes_light',
    'spell_pri_heavens_wrath', 'spell_pri_holy_mending', 'spell_pri_holy_word_salvation',
    'spell_pri_holy_word_salvation_cooldown_reduction', 'spell_pri_mind_devourer', 'spell_pri_mind_devourer_buff',
    'spell_pri_painful_punishment', 'spell_pri_pain_transformation', 'spell_pri_dark_reprimand',
    'spell_pri_holy_10_1_class_set_2pc', 'spell_pri_holy_10_1_class_set_2pc_chooser', 'spell_pri_holy_10_1_class_set_4pc',
    'spell_pri_holy_10_1_class_set_4pc_aura', 'spell_pri_purge_the_wicked', 'spell_pri_purge_the_wicked_dummy',
    'spell_pri_rhapsody', 'spell_pri_rhapsody_proc', 'spell_pri_shadow_covenant',
    'spell_pri_unfurling_darkness', 'spell_pri_whispering_shadows', 'spell_pri_whispering_shadows_effect'
);
