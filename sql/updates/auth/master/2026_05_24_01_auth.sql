DELETE FROM `rbac_linked_permissions` WHERE `linkedId` IN (884,885,886);
DELETE FROM `rbac_permissions` WHERE `id` IN (884,885,886);

INSERT INTO `rbac_permissions` (`id`,`name`) VALUES
(884,'Command: layer info'),
(885,'Command: layer list'),
(886,'Command: layer migrate');

INSERT INTO `rbac_linked_permissions` (`id`,`linkedId`) VALUES
(197,884),
(197,885),
(196,886);
