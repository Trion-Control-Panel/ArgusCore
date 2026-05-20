-- Remove stale tlpp update include path (TheLegionPreservationProject SQL was merged, directory no longer exists).
DELETE FROM `updates_include` WHERE `path` = '$/sql/tlpp/world';
