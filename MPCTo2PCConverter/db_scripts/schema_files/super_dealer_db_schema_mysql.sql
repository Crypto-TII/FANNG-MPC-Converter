-- -----------------------------------------------------
-- Schema super_dealer
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS `super_dealer` ;
-- -----------------------------------------------------
-- Schema super_dealer
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `super_dealer` ;
USE `super_dealer` ;
-- -----------------------------------------------------
-- Table `super_dealer`.`model`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `super_dealer`.`model` ;
CREATE TABLE IF NOT EXISTS `super_dealer`.`model` (
  `model_id` INT NOT NULL AUTO_INCREMENT,
  `model_name` VARCHAR(50) NOT NULL,
  PRIMARY KEY (`model_id`));
-- -----------------------------------------------------
-- Table `super_dealer`.`batch`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `super_dealer`.`batch` ;
CREATE TABLE IF NOT EXISTS `super_dealer`.`batch` (
  `batch_id` INT NOT NULL AUTO_INCREMENT,
  `model_id` INT NOT NULL,
  `used` TINYINT NOT NULL,
  INDEX `fk_batch_model_idx` (`model_id` ASC) VISIBLE,
  PRIMARY KEY (`batch_id`),
  CONSTRAINT `fk_batch_model`
    FOREIGN KEY (`model_id`)
    REFERENCES `super_dealer`.`model` (`model_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);
