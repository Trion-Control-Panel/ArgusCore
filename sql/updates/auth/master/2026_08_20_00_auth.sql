DELETE FROM `rbac_linked_permissions` WHERE `linkedId` IN (887,888,889);
DELETE FROM `rbac_permissions` WHERE `id` IN (887,888,889);

INSERT INTO `rbac_permissions` (`id`,`name`) VALUES
(887,'Command: partition disable'),
(888,'Command: partition enable'),
(889,'Command: partition status');

INSERT INTO `rbac_linked_permissions` (`id`,`linkedId`) VALUES
(192,887),
(192,888),
(193,889);
