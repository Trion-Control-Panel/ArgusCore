-- See ARGUSCORE_FIXES.md for details.

DELETE FROM `spell_proc` WHERE `SpellId` IN (257506,342076,452409);

UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 36032;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 48484;
UPDATE `spell_proc` SET `AttributesMask` = `AttributesMask` & ~(0x20) WHERE `SpellId` = 63849;
UPDATE `spell_proc` SET `AttributesMask` = `AttributesMask` & ~(0x100) WHERE `SpellId` = 69762;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 72059;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 85804;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 115946;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 144585;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 145251;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 165308;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 187213;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 198063;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 199806;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 203974;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 207779;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 210706;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 212625;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 218186;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 221661;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 221678;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 231862;
UPDATE `spell_proc` SET `HitMask` = 0 WHERE `SpellId` = 238101;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 242207;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 242209;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 242628;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 253026;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 253287;
UPDATE `spell_proc` SET `SpellPhaseMask` = 0 WHERE `SpellId` = 258984;

UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 0, `HitMask` = 2, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 7434;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 0, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0.7, `Chance` = 0, `Cooldown` = 40, `Charges` = 0 WHERE `SpellId` = 39958;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 11, `SpellFamilyMask0` = 256, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 16, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 2, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 51564;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 0, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 45, `Charges` = 0 WHERE `SpellId` = 55380;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 9, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 0, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 70727;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 16, `SpellTypeMask` = 0, `SpellPhaseMask` = 1, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 101568;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 2446676, `SpellTypeMask` = 0, `SpellPhaseMask` = 1, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 108281;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 87040, `SpellTypeMask` = 0, `SpellPhaseMask` = 1, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 108839;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 3, `SpellFamilyMask0` = 4194304, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 2097160, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 1, `SpellPhaseMask` = 2, `HitMask` = 2, `AttributesMask` = 2, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 155148;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 332048, `SpellTypeMask` = 0, `SpellPhaseMask` = 1, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 193537;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 3, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 2, `HitMask` = 2, `AttributesMask` = 2, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 195345;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 3, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 2, `HitMask` = 2, `AttributesMask` = 2, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 195419;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 10, `SpellFamilyMask0` = 1073741824, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 1024, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 2, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 196923;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 20, `SpellTypeMask` = 0, `SpellPhaseMask` = 1, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 209400;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 348176, `SpellTypeMask` = 0, `SpellPhaseMask` = 1, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 231895;
UPDATE `spell_proc` SET `SchoolMask` = 0, `SpellFamilyName` = 4, `SpellFamilyMask0` = 536870912, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SpellFamilyMask3` = 0, `ProcFlags` = 0, `SpellTypeMask` = 0, `SpellPhaseMask` = 2, `HitMask` = 0, `AttributesMask` = 0, `ProcsPerMinute` = 0, `Chance` = 0, `Cooldown` = 0, `Charges` = 0 WHERE `SpellId` = 238147;
