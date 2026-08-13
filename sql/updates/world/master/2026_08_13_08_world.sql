-- See ARGUSCORE_FIXES.md for details.
UPDATE `trainer_spell` SET `ReqSkillLine` = 0, `ReqSkillRank` = 0
WHERE `ReqSkillLine` IN (2477,2485,2494,2506,2514,2524,2532,2534,2536,2537,2538,2539,2540,2545,2548,2572);
