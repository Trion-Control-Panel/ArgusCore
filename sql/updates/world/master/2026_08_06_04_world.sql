-- Restore 4 page_text rows missing from ArgusCore's world DB but present in AshamaneCore
-- (176592 and 6279 from the same error batch were checked too and are genuinely absent from
-- AshamaneCore as well - left unimplemented, no source to restore them from).
INSERT INTO `page_text` (ID, Text, NextPageID, PlayerConditionID, Flags, VerifiedBuild) VALUES (4546,'Ten thousand years ago, Emperor Shaohao, the Last Emperor of Pandaria, used the power of these sacred waters to spare Pandaria from the devastation of the Sundering that destroyed the rest of the world.',0,0,0,0);
INSERT INTO `page_text` (ID, Text, NextPageID, PlayerConditionID, Flags, VerifiedBuild) VALUES (4874,'Care: debris.',0,0,0,0);
INSERT INTO `page_text` (ID, Text, NextPageID, PlayerConditionID, Flags, VerifiedBuild) VALUES (4958,'ATTACK! $B$BPUT FIST IN FACE! $B$BCONTINUE THE ATTACKING!',0,0,0,0);
INSERT INTO `page_text` (ID, Text, NextPageID, PlayerConditionID, Flags, VerifiedBuild) VALUES (7154,'...of sea, spirit and self...',0,0,0,0);
