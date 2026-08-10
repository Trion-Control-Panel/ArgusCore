-- Correct gameobject_template SpellFocus (type 8/GAMEOBJECT_TYPE_SPELL_FOCUS) references that
-- pointed at SpellFocus ids not present in Legion 7.3.5. `spell_focus_object` is a
-- HotfixDatabase-backed mirror of the client's SpellFocusObject.db2 (see HotfixDatabase.cpp
-- HOTFIX_SEL_SPELL_FOCUS_OBJECT / DB2Stores.cpp sSpellFocusObjectStore), not a world-DB table -
-- so as with the lock fixes in 2026_08_10_01_world.sql, there is no "spell_focus_object" row to
-- restore; the corruption is on the gameobject_template side.
--
-- Checked against wago.tools's SpellFocusObject.db2 export pinned to build 7.3.5.26972: id 2061
-- exists unpinned only as "8.1 SoS Pre Raid - Void Stone Cast - KAP" (BfA 8.1 dev/PTR content)
-- and returns nothing when pinned to this build; id 5708584 likewise returns nothing pinned
-- (the pinned SpellFocusObject.db2 table tops out at id 1930). Neither is genuine Legion 7.3.5
-- data, matching the pattern already established for the Lock ids in the companion migration.
--
-- Group A - entries 126337, 126338, 126339, 126340, 126341, 126342, 126345, 151951, 191300:
--   DestinyCore and AshamaneCore (both full 7.3.5 build-735.02 dumps) carry BYTE-IDENTICAL
--   gameobject_template rows for all 9 entries, type 8 (matches what's locally loaded, so this
--   is confirmed to be the same object, not id-space reuse), with Data0=4 in every single one -
--   never 2061. SpellFocus id 4 is "Cooking Fire" per wago.tools's build-7.3.5.26972-pinned
--   SpellFocusObject.db2 export, genuinely present at this build - a plausible "Fire" spell
--   focus reused across several cooking/totem-carving props (one is literally named
--   "Idol Oven Fire"). Local data0=2061 is corrupted; restoring to the corroborated value 4.
--
-- Group B - entries 210791, 210792, 210793, 210794 ("Doodad_UndeadCampFire29/30/31/32"):
--   DestinyCore and AshamaneCore both show these as type 0 (GAMEOBJECT_TYPE_DOOR), NOT type 8 -
--   plain decorative campfire doodads with Data0=0 and no spell-focus relationship at all. This
--   is a bigger corruption than Group A (the `type` column itself was flipped to 8 locally, not
--   just data0), but it's still 2/2-corroborated byte-identical between both primary references,
--   so it's restored as a full row correction rather than left unresolved. Reference row (both
--   cores, e.g. entry 210791): (210791,0,396,'Doodad_UndeadCampFire29','','','',1.33,0,10,
--   0,0,...,0,'','',17538) - displayId 396, size 1.33, Data0=0, Data1=10, all other Data
--   fields 0, VerifiedBuild 17538.

UPDATE `gameobject_template` SET `Data0`=4 WHERE `type`=8 AND `Data0`=2061 AND `entry` IN
    (126337,126338,126339,126340,126341,126342,126345,151951,191300);

UPDATE `gameobject_template` SET
    `type`=0, `displayId`=396, `name`='Doodad_UndeadCampFire29', `IconName`='', `castBarCaption`='',
    `unk1`='', `size`=1.33, `Data0`=0, `Data1`=10, `Data2`=0, `Data3`=0, `Data4`=0, `Data5`=0,
    `Data6`=0, `Data7`=0, `Data8`=0, `Data9`=0, `Data10`=0, `Data11`=0, `Data12`=0, `Data13`=0,
    `Data14`=0, `Data15`=0, `Data16`=0, `Data17`=0, `Data18`=0, `Data19`=0, `Data20`=0, `Data21`=0,
    `Data22`=0, `Data23`=0, `Data24`=0, `Data25`=0, `Data26`=0, `Data27`=0, `Data28`=0, `Data29`=0,
    `Data30`=0, `Data31`=0, `Data32`=0, `VerifiedBuild`=17538
    WHERE `entry`=210791 AND `type`=8 AND `Data0`=5708584;
UPDATE `gameobject_template` SET
    `type`=0, `displayId`=396, `name`='Doodad_UndeadCampFire30', `IconName`='', `castBarCaption`='',
    `unk1`='', `size`=1.33, `Data0`=0, `Data1`=10, `Data2`=0, `Data3`=0, `Data4`=0, `Data5`=0,
    `Data6`=0, `Data7`=0, `Data8`=0, `Data9`=0, `Data10`=0, `Data11`=0, `Data12`=0, `Data13`=0,
    `Data14`=0, `Data15`=0, `Data16`=0, `Data17`=0, `Data18`=0, `Data19`=0, `Data20`=0, `Data21`=0,
    `Data22`=0, `Data23`=0, `Data24`=0, `Data25`=0, `Data26`=0, `Data27`=0, `Data28`=0, `Data29`=0,
    `Data30`=0, `Data31`=0, `Data32`=0, `VerifiedBuild`=17538
    WHERE `entry`=210792 AND `type`=8 AND `Data0`=5708584;
UPDATE `gameobject_template` SET
    `type`=0, `displayId`=396, `name`='Doodad_UndeadCampFire31', `IconName`='', `castBarCaption`='',
    `unk1`='', `size`=1.33, `Data0`=0, `Data1`=10, `Data2`=0, `Data3`=0, `Data4`=0, `Data5`=0,
    `Data6`=0, `Data7`=0, `Data8`=0, `Data9`=0, `Data10`=0, `Data11`=0, `Data12`=0, `Data13`=0,
    `Data14`=0, `Data15`=0, `Data16`=0, `Data17`=0, `Data18`=0, `Data19`=0, `Data20`=0, `Data21`=0,
    `Data22`=0, `Data23`=0, `Data24`=0, `Data25`=0, `Data26`=0, `Data27`=0, `Data28`=0, `Data29`=0,
    `Data30`=0, `Data31`=0, `Data32`=0, `VerifiedBuild`=17538
    WHERE `entry`=210793 AND `type`=8 AND `Data0`=5708584;
UPDATE `gameobject_template` SET
    `type`=0, `displayId`=396, `name`='Doodad_UndeadCampFire32', `IconName`='', `castBarCaption`='',
    `unk1`='', `size`=1.33, `Data0`=0, `Data1`=10, `Data2`=0, `Data3`=0, `Data4`=0, `Data5`=0,
    `Data6`=0, `Data7`=0, `Data8`=0, `Data9`=0, `Data10`=0, `Data11`=0, `Data12`=0, `Data13`=0,
    `Data14`=0, `Data15`=0, `Data16`=0, `Data17`=0, `Data18`=0, `Data19`=0, `Data20`=0, `Data21`=0,
    `Data22`=0, `Data23`=0, `Data24`=0, `Data25`=0, `Data26`=0, `Data27`=0, `Data28`=0, `Data29`=0,
    `Data30`=0, `Data31`=0, `Data32`=0, `VerifiedBuild`=17538
    WHERE `entry`=210794 AND `type`=8 AND `Data0`=5708584;
