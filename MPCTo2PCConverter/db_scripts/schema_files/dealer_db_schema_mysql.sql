-- -----------------------------------------------------
-- Schema dealer
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS `dealer` ;
-- -----------------------------------------------------
-- Schema dealer
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `dealer` ;
USE `dealer` ;
-- -----------------------------------------------------
-- Table `dealer`.`model`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `dealer`.`model` ;
CREATE TABLE IF NOT EXISTS `dealer`.`model` (
  `model_id` INT NOT NULL AUTO_INCREMENT,
  `model_name` VARCHAR(50) NOT NULL,
  PRIMARY KEY (`model_id`));
-- -----------------------------------------------------
-- Table `dealer`.`batch`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `dealer`.`batch` ;
CREATE TABLE IF NOT EXISTS `dealer`.`batch` (
  `batch_id` INT NOT NULL AUTO_INCREMENT,
  `model_id` INT NOT NULL,
  `mac_key_share` VARCHAR(200) NOT NULL,
  `shares_seed` VARCHAR(200) NOT NULL,
  `mac_key_mask` VARCHAR(200) NOT NULL,
  PRIMARY KEY (`batch_id`),
  INDEX `fk_batch_model_idx` (`model_id` ASC) VISIBLE,
  CONSTRAINT `fk_batch_model`
    FOREIGN KEY (`model_id`)
    REFERENCES `dealer`.`model` (`model_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);
-- -----------------------------------------------------
-- Table `dealer`.`share`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `dealer`.`share` ;
CREATE TABLE IF NOT EXISTS `dealer`.`share` (
  `share_id` INT NOT NULL AUTO_INCREMENT,
  `share` VARCHAR(200) NOT NULL,
  `mac_share` VARCHAR(200) NOT NULL,
  PRIMARY KEY (`share_id`));
-- -----------------------------------------------------
-- Table `dealer`.`material_type`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `dealer`.`material_type` ;
CREATE TABLE IF NOT EXISTS `dealer`.`material_type` (
  `material_type_id` INT NOT NULL AUTO_INCREMENT,
  `material_type` VARCHAR(50) NOT NULL,
  PRIMARY KEY (`material_type_id`));
-- -----------------------------------------------------
-- Table `dealer`.`types_per_batch`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `dealer`.`types_per_batch` ;
CREATE TABLE IF NOT EXISTS `dealer`.`types_per_batch` (
  `types_per_batch_id` INT NOT NULL AUTO_INCREMENT,
  `batch_id` INT NOT NULL,
  `material_type_id` INT NOT NULL,
  `start_row` INT NOT NULL,
  `end_row` INT NOT NULL,
  `order_in_batch` INT NOT NULL,
  PRIMARY KEY (`types_per_batch_id`),
  INDEX `fk_types_per_batch_material_type_idx` (`material_type_id` ASC) VISIBLE,
  INDEX `fk_types_per_batch_batch_idx` (`batch_id` ASC) VISIBLE,
  CONSTRAINT `fk_types_per_batch_batch`
    FOREIGN KEY (`batch_id`)
    REFERENCES `dealer`.`batch` (`batch_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_types_per_batch_material_type`
    FOREIGN KEY (`material_type_id`)
    REFERENCES `dealer`.`material_type` (`material_type_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);
-- -----------------------------------------------------
-- Table `dealer`.`types_per_model`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `dealer`.`types_per_model` ;
CREATE TABLE IF NOT EXISTS `dealer`.`types_per_model` (
  `types_per_model_id` INT NOT NULL AUTO_INCREMENT,
  `model_id` INT NOT NULL,
  `material_type_id` INT NOT NULL,
  `num_materials` INT NOT NULL,
  PRIMARY KEY (`types_per_model_id`),
  INDEX `fk_types_per_model_model_idx` (`model_id` ASC) VISIBLE,
  CONSTRAINT `fk_types_per_model_model`
    FOREIGN KEY (`model_id`)
    REFERENCES `dealer`.`model` (`model_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);