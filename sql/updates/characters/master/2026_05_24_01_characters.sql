-- Phase 10: World Layering — persist last-known layer so the 30-min cooldown
-- survives server restarts and so /who-style tools can report layer per player.
ALTER TABLE `characters`
    ADD COLUMN `world_layer` INT UNSIGNED NOT NULL DEFAULT 0
    COMMENT 'Open-world layer the player was last assigned to (0 = default)'
    AFTER `honorRestBonus`;
