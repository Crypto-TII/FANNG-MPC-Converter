DROP SCHEMA IF EXISTS `mpclib-database-test` ;
CREATE SCHEMA IF NOT EXISTS `mpclib-database-test`;
USE `mpclib-database-test`;
DROP TABLE IF EXISTS `mpclib-database-test`.`share` ;
CREATE TABLE IF NOT EXISTS `mpclib-database-test`.`share` (
    `ID`               INTEGER       NOT NULL AUTO_INCREMENT,
    `PLAYER`           INTEGER       NOT NULL,
    `CHANNEL`          INTEGER       NOT NULL,
    `SHARE`            VARCHAR(200)  NOT NULL,
    `MAC_SHARE`        VARCHAR(200)  NULL,
    `CREATION_DATE`    TIMESTAMP     NOT NULL DEFAULT NOW(),
    CONSTRAINT `share_id`
        PRIMARY KEY (`ID`)
);