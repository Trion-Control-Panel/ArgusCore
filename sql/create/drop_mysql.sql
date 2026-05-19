REVOKE ALL PRIVILEGES ON * . * FROM 'arguscore'@'localhost';

REVOKE ALL PRIVILEGES ON `world` . * FROM 'arguscore'@'localhost';

REVOKE GRANT OPTION ON `world` . * FROM 'arguscore'@'localhost';

REVOKE ALL PRIVILEGES ON `characters` . * FROM 'arguscore'@'localhost';

REVOKE GRANT OPTION ON `characters` . * FROM 'arguscore'@'localhost';

REVOKE ALL PRIVILEGES ON `auth` . * FROM 'arguscore'@'localhost';

REVOKE GRANT OPTION ON `auth` . * FROM 'arguscore'@'localhost';

REVOKE ALL PRIVILEGES ON `hotfixes` . * FROM 'arguscore'@'localhost';

REVOKE GRANT OPTION ON `hotfixes` . * FROM 'arguscore'@'localhost';

DROP USER 'arguscore'@'localhost';

DROP DATABASE IF EXISTS `world`;

DROP DATABASE IF EXISTS `characters`;

DROP DATABASE IF EXISTS `auth`;

DROP DATABASE IF EXISTS `hotfixes`;
