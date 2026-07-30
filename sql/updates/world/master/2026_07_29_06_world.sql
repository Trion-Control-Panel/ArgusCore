-- Shaman: removed Unrelenting Storms (470490/470491) - confirmed The War Within patch 11.0.5
-- (2024-10-22) talent, removed again in patch 12.0.0, no Legion connection. Part of the ongoing
-- Legion 7.3.5 forward-drift removal pass (see ARGUSCORE_FIXES.md). Was Load()-gated on the talent
-- aura (pure no-op without it) and, despite its own header comment citing its own talent id
-- (470490), was actually bound via spell_script_names to Crash Lightning's own cast (187874) since
-- it modifies Crash Lightning's cooldown/Windfury interaction rather than being cast directly
-- itself - same "talent overlay bound to a different base spell" shape as Molten Thunder/Sundering
-- and Stormweaver/Maelstrom Weapon earlier this pass. Crash Lightning's own existing scripts
-- (spell_sha_crash_lightning, spell_sha_chain_lightning_crash_lightning) are untouched.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_sha_unrelenting_storms';
