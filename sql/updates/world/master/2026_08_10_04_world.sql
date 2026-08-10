-- Re-restore page_text id 7154 ("...of sea, spirit and self...", referenced by GameObject 148502
-- data7). This row was already restored once by 2026_08_06_04_world.sql (sourced from
-- AshamaneCore) using a plain `INSERT INTO`, but worldserver is still logging "PageText (Entry
-- 7154) not exist" - per the project's established INSERT IGNORE convention (see CLAUDE.md /
-- prior session notes), a plain INSERT INTO in that batch most likely hit a duplicate-key
-- conflict against a pre-existing base-TDB row for one of the OTHER ids in that same statement
-- batch (4546/4874/4958), aborting the script before reaching 7154's row. Re-issuing with
-- INSERT IGNORE so this one applies independently of the others' state.
--
-- page_text id 176592 (referenced by GameObject 176635 data7, "Shellfish Trap") remains
-- unresolved: confirmed genuinely absent from BOTH available primary reference cores
-- (DestinyCore's and AshamaneCore's full 7.3.5 build-735.02 world DB dumps), including their
-- page_text_locale tables. GO 176635 itself is present and byte-identical in both cores
-- (type 3/CHEST, "Shellfish Trap", Data7=176592 - confirming the reference is genuine and the
-- object is real Legion-valid content), but the referenced text content is not carried in either
-- dump. No source to restore the actual text from - left unresolved rather than fabricated.
-- (Note: LegionCore-7.3.5 and LegionCore-7.3.5V2, listed as secondary references in CLAUDE.md,
-- are not present in this checkout's logs/ directory and could not be checked.)

INSERT IGNORE INTO `page_text` (ID, Text, NextPageID, PlayerConditionID, Flags, VerifiedBuild) VALUES (7154,'...of sea, spirit and self...',0,0,0,0);
