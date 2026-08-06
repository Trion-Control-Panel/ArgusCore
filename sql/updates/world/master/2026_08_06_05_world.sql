-- creature_loot_template had ~180 "unused"/"does not exist" errors. Checking against AshamaneCore
-- revealed several distinct causes:
--
-- 1. Most of the flagged raid bosses (Onyxia, Kel'Thuzad, The Lich King, Sindragosa, Sartharion,
--    Anub'arak, Halion, etc.) are CORRECTLY set up - they use Timewalking-specific LootID values
--    under DifficultyID 4/5/6, which are real, populated loot tables. Their entry-matching
--    creature_loot_template rows are legitimately superseded old-original-difficulty tables, not
--    drift - left untouched.
--
-- 2. 56 creatures (mostly Alterac Valley NPCs: Korrak the Bloodrager, Wing Commanders, Frostwolf/
--    Stormpike faction guards, etc.) had NO creature_template_difficulty row at all in ArgusCore
--    for ANY difficulty. This isn't just a loot bug - Creature::SelectLevel() reads
--    CreatureDifficulty::MinLevel/MaxLevel as the *only* source of a creature's level, so these
--    were very likely spawning at level 0. Restored from AshamaneCore's data (which keeps
--    minlevel/maxlevel/lootid merged directly on creature_template, an older pre-split schema).
--    AshamaneCore's minlevel/maxlevel are frequently negative or zero (an old "relative to player
--    level" convention with no direct equivalent in ArgusCore's LevelScaling* fields) - translated
--    using the exact precedent already established elsewhere in this same DB (e.g. entry 13440
--    "Frostwolf Wolf Rider": AshamaneCore minlevel=-2 -> ArgusCore's own already-correct
--    MinLevel=1, MaxLevel=1, LevelScaling*=0): any non-positive minlevel/maxlevel pair becomes
--    MinLevel=1, MaxLevel=1 here too. HealthModifier/ManaModifier/ArmorModifier/DamageModifier/
--    LootID/PickPocketLootID/SkinLootID/GoldMin/GoldMax are carried over directly since those are
--    plain multipliers/ids with no schema-shape difference.
--
--    IMPORTANT: 17 of the originally-flagged entries (74351, 74353, 74363, 74380, 74382, 74980,
--    74983, 75058, 75135, 75285, 75286, 76036, 76037, 76038, 76039, 77232, 100003 - "Twilight
--    Highlands"-flavor trash) already have a correct, Legion-native creature_template_difficulty
--    row in ArgusCore (real LevelScalingMin/Max, VerifiedBuild=26365) that AshamaneCore's older
--    schema has no equivalent for - those are NOT touched here (AshamaneCore also shows lootid=0
--    for all of them, so there is nothing to fix for those specifically anyway).
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (10981,0,1,1,1.02,1,1,1,10981,0,10981,122,122);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (10986,0,1,1,1.3,1,1,1,10986,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (10990,0,1,1,1.3,1,1,1,10990,0,10990,124,124);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (10991,0,1,1,1.3,1,1,1,0,10991,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11657,0,1,1,1,2,1,1,11657,11657,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11675,0,1,1,1.3,1,1,1,0,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11678,0,1,1,1.3,1,1,1,11678,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11837,0,1,1,1.3,1,1,1,0,11837,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11838,0,1,1,1.35,1,1,1,11838,11838,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11839,0,1,1,1.35,1,1,1,11839,11839,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11840,0,1,1,1.35,1,1,1,11840,11840,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11947,0,1,1,50,1,1,35,11947,11947,0,109,109);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (11949,0,1,1,50,30,1,35,11949,11949,0,156,156);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (12050,0,1,1,0.4,1,1,1,12050,12050,0,159,159);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (12051,0,1,1,0.6,1,1,1,12051,12051,0,151,151);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (12053,0,1,1,0.4,1,1,1,12053,12053,0,135,135);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (12121,0,1,1,3,1,1,7.5,12121,0,12121,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (12122,0,1,1,3,1,1,7.5,12122,0,12122,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (12127,0,1,1,0.6,1,1,1,12127,12127,0,150,150);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (12159,0,2,2,35,1,1,7.5,12159,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13078,0,1,1,1,1,1,1,13078,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13176,0,1,1,10,1,1,7.5,13176,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13179,0,1,1,10,1,1,7.5,13179,13179,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13180,0,1,1,10,1,1,7.5,13180,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13181,0,1,1,10,1,1,7.5,13181,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13218,0,1,1,10,1,1,7.5,13218,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13236,0,1,1,12,12,1,7.5,13236,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13257,0,1,1,10,1,1,7.5,13257,0,0,108,108);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13284,0,1,1,8,8,1,7.5,13284,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13326,0,1,1,0.5,1,1,1,13326,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13328,0,1,1,0.5,1,1,1,13328,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13331,0,1,1,0.6,1,1,1,13331,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13332,0,1,1,0.6,1,1,1,13332,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13358,0,1,1,1.5,1.5,1,1,13358,13358,0,277,277);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13359,0,1,1,1.5,1.5,1,1,13359,13359,0,137,137);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13421,0,1,1,0.7,1,1,1,13421,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13437,0,1,1,10,1,1,7.5,13437,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13438,0,1,1,10,1,1,7.5,13438,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13439,0,1,1,10,1,1,7.5,13439,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13441,0,1,1,8,1,1,7.5,13441,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13442,0,1,1,12,12,1,7.5,13442,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13443,0,1,1,8,8,1,7.5,13443,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13447,0,1,1,3,1,1,7.5,13447,13447,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13448,0,1,1,2,1,1,1,13448,13448,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13546,0,1,1,0.5,2,1,1,13546,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13577,0,1,1,8,1,1,7.5,13577,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13617,0,1,1,15,1,1,7.5,13617,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13797,0,1,1,3.7,1,1,7.5,13797,0,0,97,97);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13798,0,1,1,3.7,1,1,7.5,13798,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (13959,0,1,1,7,1,1,7.5,13959,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (14185,0,1,1,1,1,1,1,14185,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (14186,0,1,1,1,1,1,1,14186,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (14187,0,1,1,1,1,1,1,14187,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (14188,0,1,1,1,1,1,1,14188,0,0,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (14282,0,1,1,0.4,1,1,1,14282,0,14282,0,0);
INSERT INTO `creature_template_difficulty` (Entry, DifficultyID, MinLevel, MaxLevel, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, LootID, PickPocketLootID, SkinLootID, GoldMin, GoldMax) VALUES (14283,0,1,1,0.4,1,1,1,14283,0,0,0,0);

-- 17 more creatures already have a correct creature_template_difficulty row, just with LootID=0
-- despite a real, populated creature_loot_template table existing at their own entry number.
UPDATE `creature_template_difficulty` SET LootID = 808 WHERE Entry = 808 AND DifficultyID IN (0) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 3870 WHERE Entry = 3870 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 37070 WHERE Entry = 37070 AND DifficultyID IN (0) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 37073 WHERE Entry = 37073 AND DifficultyID IN (0) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 37112 WHERE Entry = 37112 AND DifficultyID IN (0) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 37507 WHERE Entry = 37507 AND DifficultyID IN (0) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 42938 WHERE Entry = 42938 AND DifficultyID IN (0) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47132 WHERE Entry = 47132 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47136 WHERE Entry = 47136 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47137 WHERE Entry = 47137 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47138 WHERE Entry = 47138 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47140 WHERE Entry = 47140 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47141 WHERE Entry = 47141 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47143 WHERE Entry = 47143 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47145 WHERE Entry = 47145 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 47146 WHERE Entry = 47146 AND DifficultyID IN (0,2) AND LootID = 0;
UPDATE `creature_template_difficulty` SET LootID = 61408 WHERE Entry = 61408 AND DifficultyID IN (0) AND LootID = 0;

-- Creatures 39363 ("Chip Endale") and 43325 ("Rockjaw Fungus-Flinger") reference a loot table at
-- their own entry (the standard convention, confirmed via AshamaneCore) that had zero rows in
-- ArgusCore - not a linkage bug, the actual loot content was missing entirely. Restored from
-- AshamaneCore's real data (132 rows for 39363, 13 for 43325).
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,414,21.5686,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,727,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,818,0.5697,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,858,21.5686,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1179,2.8,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1495,0.2645,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1497,0.2,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1498,0.4,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1499,0.2747,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1501,0.2,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1502,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1503,0.2,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1504,0.2,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1506,0.3459,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1507,0.3256,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1509,0.2543,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1510,0.4,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1511,0.3561,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1512,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1513,0.3256,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1514,0.4578,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1515,0.3866,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1516,0.6308,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1730,0.2543,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1731,0.2035,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1732,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1734,0.3968,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1735,0.2,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,1737,0.234,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2075,0.2238,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2140,0.6206,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2214,0.2,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2215,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2287,7.7017,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2407,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2409,1.9608,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2455,0.9,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2589,53.7,0,1,0,1,3);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2632,0.7997,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2763,0.3663,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2777,0.3663,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,2778,0.2999,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3192,0.1119,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3279,0.061,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3281,0.0814,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3282,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3283,0.0509,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3284,0.0102,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3285,0.1526,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3286,0.0407,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3287,0.0407,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3288,0.1017,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3289,0.1526,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3290,0.1017,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3291,0.1017,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3292,0.0305,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3303,0.1119,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3304,0.0102,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3312,0.0814,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3374,0.2849,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3375,0.234,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3610,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3644,0.1526,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3649,0.0305,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,3654,0.0102,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4292,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4345,0.02,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4346,0.1999,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4408,0.1999,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4537,13.7255,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4541,9.8039,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4561,0.2849,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4564,0.05,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4566,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4569,0.05,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4669,0.0712,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4672,0.1323,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4675,0.0509,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4680,0.0712,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4681,0.0305,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4686,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4687,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4692,0.061,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,4693,0.0102,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,5071,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,5368,43.1373,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,5573,0.05,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,6342,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,6527,0.1017,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,7288,0.0407,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,8178,0.1,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,8180,0.2238,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14086,0.5799,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14089,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14090,0.234,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14091,0.0916,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14094,0.0814,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14095,0.2543,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14097,0.0407,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14098,0.1323,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14099,0.0407,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14102,0.2137,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14110,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14115,0.0509,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,14116,0.0102,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15008,0.0814,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15009,0.1119,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15010,0.1017,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15013,0.0305,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15015,0.061,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15297,0.1424,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15298,0.0102,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15299,0.0509,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15300,0.0305,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15301,0.234,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15302,0.1933,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15303,0.0509,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15473,0.0814,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15476,0.0305,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15477,0.1526,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15479,0.0916,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15480,0.0305,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15481,0.061,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15482,0.0814,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15483,0.0712,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15484,0.0814,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15485,0.0102,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15486,0.0916,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15490,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15944,0.061,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15969,0.0509,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (39363,0,15970,0.0203,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,117,42.1796,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,159,20.3835,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,805,5.0454,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,828,6.2563,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,4496,6.8618,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,5571,5.0454,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,5572,5.449,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,55973,1.11,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,55983,1.11,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,57987,6.0545,1,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,58202,0.8073,1,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,62771,0.4036,0,1,0,1,1);
INSERT INTO `creature_loot_template` (Entry, ItemType, Item, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount) VALUES (43325,0,62772,5.9536,0,1,0,1,1);
